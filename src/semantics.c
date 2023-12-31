#include "common.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ERROR_MSG_LEN 500

typedef struct {
    size_t arglen;
    Argument *args;
} Context;

bool verify_function_semantics(Function *func);
bool verify_variable_semantics(Variable *var);
bool verify_function_call_arguments(Function *func, Expression **args,
                                    size_t arglen, Context *cxt);
bool verify_expression_type(Expression *exp, Type type, Context *cxt);
bool verify_ast_semantics(AST *tree);

char semantic_error_msg[ERROR_MSG_LEN] = {0};

// TODO: improve error message with line number

bool verify_semantics() {
    char *key;
    AST *tree;
    ast_table_iter(ast);

    while (NULL != (tree = ast_table_iter_next(ast, &key))) {
        if (tree->semantically_correct)
            continue;
        if (!verify_ast_semantics(tree))
            return false;
    }

    return true;
}

bool verify_ast_semantics(AST *tree) {
    if (tree->semantically_correct)
        return true;

    switch (tree->type) {
    case AST_FUNCTION:
        if (!verify_function_semantics(tree->value.func)) {
            return false;
        }
        break;
    case AST_VARIABLE:
        if (!verify_variable_semantics(tree->value.var)) {
            return false;
        }
        break;
    case AST_EXPRESSION:
        snprintf(semantic_error_msg, ERROR_MSG_LEN, "Internal Error");
        return false;
    }
    tree->semantically_correct = true;
    return tree;
}

bool verify_function_call_arguments(Function *func, Expression **args,
                                    size_t arglen, Context *cxt) {
    if (func->arglen != arglen) {
        snprintf(semantic_error_msg, ERROR_MSG_LEN,
                 "Parameter count and argument count mismatch");
        return false;
    }

    for (size_t i = 0; i < func->arglen; i++) {
        if (!verify_expression_type(args[i], func->args[i].type, cxt)) {
            snprintf(semantic_error_msg, ERROR_MSG_LEN,
                     "Expected argument type %s, but got other type",
                     Type_to_string(func->args[i].type));
            return false;
        }
    }
    return true;
}

bool verify_expression_type(Expression *exp, Type type, Context *cxt) {
    switch (exp->type) {
    case INTEGER_EXPRESSION:
        if (type == INT)
            return true;
        goto expected_int_error;
    case VARIABLE_EXPRESSION: {
        AST *variable_ast = ast_table_get_ptr(ast, exp->value.variable);
        if (variable_ast) {
            if (variable_ast->type == AST_VARIABLE) {
                if (variable_ast->value.var->type == type) {
                    return true;
                }
                snprintf(semantic_error_msg, ERROR_MSG_LEN,
                         "%s has type %s, but expected %s type",
                         variable_ast->value.var->name,
                         Type_to_string(variable_ast->value.var->type),
                         Type_to_string(type));
                return false;
            }
            snprintf(semantic_error_msg, ERROR_MSG_LEN,
                     "%s is not a variable definition",
                     variable_ast->value.var->name);
            return false;
        }

        if (!cxt)
            goto variable_not_found_error;

        // TODO: try changing the below to something other then linear search
        for (size_t i = 0; i < cxt->arglen; i++) {
            if (!strcmp(exp->value.variable, cxt->args[i].name)) {
                if (cxt->args[i].type == type) {
                    return true;
                }
            }
        }

    variable_not_found_error:
        snprintf(semantic_error_msg, ERROR_MSG_LEN,
                 "Could not find %s's variable definition",
                 variable_ast->value.var->name);
        return false;
    }
    case BOOLEAN_EXPRESSION:
        if (type == BOOL)
            return true;
        goto expected_bool_error;
    case PLUS_EXPRESSION:
    case MULTIPLY_EXPRESSION:
    case DIVIDE_EXPRESSION:
    case MODULO_EXPRESSION:
        if (type == INT) {
            if (verify_expression_type(exp->value.binary.fst, INT, cxt) &&
                verify_expression_type(exp->value.binary.snd, INT, cxt))
                return true;
        }
        goto expected_int_error;
    case MINUS_EXPRESSION:
        if (type == INT) {
            if (verify_expression_type(exp->value.unary.fst, INT, cxt))
                return true;
        }
        goto expected_int_error;
    case AND_EXPRESSION:
    case OR_EXPRESSION:
        if (type == BOOL) {
            if (verify_expression_type(exp->value.binary.fst, BOOL, cxt) &&
                verify_expression_type(exp->value.binary.snd, BOOL, cxt))
                return true;
        }
        goto expected_bool_error;
    case NOT_EXPRESSION:
        if (type == BOOL) {
            if (verify_expression_type(exp->value.unary.fst, BOOL, cxt))
                return true;
        }
        goto expected_bool_error;
    case EQUALS_EXPRESSION:
    case NOT_EQUALS_EXPRESSION:
    case GREATER_EXPRESSION:
    case GREATER_EQUALS_EXPRESSION:
    case LESSER_EXPRESSION:
    case LESSER_EQUALS_EXPRESSION:
        if (type == BOOL) {
            if (verify_expression_type(exp->value.binary.fst, INT, cxt) &&
                verify_expression_type(exp->value.binary.snd, INT, cxt))
                return true;
            goto expected_int_error;
        }
        goto expected_bool_error;
    case IF_EXPRESSION:
        if (verify_expression_type(exp->value.if_statement.condition, BOOL,
                                   cxt)) {
            if (type == INT) {
                if (verify_expression_type(exp->value.if_statement.yes, INT,
                                           cxt) &&
                    verify_expression_type(exp->value.if_statement.no, INT,
                                           cxt))
                    return true;
                goto expected_int_error;
            }
            if (type == BOOL) {
                if (verify_expression_type(exp->value.if_statement.yes, BOOL,
                                           cxt) &&
                    verify_expression_type(exp->value.if_statement.no, BOOL,
                                           cxt))
                    return true;
                goto expected_bool_error;
            }
        }
        goto expected_bool_error;
    case FUNCTION_CALL_EXPRESSION: {
        AST *func_ast =
            ast_table_get_ptr(ast, exp->value.function_call.funcname);
        if (!func_ast)
            return false;

        if (func_ast->value.func->return_type == type) {
            if (verify_function_call_arguments(
                    func_ast->value.func, exp->value.function_call.args,
                    exp->value.function_call.arglen, cxt))
                return true;
            return false;
        }
        snprintf(semantic_error_msg, ERROR_MSG_LEN,
                 "Function return type is not the expected type");
        return false;
    }

    default:
        printf("This should not be printed\n");
        return false;
    }

expected_bool_error:
    snprintf(semantic_error_msg, ERROR_MSG_LEN,
             "Expected bool type but got other type");
    return false;

expected_int_error:
    snprintf(semantic_error_msg, ERROR_MSG_LEN,
             "Expected int type but got other type");
    return false;
}

bool verify_function_semantics(Function *func) {
    Context cxt = {.arglen = func->arglen, .args = func->args};
    return verify_expression_type(func->expression, func->return_type, &cxt);
}

bool verify_variable_semantics(Variable *var) {
    if (verify_expression_type(var->expression, var->type, NULL))
        return true;

    snprintf(semantic_error_msg, ERROR_MSG_LEN,
             "Expected %s to be %s type but got other type", var->name,
             Type_to_string(var->type));
    return false;
}
