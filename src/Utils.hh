#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::unique_ptr<T> &x) {
    return os << *x;
}

class FunctionDef;
class ValueDef;

extern std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
    functions_ast;
extern std::unordered_map<std::string, std::unique_ptr<ValueDef>> globals_ast;
