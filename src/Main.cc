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
}
