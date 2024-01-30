#include "AST.hh"

bool Expression::verify_semantics() {
    semantics_verified = true;
    semantics_correct = false;
    return semantics_correct;
}

std::variant<bool, int>
Expression::interpret(/* ???: a context maybe required */) {
    return 5;
}

void Expression::generate_llvm_ir() {}

bool UnaryOperator::verify_semantics() {
    semantics_verified = true;
    semantics_correct = false;
    return semantics_correct;
}

std::variant<bool, int>
UnaryOperator::interpret(/* ???: a context maybe required */) {
    return 5;
}

void UnaryOperator::generate_llvm_ir() {}

bool BinaryOperator::verify_semantics() {
    semantics_verified = true;
    semantics_correct = false;
    return semantics_correct;
}

std::variant<bool, int>
BinaryOperator::interpret(/* ???: a context maybe required */) {
    return 5;
}

void BinaryOperator::generate_llvm_ir() {}

bool IfOperator::verify_semantics() {
    semantics_verified = true;
    semantics_correct = false;
    return semantics_correct;
}

std::variant<bool, int>
IfOperator::interpret(/* ???: a context maybe required */) {
    return 5;
}

void IfOperator::generate_llvm_ir() {}

bool FunctionCall::verify_semantics() {
    semantics_verified = true;
    semantics_correct = false;
    return semantics_correct;
}

std::variant<bool, int>
FunctionCall::interpret(/* ???: a context maybe required */) {
    return 5;
}

void FunctionCall::generate_llvm_ir() {}
