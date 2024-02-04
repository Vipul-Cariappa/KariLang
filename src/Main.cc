#include "AST.hh"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

int parse(
    std::string filename, bool compile,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast);

int main(int argc, char *argv[]) {
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>> functions_ast;
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> globals_ast;

    int status;

    if (argc == 3) {
        status = parse(argv[1], false, functions_ast, globals_ast);
    } else if (argc == 1) {
        // TODO: implement
        std::cerr << "Interactive repl not implemented\n";
        return 1;
    } else {
        std::cerr << "File and input required to execute the program\n";
        return 1;
    }

    if (status) {
        std::cerr << "Error while parsing. Exiting\n";
        return 1;
    }

    print(std::cout, functions_ast);
    print(std::cout, globals_ast);

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

    // interpret
    try {
        std::unique_ptr<FunctionDef> &func_main = functions_ast.at("main");
        
        if ((func_main->args_name.size() != 1) && (func_main->args_type.at(0) != INT_T)) {
            std::cerr << "main function can tak only 1 int type argument\n";
            return 1;
        }

        std::unordered_map<std::string, std::variant<int, bool>> context;
        context.insert({func_main->args_name.at(0), std::stoi(argv[2])});
        // func_main->interpret();

    } catch (std::out_of_range e) {
        std::cerr << "Error: Could not find main function\n";
        return 1;
    }

    return 0;
}
