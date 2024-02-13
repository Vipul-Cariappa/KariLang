#include "JIT.hh"
#include "AST.hh"
#include "Compile.hh"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <memory>
#include <utility>
#include <vector>

llvm::ExitOnError ExitOnErr;
std::unique_ptr<KariLangJIT> TheJIT;

namespace jit {
int jit() {
    // Open a new context and module.
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("LLVM JIT", *TheContext);
    TheModule->setDataLayout(TheJIT->getDataLayout());

    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);

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
    return 0;
}

void JIT_Expression(std::unique_ptr<Expression> exp) {
    TYPE expected_result_type = exp->deduce_result_type();
    std::unordered_map<std::string, TYPE> semantics_context;
    if (!exp->verify_semantics(expected_result_type, functions_ast, globals_ast,
                               semantics_context))
        return;

    // Creating prototype
    // setup arguments type
    std::vector<llvm::Type *> ArgV;

    // setup return type
    llvm::FunctionType *FT;
    switch (exp->result_type) {
    case TYPE::BOOL_T:
        FT = llvm::FunctionType::get(llvm::Type::getInt1Ty(*TheContext), ArgV,
                                     false);
        break;
    case TYPE::INT_T:
    default:
        FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), ArgV,
                                     false);
        break;
    }
    // creating function
    llvm::Function *F = llvm::Function::Create(
        FT, llvm::Function::ExternalLinkage,
        "___anonymous_expression_evaluator_function", TheModule.get());

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "_entry", F);
    Builder->SetInsertPoint(BB);

    llvm::Value *RetVal = exp->generate_llvm_ir();
    // Finish off the function.
    Builder->CreateRet(RetVal);
    // F->print(llvm::errs(), nullptr);
    // Validate the generated code, checking for consistency.
    assert(!verifyFunction(*F));

    // Create a ResourceTracker to track JIT'd memory allocated to our
    // anonymous expression -- that way we can free it after executing.
    llvm::orc::ResourceTrackerSP RT =
        TheJIT->getMainJITDylib().createResourceTracker();

    llvm::orc::ThreadSafeModule TSM = llvm::orc::ThreadSafeModule(
        std::move(TheModule), std::move(TheContext));
    ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
    jit();

    // Search the JIT for the __anon_expr symbol.
    llvm::orc::ExecutorSymbolDef ExprSymbol =
        ExitOnErr(TheJIT->lookup("___anonymous_expression_evaluator_function"));

    // Get the symbol's address and cast it to the right type (takes no
    // arguments, returns a double) so we can call it as a native function.
    switch (exp->result_type) {
    case INT_T: {
        int (*FP)() = ExprSymbol.getAddress().toPtr<int (*)()>();
        std::cout << FP() << "\n";
        break;
    }
    case BOOL_T: {
        bool (*FP)() = ExprSymbol.getAddress().toPtr<bool (*)()>();
        std::cout << (FP() ? "true" : "false") << "\n";
        break;
    }
    }

    // Delete the anonymous expression module from the JIT.
    ExitOnErr(RT->remove());
}
} // namespace jit
