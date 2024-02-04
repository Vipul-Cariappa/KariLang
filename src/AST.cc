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
        if ((globals_ast.find(std::get<std::string>(value)) !=
             globals_ast.end()) &&
            (globals_ast.at(std::get<std::string>(value))->type == result_type))
            return true;
        if ((context.find(std::get<std::string>(value)) != context.end()) &&
            (context.at(std::get<std::string>(value)) == result_type))
            return true;
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

std::variant<bool, int> Expression::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    switch (type) {
    case INTEGER_EXP:
        return std::get<int>(value);
    case BOOLEAN_EXP:
        return std::get<bool>(value);
    case VARIABLE_EXP:
        if (globals_ast.find(std::get<std::string>(value)) !=
            globals_ast.end()) {
            return globals_ast.at(std::get<std::string>(value))
                ->interpret(functions_ast, globals_ast, context);
        } else {
            return context.at(std::get<std::string>(value));
        }
    case UNARY_OP_EXP:
        return std::get<std::unique_ptr<UnaryOperator>>(value)->interpret(
            functions_ast, globals_ast, context);
    case BINARY_OP_EXP:
        return std::get<std::unique_ptr<BinaryOperator>>(value)->interpret(
            functions_ast, globals_ast, context);
    case IF_EXP:
        return std::get<std::unique_ptr<IfOperator>>(value)->interpret(
            functions_ast, globals_ast, context);
    case FUNCTION_CALL_EXP:
        return std::get<std::unique_ptr<FunctionCall>>(value)->interpret(
            functions_ast, globals_ast, context);
    }
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

std::variant<bool, int> UnaryOperator::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    switch (op_type) {
    case NOT_OP:
        return std::get<bool>(
                   fst->interpret(functions_ast, globals_ast, context))
                   ? false
                   : true;
    case NEG_OP:
        return -(
            std::get<int>(fst->interpret(functions_ast, globals_ast, context)));
    }
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

std::variant<bool, int> BinaryOperator::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    switch (op_type) {
    case ADD_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) +
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case MUL_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) *
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case DIV_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) /
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case MOD_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) %
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case AND_OP:
        return (std::get<bool>(
                   fst->interpret(functions_ast, globals_ast, context))) &&
               (std::get<bool>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case OR_OP:
        return (std::get<bool>(
                   fst->interpret(functions_ast, globals_ast, context))) ||
               (std::get<bool>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case EQS_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) ==
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case NEQ_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) !=
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case GT_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) >
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case GTE_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) >=
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case LT_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) <
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    case LTE_OP:
        return (std::get<int>(
                   fst->interpret(functions_ast, globals_ast, context))) <=
               (std::get<int>(
                   snd->interpret(functions_ast, globals_ast, context)));
    }
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

std::variant<bool, int> IfOperator::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    return std::get<bool>(cond->interpret(functions_ast, globals_ast, context))
               ? yes->interpret(functions_ast, globals_ast, context)
               : no->interpret(functions_ast, globals_ast, context);
}

bool FunctionCall::verify_semantics(
    TYPE result_type,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, TYPE> &context) {
    if (functions_ast.find(function_name) != functions_ast.end()) {
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
    }
    return false;
}

std::variant<bool, int> FunctionCall::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    // TODO: change context 's type to std::unique_ptr<Expression>
    //        that will automatically change to lazy evaluation of arguments
    std::unordered_map<std::string, std::variant<bool, int>> my_context;
    std::unique_ptr<FunctionDef> &func = functions_ast.at(function_name);
    for (size_t i = 0; i < func->args_name.size(); i++) {
        my_context.insert(
            {func->args_name.at(i),
             args.at(i)->interpret(functions_ast, globals_ast, context)});
    }
    return func->interpret(functions_ast, globals_ast, my_context);
}

bool ValueDef::verify_semantics(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast) {
    std::unordered_map<std::string, TYPE> r;
    return expression->verify_semantics(type, functions_ast, globals_ast, r);
}

std::variant<bool, int> ValueDef::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    return expression->interpret(functions_ast, globals_ast, context);
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

std::variant<bool, int> FunctionDef::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    return expression->interpret(functions_ast, globals_ast, context);
}
