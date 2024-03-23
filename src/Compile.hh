#include "AST.hh"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DIBuilder.h"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::map<std::string, llvm::Value *> NamedValues;
extern std::unique_ptr<llvm::legacy::FunctionPassManager> TheFPM;
extern std::unique_ptr<llvm::DIBuilder> DBuilder;

typedef struct {
  llvm::DICompileUnit *TheCU;
  llvm::DIType *IntTy;
  llvm::DIType *BoolTy;
  llvm::DIType *getIntTy();
  llvm::DIType *getBoolTy();
  std::vector<llvm::DIScope *> LexicalBlocks;
  void emitLocation(BaseExpression *AST);
} DebugInfo;

extern DebugInfo KLDbgInfo;

inline llvm::DIType *DebugInfo::getIntTy() {
  if (IntTy)
    return IntTy;

  IntTy = DBuilder->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
  return IntTy;
}

inline llvm::DIType *DebugInfo::getBoolTy() {
  if (BoolTy)
    return BoolTy;

  BoolTy = DBuilder->createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
  return BoolTy;
}

int Compile(const std::string filename,
            const std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
                &functions_ast,
            const std::unordered_map<std::string, std::unique_ptr<ValueDef>>
                &globals_ast);

namespace FunctionPrototype {
inline llvm::Function *
generate_llvm_ir(const std::string &name,
                 const std::vector<std::string> &args_name,
                 const std::vector<TYPE> &args_type, TYPE return_type) {
    // setup arguments type
    std::vector<llvm::Type *> ArgV;
    for (size_t i = 0; i < args_name.size(); i++) {
        switch (args_type.at(i)) {
        case TYPE::INT_T:
            ArgV.push_back(llvm::Type::getInt32Ty(*TheContext));
            break;
        case TYPE::BOOL_T:
            ArgV.push_back(llvm::Type::getInt1Ty(*TheContext));
            break;
        }
    }

    // setup return type
    llvm::FunctionType *FT;
    switch (return_type) {
    case TYPE::INT_T:
        FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), ArgV,
                                     false);
        break;
    case TYPE::BOOL_T:
        FT = llvm::FunctionType::get(llvm::Type::getInt1Ty(*TheContext), ArgV,
                                     false);
        break;
    }

    // create the function prototype
    llvm::Function *F;
    if (name == "main")
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                   "____karilang_main", TheModule.get());
    else
        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name,
                                   TheModule.get());

    // setup argument names
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(args_name.at(Idx++));

    return F;
}
} // namespace FunctionPrototype
