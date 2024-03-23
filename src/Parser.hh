#include "AST.hh"
#include "Compile.hh"
#include "JIT.hh"
#include "Utils.hh"
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#define PROMPT "\n>>> "

inline void handle_expressions(LANGUAGE_ACTION_TYPE flags,
                               std::unique_ptr<Expression> exp) {

    if (flags & KARILANG_INTERACTIVE) {
        std::unordered_map<std::string, TYPE> semantics_context;
        TYPE result_type = exp->deduce_result_type();
        if (!exp->verify_semantics(result_type, functions_ast, globals_ast,
                                   semantics_context)) {
            std::cerr << "Invalid semantics for the given expression\n"
                      << PROMPT;
            return;
        }
    } else {
        std::cerr << "Cannot have expressions at top level\n";
        return;
    }

    if (flags == INTERACTIVE_INTERPRET) {
        std::unordered_map<std::string, std::variant<bool, int>>
            interpret_context;

        std::variant<bool, int> res =
            exp->interpret(functions_ast, globals_ast, interpret_context);
        switch (exp->result_type) {
        case INT_T:
            std::cout << std::get<int>(res);
            break;
        case BOOL_T:
            std::cout << (std::get<bool>(res) ? "true" : "false");
            break;
        }

    } else {
        jit::JIT_Expression(std::move(exp));
    }

    if (flags & KARILANG_INTERACTIVE)
        std::cout << PROMPT;
}

inline void handle_valdef(LANGUAGE_ACTION_TYPE flags,
                          std::unique_ptr<ValueDef> _val) {
    const std::string &name = _val->name;

    if (globals_ast.find(name) != globals_ast.end()) {
        std::cerr << "Cannot rewrite variable. Consider changing "
                     "variable name\n";
        if (flags & KARILANG_INTERACTIVE)
            std::cout << PROMPT;
    }

    globals_ast.insert({_val->name, std::move(_val)});
    std::unique_ptr<ValueDef> &val = globals_ast.at(name);

    // verify semantics if interactive
    if (flags & KARILANG_INTERACTIVE) {
        if (!val->verify_semantics(functions_ast, globals_ast)) {
            std::cerr << "Invalid semantics for the given variable\n" << PROMPT;
            globals_ast.erase(name);
            return;
        }
    }

    if (flags == INTERACTIVE_COMPILE) {
        val->generate_llvm_ir();
        ExitOnErr(TheJIT->addModule(llvm::orc::ThreadSafeModule(
            std::move(TheModule), std::move(TheContext))));
        jit::jit();
    }

    if (flags & KARILANG_INTERACTIVE)
        std::cout << val << "\n" << PROMPT;
}

inline void handle_funcdef(LANGUAGE_ACTION_TYPE flags,
                           std::unique_ptr<FunctionDef> _func) {
    const std::string &name = _func->name;

    if (functions_ast.find(name) != functions_ast.end()) {
        std::cerr << "Cannot rewrite variable. Consider changing "
                     "function name\n";
        if (flags & KARILANG_INTERACTIVE)
            std::cout << PROMPT;
    }

    functions_ast.insert({_func->name, std::move(_func)});
    std::unique_ptr<FunctionDef> &func = functions_ast.at(name);

    // verify semantics if interactive
    if (flags & KARILANG_INTERACTIVE) {
        if (!func->verify_semantics(functions_ast, globals_ast)) {
            std::cerr << "Invalid semantics for the given function\n" << PROMPT;
            functions_ast.erase(name);
            return;
        }
    }

    if (flags == INTERACTIVE_COMPILE) {
        FunctionPrototype::generate_llvm_ir(func->name, func->args_name,
                                            func->args_type, func->return_type);
        func->generate_llvm_ir();
        ExitOnErr(TheJIT->addModule(llvm::orc::ThreadSafeModule(
            std::move(TheModule), std::move(TheContext))));
        jit::jit();
    }

    if (flags & KARILANG_INTERACTIVE)
        std::cout << func << "\n" << PROMPT;
}
