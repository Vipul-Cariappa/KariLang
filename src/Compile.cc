#include "Compile.hh"
#include "AST.hh"
#include "Utils.hh"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

std::unique_ptr<llvm::LLVMContext> TheContext;
std::unique_ptr<llvm::IRBuilder<>> Builder;
std::unique_ptr<llvm::Module> TheModule;
std::map<std::string, llvm::Value *> NamedValues;
std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
std::unique_ptr<llvm::DIBuilder> DBuilder;
DebugInfo KLDbgInfo;

llvm::Value *LogErrorV(const char *str) {
    std::cerr << str;
    return nullptr;
}

void DebugInfo::emitLocation(BaseExpression *AST) {
    if (!AST)
        return Builder->SetCurrentDebugLocation(llvm::DebugLoc());
    llvm::DIScope *Scope;
    if (LexicalBlocks.empty())
        Scope = TheCU;
    else
        Scope = LexicalBlocks.back();
    Builder->SetCurrentDebugLocation(llvm::DILocation::get(
        Scope->getContext(), AST->start_line, AST->start_column, Scope));
}

static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                                llvm::StringRef VarName) {
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                           TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(NamedValues.at(VarName.str())->getType(), nullptr,
                             VarName);
}

llvm::Value *Expression::generate_llvm_ir() {
    switch (type) {
    case INTEGER_EXP:
        KLDbgInfo.emitLocation(this);
        return llvm::ConstantInt::get(
            *TheContext, llvm::APInt(32, std::get<int>(value), true));
    case BOOLEAN_EXP:
        KLDbgInfo.emitLocation(this);
        return llvm::ConstantInt::get(
            *TheContext, llvm::APInt(1, std::get<bool>(value), false));
    case VARIABLE_EXP: {
        KLDbgInfo.emitLocation(this);
        if (NamedValues[std::get<std::string>(value)])
            return NamedValues[std::get<std::string>(value)];

        // all variables (except arguments for now) are no args function which
        // return the datatype as the variable
        llvm::Function *CalleeF =
            TheModule->getFunction(std::get<std::string>(value));

        // ???: below is required for JIT
        auto f = globals_ast.find(std::get<std::string>(value));
        if ((!CalleeF) && (f != globals_ast.end())) {
            CalleeF = FunctionPrototype::generate_llvm_ir(f->second->name, {},
                                                          {}, f->second->type);
        }

        std::vector<llvm::Value *> ArgsV;

        return Builder->CreateCall(CalleeF, ArgsV, "variable_funccall");
    }
    case UNARY_OP_EXP:
        return std::get<std::unique_ptr<UnaryOperator>>(value)
            ->generate_llvm_ir();
    case BINARY_OP_EXP:
        return std::get<std::unique_ptr<BinaryOperator>>(value)
            ->generate_llvm_ir();
    case IF_EXP:
        return std::get<std::unique_ptr<IfOperator>>(value)->generate_llvm_ir();
    case FUNCTION_CALL_EXP:
        return std::get<std::unique_ptr<FunctionCall>>(value)
            ->generate_llvm_ir();
    }
}

llvm::Value *UnaryOperator::generate_llvm_ir() {
    KLDbgInfo.emitLocation(this);
    llvm::Value *L = fst->generate_llvm_ir();
    if (!L)
        return nullptr;
    switch (op_type) {
    case NEG_OP: {
        llvm::Value *R =
            llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
        if (!R)
            return nullptr;
        return Builder->CreateSub(R, L, "neg_op");
    }
    case NOT_OP:
        return Builder->CreateNeg(L, "not_op");
    }
}

llvm::Value *BinaryOperator::generate_llvm_ir() {
    KLDbgInfo.emitLocation(this);
    llvm::Value *L = fst->generate_llvm_ir();
    llvm::Value *R = snd->generate_llvm_ir();
    if ((!L) || (!R))
        return nullptr;

    switch (op_type) {
    case ADD_OP:
        return Builder->CreateAdd(L, R, "add_op");
    case MUL_OP:
        return Builder->CreateMul(L, R, "mul_op");
    case DIV_OP:
        return Builder->CreateSDiv(L, R, "div_op");
    case MOD_OP:
        return Builder->CreateSRem(L, R, "mod_op");
    case AND_OP:
        return Builder->CreateAnd(L, R, "and_op");
    case OR_OP:
        return Builder->CreateOr(L, R, "or_op");
    case EQS_OP:;
        return Builder->CreateICmpEQ(L, R, "eqs_op");
    case NEQ_OP:
        return Builder->CreateICmpNE(L, R, "neq_op");
    case GT_OP:
        return Builder->CreateICmpSGT(L, R, "gt_op");
    case GTE_OP:
        return Builder->CreateICmpSGE(L, R, "gte_op");
    case LT_OP:
        return Builder->CreateICmpSLT(L, R, "lt_op");
    case LTE_OP:
        return Builder->CreateICmpSLE(L, R, "lte_op");
    }
}

llvm::Value *IfOperator::generate_llvm_ir() {
    KLDbgInfo.emitLocation(this);
    llvm::Value *Cond = cond->generate_llvm_ir();
    if (!Cond)
        return nullptr;

    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();

    // create a temporary variable to store the result of if operation
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                           TheFunction->getEntryBlock().begin());
    llvm::AllocaInst *if_op_result;
    switch (result_type) {
    case BOOL_T:
        if_op_result = Builder->CreateAlloca(llvm::Type::getInt1Ty(*TheContext),
                                             nullptr, "if_op_result");
        break;
    case INT_T:
        if_op_result = TmpB.CreateAlloca(llvm::Type::getInt32Ty(*TheContext),
                                         nullptr, "if_op_result_ptr");
        break;
    }

    // Create blocks for the then and else cases
    llvm::BasicBlock *ThenBB =
        llvm::BasicBlock::Create(*TheContext, "if_then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*TheContext, "if_else");
    llvm::BasicBlock *MergeBB =
        llvm::BasicBlock::Create(*TheContext, "if_continued");

    Builder->CreateCondBr(Cond, ThenBB, ElseBB);

    // Emit then value
    Builder->SetInsertPoint(ThenBB);
    if (!Builder->CreateStore(yes->generate_llvm_ir(), if_op_result))
        return nullptr;
    Builder->CreateBr(MergeBB);

    ThenBB = Builder->GetInsertBlock();
    TheFunction->insert(TheFunction->end(), ElseBB);

    // Emit else value
    Builder->SetInsertPoint(ElseBB);
    if (!Builder->CreateStore(no->generate_llvm_ir(), if_op_result))
        return nullptr;
    Builder->CreateBr(MergeBB);

    // Emit merge value
    ElseBB = Builder->GetInsertBlock();
    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder->SetInsertPoint(MergeBB);

    return Builder->CreateLoad(llvm::Type::getInt32Ty(*TheContext),
                               if_op_result, "if_op_result");
}

llvm::Value *FunctionCall::generate_llvm_ir() {
    llvm::Function *CalleeF = TheModule->getFunction(function_name);
    // ???: below is required for JIT
    auto f = functions_ast.find(function_name);
    if ((!CalleeF) && (f != functions_ast.end())) {
        CalleeF = FunctionPrototype::generate_llvm_ir(
            f->second->name, f->second->args_name, f->second->args_type,
            f->second->return_type);
    }

    KLDbgInfo.emitLocation(this);

    std::vector<llvm::Value *> ArgsV;
    for (size_t i = 0; i < args.size(); i++) {
        ArgsV.push_back(args.at(i)->generate_llvm_ir());
        if (!ArgsV.back())
            return nullptr;
    }
    return Builder->CreateCall(CalleeF, ArgsV, "funccall");
}

llvm::Value *ValueDef::generate_llvm_ir() {
    // TODO: add debug info

    std::vector<llvm::Type *> ArgV;
    llvm::FunctionType *FT;
    switch (type) {
    case TYPE::INT_T:
        FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), ArgV,
                                     false);
        break;
    case TYPE::BOOL_T:
        FT = llvm::FunctionType::get(llvm::Type::getInt1Ty(*TheContext), ArgV,
                                     false);
        break;
    }
    llvm::Function *TheFunction = llvm::Function::Create(
        FT, llvm::Function::ExternalLinkage, name, TheModule.get());

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(
        *TheContext, std::string(name) + "_entry", TheFunction);
    Builder->SetInsertPoint(BB);

    if (llvm::Value *RetVal = expression->generate_llvm_ir()) {
        // Finish off the function.
        Builder->CreateRet(RetVal);

        // TheFunction->print(llvm::errs(), nullptr);

        // Validate the generated code, checking for consistency.
        assert(!verifyFunction(*TheFunction));

        // TheFPM->run(*TheFunction);
        return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();
    return nullptr;
}

llvm::Value *FunctionDef::generate_llvm_ir() {
    // First, check for an existing function
    llvm::Function *TheFunction;
    std::string name = this->name;
    if (name == "main") {
        TheFunction = TheModule->getFunction("____karilang_main");
        name = "____karilang_main";
    } else
        TheFunction = TheModule->getFunction(name);

    if (!TheFunction)
        return (llvm::Function *)LogErrorV(
            "Function prototype not found. "
            "Prototype should be defined before body is defined.");
    if (!TheFunction->empty())
        return (llvm::Function *)LogErrorV("Function cannot be redefined");

    /* DEBUG INFO START */
    // Create a subprogram DIE for this function.
    std::vector<llvm::Metadata *> DArgV;
    // setup return type
    switch (return_type) {
    case TYPE::INT_T:
        DArgV.push_back(KLDbgInfo.getIntTy());
        break;
    case TYPE::BOOL_T:
        DArgV.push_back(KLDbgInfo.getBoolTy());
        break;
    }
    // setup arguments type
    for (size_t i = 0; i < args_name.size(); i++) {
        switch (args_type.at(i)) {
        case TYPE::INT_T:
            DArgV.push_back(KLDbgInfo.getIntTy());
            break;
        case TYPE::BOOL_T:
            DArgV.push_back(KLDbgInfo.getBoolTy());
            break;
        }
    }
    llvm::DISubroutineType *DFunctionType =
        DBuilder->createSubroutineType(DBuilder->getOrCreateTypeArray(DArgV));

    llvm::DIFile *Unit = DBuilder->createFile(KLDbgInfo.TheCU->getFilename(),
                                              KLDbgInfo.TheCU->getDirectory());
    /* DEBUG INFO END */

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(
        *TheContext, std::string(name) + "_entry", TheFunction);
    Builder->SetInsertPoint(BB);

    llvm::DIScope *FContext = Unit;
    unsigned LineNo = start_line;
    unsigned ScopeLine = LineNo;
    llvm::DISubprogram *SP = DBuilder->createFunction(
        FContext, name, llvm::StringRef(), Unit, LineNo, DFunctionType,
        ScopeLine, llvm::DINode::FlagPrototyped,
        llvm::DISubprogram::SPFlagDefinition);
    TheFunction->setSubprogram(SP);

    KLDbgInfo.LexicalBlocks.push_back(SP);

    KLDbgInfo.emitLocation(nullptr);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    unsigned ArgIdx = 0;
    for (auto &Arg : TheFunction->args()) {
        NamedValues[std::string(Arg.getName())] = &Arg;
        ++ArgIdx;

        // Create an alloca for this variable.
        llvm::AllocaInst *Alloca =
            CreateEntryBlockAlloca(TheFunction, Arg.getName());

        // Create a debug descriptor for the variable.
        llvm::DILocalVariable *D = DBuilder->createParameterVariable(
            SP, Arg.getName(), ArgIdx, Unit, LineNo,
            static_cast<llvm::DIType *>(DArgV.at(ArgIdx)), true);

        DBuilder->insertDeclare(
            Alloca, D, DBuilder->createExpression(),
            llvm::DILocation::get(SP->getContext(), LineNo, 0, SP),
            Builder->GetInsertBlock());

        // Store the initial value into the alloca.
        Builder->CreateStore(&Arg, Alloca);
    }

    KLDbgInfo.emitLocation(expression.get());

    if (llvm::Value *RetVal = expression->generate_llvm_ir()) {
        // Finish off the function.
        Builder->CreateRet(RetVal);

        // TheFunction->print(llvm::errs(), nullptr);

        KLDbgInfo.LexicalBlocks.pop_back();

        // Validate the generated code, checking for consistency.
        assert(!verifyFunction(*TheFunction));

        // TheFPM->run(*TheFunction);
        return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();
    KLDbgInfo.LexicalBlocks.pop_back();

    return nullptr;
}

int Compile(const std::string filename,
            const std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
                &functions_ast,
            const std::unordered_map<std::string, std::unique_ptr<ValueDef>>
                &globals_ast) {
    // Open a new context and module.
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("LLVM Compiler", *TheContext);

    // // Create a new pass manager attached to it.
    // TheFPM =
    //     std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

    // // Do simple "peephole" optimizations and bit-twiddling optzns.
    // TheFPM->add(llvm::createInstructionCombiningPass());
    // // Re-associate expressions.
    // TheFPM->add(llvm::createReassociatePass());
    // // Eliminate Common SubExpressions.
    // TheFPM->add(llvm::createGVNPass());
    // // Simplify the control flow graph (deleting unreachable blocks, etc).
    // TheFPM->add(llvm::createCFGSimplificationPass());
    // // tail call optimization
    // TheFPM->add(llvm::createTailCallEliminationPass());

    // TheFPM->doInitialization();

    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

    // Creating Debug builder
    DBuilder = std::make_unique<llvm::DIBuilder>(*TheModule);

    KLDbgInfo.TheCU = DBuilder->createCompileUnit(
        llvm::dwarf::DW_LANG_hi_user, DBuilder->createFile(filename, "."),
        "Kaleidoscope Compiler", false, "", 0);

    // Generate LLVM IR
    for (auto &i : functions_ast)
        // First create prototype for all the functions
        FunctionPrototype::generate_llvm_ir(i.second->name, i.second->args_name,
                                            i.second->args_type,
                                            i.second->return_type);
    for (auto &i : functions_ast)
        i.second->generate_llvm_ir();
    for (auto &i : globals_ast)
        i.second->generate_llvm_ir();

    // Getting target type
    std::string TargetTriple = llvm::sys::getDefaultTargetTriple();
    // std::cout << TargetTriple << std::endl;

    // Initializing target
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();
    
    std::string Error;
    const llvm::Target *Target =
        llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {
        llvm::errs() << Error;
        return 1;
    }

    // Configure other target related types
    std::string CPU = "generic";
    std::string Features = "";
    llvm::TargetOptions opt;
    llvm::TargetMachine *TargetMachine = Target->createTargetMachine(
        TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);
    TheModule->setDataLayout(TargetMachine->createDataLayout());
    TheModule->setTargetTriple(TargetTriple);

    // Compile
    std::filesystem::path output_file(filename);
    output_file.replace_extension("o");

    std::error_code EC;
    llvm::raw_fd_ostream dest(output_file.c_str(), EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return 1;
    }

    llvm::legacy::PassManager pass;
    llvm::CodeGenFileType FileType = llvm::CGFT_ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type ";
        return 1;
    }

    pass.run(*TheModule);
    
    // Write all debug info
    DBuilder->finalize();

    // Print generated IR
    TheModule->print(llvm::errs(), nullptr);
    
    dest.flush();

    return 0;
}
