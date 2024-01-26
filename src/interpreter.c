#include "DS.h"
#include "common.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#define ERROR_MSG_LEN 500

char runtime_error_msg[ERROR_MSG_LEN];

struct _context {
    const char *var_name;
    ExpressionResult var_value;
};

typedef struct {
    size_t len;
    struct _context *variable;
} Context;

DS_TABLE_DEF(global, global, free_global);

ExpressionResult evaluate_expression(Expression *exp, Context *cxt);
ExpressionResult execute_function_call(Function *func, Expression **args,
                                       Context *cxt);

global_table_t *globals;

bool interpret(int input, int *output) {
    // initialize global variables table
    globals = global_table_new(100);

    Function *main_func = NULL;
    char *key;
    AST *tree;
    ast_table_iter(ast);

    while (NULL != (tree = ast_table_iter_next(ast, &key))) {
        switch (tree->type) {
        case AST_VARIABLE:
            if (global_table_get_ptr(globals, tree->value.var->name)) {
                break;
            }
            errno = 0;

            assert(global_table_insert(
                globals, tree->value.var->name,
                (global){.value = evaluate_expression(
                             tree->value.var->expression, NULL),
                         .type = tree->value.var->type}));
            break;

        case AST_FUNCTION:
            if (!strcmp(tree->value.func->funcname, "main")) {
                main_func = tree->value.func;
                if (main_func->return_type != INT) {
                    snprintf(runtime_error_msg, ERROR_MSG_LEN, "%s",
                             "'main' function should return an integer");
                    return false;
                }
                if ((main_func->arglen != 1) ||
                    (main_func->args[0].type != INT)) {
                    snprintf(
                        runtime_error_msg, ERROR_MSG_LEN, "%s",
                        "'main' function should have only 1 integer argument");
                    return false;
                }
            }
            break;
        case AST_EXPRESSION:
            snprintf(syntax_error_msg, ERROR_MSG_LEN, "Internal Error");
            // TODO: clean memory
            return false;
        }
    }

    if (!main_func) {
        snprintf(runtime_error_msg, ERROR_MSG_LEN, "%s",
                 "Could not find 'main' function");
        return false;
    }

    struct _context args[1] = {
        {.var_name = main_func->args[0].name,
         .var_value = (ExpressionResult){.integer = input}}};
    Context cxt = {.len = 1, .variable = args};

    *output = evaluate_expression(main_func->expression, &cxt).integer;
    return true;
}

ExpressionResult evaluate_expression(Expression *exp, Context *cxt) {
    switch (exp->type) {
    case INTEGER_EXPRESSION:
        return (ExpressionResult){.integer = exp->value.integer};
    case BOOLEAN_EXPRESSION:
        return (ExpressionResult){.boolean = exp->value.boolean};
    case VARIABLE_EXPRESSION: {
        // check the context
        if (cxt) {
            for (size_t i = 0; i < cxt->len; i++) {
                if (!strcmp(cxt->variable[i].var_name, exp->value.variable))
                    return cxt->variable[i].var_value;
            }
        }

        // check the global variables
        global *v = global_table_get_ptr(globals, exp->value.variable);
        if (v)
            return v->value;
        errno = 0;
        AST *tree = ast_table_get_ptr(ast, exp->value.variable);
        if (tree) {
            ExpressionResult result_exp =
                evaluate_expression(tree->value.var->expression, NULL);
            assert(global_table_insert(
                globals, tree->value.var->name,
                (global){.type = tree->value.var->type, .value = result_exp}));
            
            return result_exp;
        }

        goto error;
    }
    case PLUS_EXPRESSION:
        return (ExpressionResult){
            .integer = evaluate_expression(exp->value.binary.fst, cxt).integer +
                       evaluate_expression(exp->value.binary.snd, cxt).integer};
    case MINUS_EXPRESSION:
        return (ExpressionResult){
            .integer =
                -(evaluate_expression(exp->value.unary.fst, cxt).integer)};
    case MULTIPLY_EXPRESSION:
        return (ExpressionResult){
            .integer = evaluate_expression(exp->value.binary.fst, cxt).integer *
                       evaluate_expression(exp->value.binary.snd, cxt).integer};
    case DIVIDE_EXPRESSION:
        return (ExpressionResult){
            .integer = evaluate_expression(exp->value.binary.fst, cxt).integer /
                       evaluate_expression(exp->value.binary.snd, cxt).integer};
    case MODULO_EXPRESSION:
        return (ExpressionResult){
            .integer = evaluate_expression(exp->value.binary.fst, cxt).integer %
                       evaluate_expression(exp->value.binary.snd, cxt).integer};
    case AND_EXPRESSION:
        return (ExpressionResult){
            .boolean =
                evaluate_expression(exp->value.binary.fst, cxt).boolean &&
                evaluate_expression(exp->value.binary.snd, cxt).boolean};
    case OR_EXPRESSION:
        return (ExpressionResult){
            .boolean =
                evaluate_expression(exp->value.binary.fst, cxt).boolean ||
                evaluate_expression(exp->value.binary.snd, cxt).boolean};
    case NOT_EXPRESSION:
        return (ExpressionResult){
            .boolean =
                !(evaluate_expression(exp->value.unary.fst, cxt).boolean)};
    case EQUALS_EXPRESSION:
        return (ExpressionResult){
            .boolean =
                evaluate_expression(exp->value.binary.fst, cxt).integer ==
                evaluate_expression(exp->value.binary.snd, cxt).integer};
    case NOT_EQUALS_EXPRESSION:
        return (ExpressionResult){
            .boolean =
                evaluate_expression(exp->value.binary.fst, cxt).integer !=
                evaluate_expression(exp->value.binary.snd, cxt).integer};
    case GREATER_EXPRESSION:
        return (ExpressionResult){
            .boolean = evaluate_expression(exp->value.binary.fst, cxt).integer >
                       evaluate_expression(exp->value.binary.snd, cxt).integer};
    case GREATER_EQUALS_EXPRESSION:
        return (ExpressionResult){
            .boolean =
                evaluate_expression(exp->value.binary.fst, cxt).integer >=
                evaluate_expression(exp->value.binary.snd, cxt).integer};
    case LESSER_EXPRESSION:
        return (ExpressionResult){
            .boolean = evaluate_expression(exp->value.binary.fst, cxt).integer <
                       evaluate_expression(exp->value.binary.snd, cxt).integer};
    case LESSER_EQUALS_EXPRESSION:
        return (ExpressionResult){
            .boolean =
                evaluate_expression(exp->value.binary.fst, cxt).integer <=
                evaluate_expression(exp->value.binary.snd, cxt).integer};
    case IF_EXPRESSION:
        if (evaluate_expression(exp->value.if_statement.condition, cxt).boolean)
            return evaluate_expression(exp->value.if_statement.yes, cxt);
        return evaluate_expression(exp->value.if_statement.no, cxt);
    case FUNCTION_CALL_EXPRESSION: {
        Function *f = ast_table_get(ast, exp->value.function_call.funcname)
                          .value.func; /* ???: direct dereferening the pointer
                                          with checking for NULL. Should be safe
                                          because of the semantic checker */
        return execute_function_call(f, exp->value.function_call.args, cxt);
    }
    default:
    error:
        fprintf(stderr, "Error Encounter while interpreting");
        exit(1);
    }
}

ExpressionResult execute_function_call(Function *func, Expression **args,
                                       Context *cxt) {
    Context new_context = {.len = func->arglen};

    new_context.variable = calloc(func->arglen, sizeof(struct _context));
    if (!new_context.variable) {
        fprintf(stderr, "Error Encounter while interpreting (Memory Error)");
        exit(1);
    }

    for (size_t i = 0; i < new_context.len; i++) {
        new_context.variable[i] =
            (struct _context){.var_name = func->args[i].name,
                              .var_value = evaluate_expression(args[i], cxt)};
    }

    ExpressionResult result =
        evaluate_expression(func->expression, &new_context);
    free(new_context.variable);

    return result;
}
