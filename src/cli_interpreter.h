#include "common.h"

typedef union {
    int integer;
    bool boolean;
} ExpressionResult;

typedef struct {
    size_t arglen;
    Argument *args;
} Context;

ExpressionResult evaluate_expression(Expression *exp, Context *cxt);
bool verify_expression_type(Expression *exp, Type type, Context *cxt);
bool verify_ast_semantics(AST *tree);

static inline bool cli_interpret(AST tree) {
    if (tree.type == AST_VARIABLE) {
        if (ast_table_get_ptr(ast, tree.value.var->name)) {
            ast_table_delete(ast, tree.value.var->name);
            integer_table_delete(globalIntegers, tree.value.var->name);
            boolean_table_delete(globalBooleans, tree.value.var->name);
            errno = 0;
        }
        if (!ast_table_insert(ast, tree.value.var->name, tree)) {
            fprintf(stderr, "AST insertion Error\n");
            errno = 0;
            return false;
        }
    } else if (tree.type == AST_FUNCTION) {
        if (ast_table_get_ptr(ast, tree.value.func->funcname)) {
            ast_table_delete(ast, tree.value.func->funcname);
            integer_table_delete(globalIntegers, tree.value.func->funcname);
            boolean_table_delete(globalBooleans, tree.value.func->funcname);
            errno = 0;
        }
        if (!ast_table_insert(ast, tree.value.func->funcname, tree)) {
            fprintf(stderr, "AST insertion Error\n");
            errno = 0;
            return false;
        }
    } else {
        if (verify_expression_type(tree.value.exp, BOOL, NULL)) {
            printf(evaluate_expression(tree.value.exp, NULL).boolean
                       ? "true\n"
                       : "false\n");
            return true;
        }
        if (verify_expression_type(tree.value.exp, INT, NULL)) {
            printf("%d\n", evaluate_expression(tree.value.exp, NULL).integer);
            return true;
        }

        fprintf(stderr,
                "Error While Evaluation Expression\nSemantic Error: %s\n",
                semantic_error_msg);
        semantic_error_msg[0] = 0;
        return false;
    }

    if (!verify_ast_semantics(&tree)) {
        fprintf(stderr, "Semantic Error: %s\n", semantic_error_msg);
        semantic_error_msg[0] = 0;
        return false;
    }

    if (tree.type == AST_VARIABLE) {
        if (tree.value.var->type == INT) {
            assert(integer_table_insert(
                globalIntegers, tree.value.var->name,
                evaluate_expression(tree.value.var->expression, NULL).integer));
        } else {
            assert(boolean_table_insert(
                globalBooleans, tree.value.var->name,
                evaluate_expression(tree.value.var->expression, NULL).boolean));
        }
    }

    return true;
}
