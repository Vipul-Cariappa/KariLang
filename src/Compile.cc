#include "Compile.hh"
#include "AST.hh"
#include "Utils.hh"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
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

llvm::Value *LogErrorV(const char *str) {
    std::cerr << str;
    return nullptr;
}

llvm::Value *Expression::generate_llvm_ir() {
    switch (type) {
    case INTEGER_EXP:
        return llvm::ConstantInt::get(
            *TheContext, llvm::APInt(32, std::get<int>(value), true));
    case BOOLEAN_EXP:
        return llvm::ConstantInt::get(
            *TheContext, llvm::APInt(1, std::get<bool>(value), false));
    case VARIABLE_EXP:
        return NamedValues[std::get<std::string>(value)];
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
        // FIXME: the below ops should be converted to 1 bit result
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

    std::vector<llvm::Value *> ArgsV;
    for (size_t i = 0; i < args.size(); i++) {
        ArgsV.push_back(args.at(i)->generate_llvm_ir());
        if (!ArgsV.back())
            return nullptr;
    }
    return Builder->CreateCall(CalleeF, ArgsV, "funccall");
}

llvm::Value *ValueDef::generate_llvm_ir() {
    // TODO: implement
    std::cerr << "Not Implemented";
    exit(1);
}

llvm::Value *FunctionDef::generate_llvm_ir() {
    // First, check for an existing function
    llvm::Function *TheFunction;
    if (name == "main")
        TheFunction = TheModule->getFunction("____karilang_main");
    else
        TheFunction = TheModule->getFunction(name);

    if (!TheFunction)
        return (llvm::Function *)LogErrorV(
            "Function prototype not found. "
            "Prototype should be defined before body is defined.");
    if (!TheFunction->empty())
        return (llvm::Function *)LogErrorV("Function cannot be redefined");

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(
        *TheContext, std::string(name) + "_entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

    if (llvm::Value *RetVal = expression->generate_llvm_ir()) {
        // Finish off the function.
        Builder->CreateRet(RetVal);

        // TheFunction->print(llvm::errs(), nullptr);

        // Validate the generated code, checking for consistency.
        assert(!verifyFunction(*TheFunction));

        TheFPM->run(*TheFunction);
        return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();
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

    // Create a new pass manager attached to it.
    TheFPM =
        std::make_unique<llvm::legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(llvm::createInstructionCombiningPass());
    // Re-associate expressions.
    TheFPM->add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(llvm::createCFGSimplificationPass());
    // tail call optimization
    TheFPM->add(llvm::createTailCallEliminationPass());

    TheFPM->doInitialization();

    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

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

    // Print generated IR
    // TheModule->print(llvm::errs(), nullptr);

    // Getting target type
    std::string TargetTriple = llvm::sys::getDefaultTargetTriple();
    // std::cout << TargetTriple << std::endl;

    // Initializing target
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

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
    dest.flush();

    return 0;
}
