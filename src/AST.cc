#include "AST.hh"

bool Expression::verify_semantics(
    TYPE result_type,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, TYPE> &context) {
    switch (type) {
    case INTEGER_EXP:
        if (result_type == TYPE::INT_T)
            return true;
        return false;
    case BOOLEAN_EXP:
        if (result_type == TYPE::BOOL_T)
            return true;
        return false;
    case VARIABLE_EXP:
        try {
            if (globals_ast.at(std::get<std::string>(value))->type ==
                result_type)
                return true;
        } catch (std::out_of_range e) {
        }
        try {
            if (context.at(std::get<std::string>(value)) == result_type)
                return true;
        } catch (std::out_of_range e) {
        }
        return false;
    case UNARY_OP_EXP:
        return std::get<std::unique_ptr<UnaryOperator>>(value)
            ->verify_semantics(result_type, functions_ast, globals_ast,
                               context);
    case BINARY_OP_EXP:
        return std::get<std::unique_ptr<BinaryOperator>>(value)
            ->verify_semantics(result_type, functions_ast, globals_ast,
                               context);
    case IF_EXP:
        return std::get<std::unique_ptr<IfOperator>>(value)->verify_semantics(
            result_type, functions_ast, globals_ast, context);
    case FUNCTION_CALL_EXP:
        return std::get<std::unique_ptr<FunctionCall>>(value)->verify_semantics(
            result_type, functions_ast, globals_ast, context);
    }
}

std::variant<bool, int>
Expression::interpret(/* ???: a context maybe required */) {
    // TODO: implement
    throw "Not Implemented";
}

void Expression::generate_llvm_ir() {
    // TODO: implement
    throw "Not Implemented";
}

bool UnaryOperator::verify_semantics(
    TYPE result_type,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, TYPE> &context) {
    switch (op_type) {
    case NOT_OP:
        if (result_type == TYPE::BOOL_T)
            return fst->verify_semantics(TYPE::BOOL_T, functions_ast,
                                         globals_ast, context);
        return false;
    case NEG_OP:
        if (result_type == TYPE::INT_T)
            return fst->verify_semantics(TYPE::INT_T, functions_ast,
                                         globals_ast, context);
        return false;
    }
}

std::variant<bool, int>
UnaryOperator::interpret(/* ???: a context maybe required */) {
    // TODO: implement
    throw "Not Implemented";
}

void UnaryOperator::generate_llvm_ir() {
    // TODO: implement
    throw "Not Implemented";
}

bool BinaryOperator::verify_semantics(
    TYPE result_type,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, TYPE> &context) {
    switch (op_type) {
    case ADD_OP:
    case MUL_OP:
    case DIV_OP:
    case MOD_OP:
        if (result_type == TYPE::INT_T)
            return fst->verify_semantics(TYPE::INT_T, functions_ast,
                                         globals_ast, context);
        return false;
    case AND_OP:
    case OR_OP:
        if (result_type == TYPE::BOOL_T)
            return fst->verify_semantics(TYPE::BOOL_T, functions_ast,
                                         globals_ast, context);
        return false;
    case EQS_OP:
    case NEQ_OP:
    case GT_OP:
    case GTE_OP:
    case LT_OP:
    case LTE_OP:
        if (result_type == TYPE::BOOL_T)
            return fst->verify_semantics(TYPE::INT_T, functions_ast,
                                         globals_ast, context);
        return false;
    }
}

std::variant<bool, int>
BinaryOperator::interpret(/* ???: a context maybe required */) {
    // TODO: implement
    throw "Not Implemented";
}

void BinaryOperator::generate_llvm_ir() {
    // TODO: implement
    throw "Not Implemented";
}

bool IfOperator::verify_semantics(
    TYPE result_type,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, TYPE> &context) {
    if (cond->verify_semantics(TYPE::INT_T, functions_ast, globals_ast,
                               context))
        return false;
    return yes->verify_semantics(result_type, functions_ast, globals_ast,
                                 context) &&
           no->verify_semantics(result_type, functions_ast, globals_ast,
                                context);
}

std::variant<bool, int>
IfOperator::interpret(/* ???: a context maybe required */) {
    // TODO: implement
    throw "Not Implemented";
}

void IfOperator::generate_llvm_ir() {
    // TODO: implement
    throw "Not Implemented";
}

bool FunctionCall::verify_semantics(
    TYPE result_type,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, TYPE> &context) {
    try {
        std::unique_ptr<FunctionDef> &func = functions_ast.at(function_name);
        if (func->args_type.size() != args.size())
            return false;

        if (func->return_type != result_type)
            return false;

        for (size_t i = 0; i < args.size(); i++) {
            if (!args[i]->verify_semantics(func->args_type[i], functions_ast,
                                           globals_ast, context))
                return false;
        }
        return true;
    } catch (std::out_of_range e) {
        return false;
    }
}

std::variant<bool, int>
FunctionCall::interpret(/* ???: a context maybe required */) {
    // TODO: implement
    throw "Not Implemented";
}

void FunctionCall::generate_llvm_ir() {
    // TODO: implement
    throw "Not Implemented";
}

bool ValueDef::verify_semantics(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast) {
    std::unordered_map<std::string, TYPE> r;
    return expression->verify_semantics(type, functions_ast, globals_ast, r);
}

std::variant<bool, int>
ValueDef::interpret(/* ???: a context maybe required */) {
    // TODO: implement
    throw "Not Implemented";
}

void ValueDef::generate_llvm_ir() {
    // TODO: implement
    throw "Not Implemented";
}

bool FunctionDef::verify_semantics(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast) {
    // creating a context
    std::unordered_map<std::string, TYPE> context;
    for (size_t i = 0; i < args_name.size(); i++) {
        context.insert({args_name[i], args_type[i]});
    }
    return expression->verify_semantics(return_type, functions_ast, globals_ast,
                                        context);
}

std::variant<bool, int>
FunctionDef::interpret(/* ???: a context maybe required */) {
    // TODO: implement
    throw "Not Implemented";
}

void FunctionDef::generate_llvm_ir() {
    // TODO: implement
    throw "Not Implemented";
}
