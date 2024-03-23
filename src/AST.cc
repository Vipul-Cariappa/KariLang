#include "AST.hh"

#define GOOD_SEMANTICS(t)                                                      \
    {                                                                          \
        this->result_type = t;                                                 \
        this->semantics_verified = true;                                       \
        this->semantics_correct = true;                                        \
        return true;                                                           \
    }

#define BAD_SEMANTICS_MSG(msg)                                                 \
    {                                                                          \
        std::cerr << "Semantic Error: " << msg << "\n";                        \
        this->result_type = TYPE::INT_T;                                       \
        this->semantics_verified = true;                                       \
        this->semantics_correct = false;                                       \
        return false;                                                          \
    }

#define BAD_SEMANTICS()                                                        \
    {                                                                          \
        this->result_type = TYPE::INT_T;                                       \
        this->semantics_verified = true;                                       \
        this->semantics_correct = false;                                       \
        return false;                                                          \
    }

TYPE Expression::deduce_result_type() {
    switch (type) {
    case INTEGER_EXP:
        return TYPE::INT_T;
    case BOOLEAN_EXP:
        return TYPE::BOOL_T;
    case VARIABLE_EXP:
        if (globals_ast.find(std::get<std::string>(value)) != globals_ast.end())
            return globals_ast.at(std::get<std::string>(value))->type;
        return TYPE::INT_T; /* Hopefully sematic verification will catch the
                               error */
    case UNARY_OP_EXP:
        return std::get<std::unique_ptr<UnaryOperator>>(value)
            ->deduce_result_type();
    case BINARY_OP_EXP:
        return std::get<std::unique_ptr<BinaryOperator>>(value)
            ->deduce_result_type();
    case IF_EXP:
        return std::get<std::unique_ptr<IfOperator>>(value)
            ->deduce_result_type();
    case FUNCTION_CALL_EXP:
        return std::get<std::unique_ptr<FunctionCall>>(value)
            ->deduce_result_type();
    }
}

bool Expression::verify_semantics(
    TYPE result_type,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, TYPE> &context) {
    switch (type) {
    case INTEGER_EXP:
        if (result_type == TYPE::INT_T)
            GOOD_SEMANTICS(result_type)
        BAD_SEMANTICS_MSG("Expected type " << ToString(result_type)
                                           << " but got int")
    case BOOLEAN_EXP:
        if (result_type == TYPE::BOOL_T)
            GOOD_SEMANTICS(result_type)
        BAD_SEMANTICS_MSG("Expected type " << ToString(result_type)
                                           << " but got bool")
    case VARIABLE_EXP:
        if (globals_ast.find(std::get<std::string>(value)) !=
            globals_ast.end()) {
            if (globals_ast.at(std::get<std::string>(value))->type ==
                result_type)
                GOOD_SEMANTICS(result_type)
            else
                BAD_SEMANTICS_MSG(
                    "Variable type "
                    << ToString(
                           globals_ast.at(std::get<std::string>(value))->type)
                    << " but " << ToString(result_type) << "expected")
        }
        if (context.find(std::get<std::string>(value)) != context.end()) {
            if (context.at(std::get<std::string>(value)) == result_type)
                GOOD_SEMANTICS(result_type)
            else
                BAD_SEMANTICS_MSG(
                    "Variable type "
                    << ToString(context.at(std::get<std::string>(value)))
                    << " but " << ToString(result_type) << "expected")
        }
        BAD_SEMANTICS_MSG("Could not find \"" << std::get<std::string>(value)
                                              << "\" variable")
    case UNARY_OP_EXP:
        if (std::get<std::unique_ptr<UnaryOperator>>(value)->verify_semantics(
                result_type, functions_ast, globals_ast, context))
            GOOD_SEMANTICS(result_type);
        BAD_SEMANTICS()
    case BINARY_OP_EXP:
        if (std::get<std::unique_ptr<BinaryOperator>>(value)->verify_semantics(
                result_type, functions_ast, globals_ast, context))
            GOOD_SEMANTICS(result_type);
        BAD_SEMANTICS()
    case IF_EXP:
        if (std::get<std::unique_ptr<IfOperator>>(value)->verify_semantics(
                result_type, functions_ast, globals_ast, context))
            GOOD_SEMANTICS(result_type);
        BAD_SEMANTICS()
    case FUNCTION_CALL_EXP:
        if (std::get<std::unique_ptr<FunctionCall>>(value)->verify_semantics(
                result_type, functions_ast, globals_ast, context))
            GOOD_SEMANTICS(result_type);
        BAD_SEMANTICS()
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

TYPE UnaryOperator::deduce_result_type() {
    switch (op_type) {
    case NOT_OP:
        return TYPE::BOOL_T;
    case NEG_OP:
        return TYPE::INT_T;
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
        if (result_type == TYPE::BOOL_T) {
            if (fst->verify_semantics(TYPE::BOOL_T, functions_ast, globals_ast,
                                      context))
                GOOD_SEMANTICS(result_type)
            BAD_SEMANTICS()
        }
        BAD_SEMANTICS_MSG("Expected " << ToString(result_type)
                                      << " but got bool (unary not operator)")
    case NEG_OP:
        if (result_type == TYPE::INT_T) {
            if (fst->verify_semantics(TYPE::INT_T, functions_ast, globals_ast,
                                      context))
                GOOD_SEMANTICS(result_type)
            BAD_SEMANTICS()
        }
        BAD_SEMANTICS_MSG("Expected "
                          << ToString(result_type)
                          << " but got int (unary negation operator)")
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

TYPE BinaryOperator::deduce_result_type() {
    switch (op_type) {
    case ADD_OP:
    case MUL_OP:
    case DIV_OP:
    case MOD_OP:
        return TYPE::INT_T;
    case AND_OP:
    case OR_OP:
    case EQS_OP:
    case NEQ_OP:
    case GT_OP:
    case GTE_OP:
    case LT_OP:
    case LTE_OP:
        return TYPE::BOOL_T;
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
        if (result_type == TYPE::INT_T) {
            if (fst->verify_semantics(TYPE::INT_T, functions_ast, globals_ast,
                                      context) &&
                snd->verify_semantics(TYPE::INT_T, functions_ast, globals_ast,
                                      context))
                GOOD_SEMANTICS(result_type)
            BAD_SEMANTICS()
        }
        BAD_SEMANTICS_MSG("Expected "
                          << ToString(result_type)
                          << " but got int (binary arithmetic operator)")
    case AND_OP:
    case OR_OP:
        if (result_type == TYPE::BOOL_T) {
            if (fst->verify_semantics(TYPE::BOOL_T, functions_ast, globals_ast,
                                      context) &&
                snd->verify_semantics(TYPE::BOOL_T, functions_ast, globals_ast,
                                      context))
                GOOD_SEMANTICS(result_type)
            BAD_SEMANTICS()
        }
        BAD_SEMANTICS_MSG("Expected "
                          << ToString(result_type)
                          << " but got bool (binary logical operator)")
    case EQS_OP:
    case NEQ_OP:
    case GT_OP:
    case GTE_OP:
    case LT_OP:
    case LTE_OP:
        if (result_type == TYPE::BOOL_T) {
            if (fst->verify_semantics(TYPE::INT_T, functions_ast, globals_ast,
                                      context) &&
                snd->verify_semantics(TYPE::INT_T, functions_ast, globals_ast,
                                      context))
                GOOD_SEMANTICS(result_type)
            BAD_SEMANTICS()
        }
        BAD_SEMANTICS_MSG("Expected "
                          << ToString(result_type)
                          << " but got bool (binary comparision operator)")
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

TYPE IfOperator::deduce_result_type() { return yes->deduce_result_type(); }

bool IfOperator::verify_semantics(
    TYPE result_type,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, TYPE> &context) {
    if (!cond->verify_semantics(TYPE::BOOL_T, functions_ast, globals_ast,
                                context))
        BAD_SEMANTICS_MSG("Condition does not yield a boolean value");
    if (yes->verify_semantics(result_type, functions_ast, globals_ast,
                              context) &&
        no->verify_semantics(result_type, functions_ast, globals_ast, context))
        GOOD_SEMANTICS(result_type)
    BAD_SEMANTICS_MSG(
        "Conditional branches does not yield the expected type of "
        << ToString(result_type));
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

TYPE FunctionCall::deduce_result_type() {
    if (functions_ast.find(function_name) != functions_ast.end()) {
        std::unique_ptr<FunctionDef> &func = functions_ast.at(function_name);
        return func->return_type;
    }
    return TYPE::INT_T; /* Hopefully sematic verification will catch the error
                         */
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
            BAD_SEMANTICS_MSG("Expected "
                              << func->args_type.size()
                              << " number of arguments but supplied "
                              << args.size() << " arguments")

        if (func->return_type != result_type)
            BAD_SEMANTICS_MSG("Expected return type to be "
                              << ToString(result_type)
                              << " but actual return type is "
                              << ToString(func->return_type))

        for (size_t i = 0; i < args.size(); i++) {
            if (!args[i]->verify_semantics(func->args_type[i], functions_ast,
                                           globals_ast, context))
                BAD_SEMANTICS_MSG("\t in function arguments");
        }
        GOOD_SEMANTICS(result_type);
    }
    BAD_SEMANTICS_MSG("Could not find \"" << function_name << "\" function")
}

std::variant<bool, int> FunctionCall::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    // TODO: change context's type to std::unique_ptr<Expression>
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
    if (expression->verify_semantics(type, functions_ast, globals_ast, r)) {
        semantics_verified = true;
        semantics_correct = true;
        return true;
    }
    semantics_verified = true;
    semantics_correct = false;
    return false;
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
    if (expression->verify_semantics(return_type, functions_ast, globals_ast,
                                     context)) {
        semantics_verified = true;
        semantics_correct = true;
        return true;
    }
    semantics_verified = true;
    semantics_correct = false;
    return false;
}

std::variant<bool, int> FunctionDef::interpret(
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
        &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
    std::unordered_map<std::string, std::variant<bool, int>> &context) {
    return expression->interpret(functions_ast, globals_ast, context);
}
