#include "AST.hh"
#include "Compile.hh"
#include "JIT.hh"
#include "llvm/Support/TargetSelect.h"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::unique_ptr<FunctionDef>> functions_ast;
std::unordered_map<std::string, std::unique_ptr<ValueDef>> globals_ast;

int parse(
    std::string filename, LANGUAGE_ACTION_TYPE comp_flags,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast);

int interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    int input);

int main(int argc, char *argv[]) {
    int status;

    if ((argc == 3) && (std::string(argv[2]) == "-c")) {
        status = parse(argv[1], COMPILE, functions_ast, globals_ast);
    } else if ((argc == 2) && ((std::string(argv[1]) == "-jit") ||
                               (std::string(argv[1]) == "--jit"))) {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
        TheJIT = ExitOnErr(KariLangJIT::Create());
        jit::jit();
        std::cout << ">>> ";
        return parse("", JIT, functions_ast, globals_ast);
    } else if (argc == 1) {
        std::cout << ">>> ";
        return parse("", INTERPRET, functions_ast, globals_ast);
    } else {
        std::cerr << "File and input required to execute the program\n";
        std::cerr << "Or pass \"-c\" flag to compile\n";
        return 1;
    }

    if (status) {
        std::cerr << "Error while parsing. Exiting\n";
        return 1;
    }

    // verify semantics
    for (auto &i : functions_ast) {
        if (!i.second->verify_semantics(functions_ast, globals_ast)) {
            std::cerr << "Invalid semantics for function " << i.first << "\n\t"
                      << i.second << "\n";
            return 1;
        }
    }
    for (auto &i : globals_ast) {
        if (!i.second->verify_semantics(functions_ast, globals_ast)) {
            std::cerr << "Invalid semantics for variable " << i.first << "\n\t"
                      << i.second << "\n";
            return 1;
        }
    }

    if (std::string(argv[2]) == "-c")
        return Compile(argv[1], functions_ast, globals_ast);
    return interpret(functions_ast, globals_ast, std::stoi(argv[2]));
}

int interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    int input) {
    // interpret
    if (functions_ast.find("main") != functions_ast.end()) {
        std::unique_ptr<FunctionDef> &func_main = functions_ast.at("main");

        if ((func_main->args_name.size() != 1) &&
            (func_main->args_type.at(0) != INT_T) &&
            (func_main->return_type != INT_T)) {
            std::cerr << "main function can take only 1 int type argument and "
                         "should return int\n";
            return 1;
        }

        std::unordered_map<std::string, std::variant<bool, int>> context;
        context.insert({func_main->args_name.at(0), input});
        int res = std::get<int>(
            func_main->interpret(functions_ast, globals_ast, context));
        std::cout << "Input: " << input << "\nOutput: " << res << "\n";
    } else {
        std::cerr << "Error: Could not find main function\n";
        return 1;
    }

    return 0;
}
