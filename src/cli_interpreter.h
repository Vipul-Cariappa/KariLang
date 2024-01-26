#include "common.h"
#include <stdarg.h>

typedef struct {
    size_t arglen;
    Argument *args;
} Context;

ExpressionResult evaluate_expression(Expression *exp, Context *cxt);
bool verify_expression_type(Expression *exp, Type type, Context *cxt);
bool verify_ast_semantics(AST *tree);
static inline int my_print(FILE *file, const char *msg, ...);

static inline bool cli_interpret(AST tree) {
    if (tree.type == AST_VARIABLE) {
        if (ast_table_get_ptr(ast, tree.value.var->name)) {
            ast_table_delete(ast, tree.value.var->name);
            global_table_delete(globals, tree.value.var->name);
            errno = 0;
        }
        if (!ast_table_insert(ast, tree.value.var->name, tree)) {
            my_print(stderr, "AST insertion Error\n");
            errno = 0;
            return false;
        }
    } else if (tree.type == AST_FUNCTION) {
        if (ast_table_get_ptr(ast, tree.value.func->funcname)) {
            ast_table_delete(ast, tree.value.func->funcname);
            // ???: is the below deletion necessary here
            global_table_delete(globals, tree.value.func->funcname);
            errno = 0;
        }
        if (!ast_table_insert(ast, tree.value.func->funcname, tree)) {
            my_print(stderr, "AST insertion Error\n");
            errno = 0;
            return false;
        }
    } else {
        if (verify_expression_type(tree.value.exp, BOOL, NULL)) {
            my_print(stdout, evaluate_expression(tree.value.exp, NULL).boolean
                                 ? "true\n"
                                 : "false\n");
            return true;
        }
        if (verify_expression_type(tree.value.exp, INT, NULL)) {
            my_print(stdout, "%d\n",
                     evaluate_expression(tree.value.exp, NULL).integer);
            return true;
        }

        my_print(stderr,
                 "Error While Evaluation Expression\nSemantic Error: %s\n",
                 semantic_error_msg);
        semantic_error_msg[0] = 0;
        return false;
    }

    if (!verify_ast_semantics(&tree)) {
        my_print(stderr, "Semantic Error: %s\n", semantic_error_msg);
        semantic_error_msg[0] = 0;
        return false;
    }

    if (tree.type == AST_VARIABLE) {
        assert(global_table_insert(
            globals, tree.value.var->name,
            (global){.value =
                         evaluate_expression(tree.value.var->expression, NULL),
                     .type = tree.value.var->type}));
    }
    return true;
}

static inline int my_print(FILE *file, const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    int len = 0;

    if ((file == stdout) && (STDOUT_REDIRECT_STRING)) {
        if (STDOUT_REDIRECT_STRING)
            len = vsnprintf(STDOUT_REDIRECT_STRING, STDOUT_STRING_LENGTH, msg,
                            args);
        else
            len = vprintf(msg, args);
    } else {
        if (STDERR_REDIRECT_STRING)
            len = vsnprintf(STDERR_REDIRECT_STRING, STDERR_STRING_LENGTH, msg,
                            args);
        else
            len = vfprintf(stderr, msg, args);
    }

    va_end(args);
    return len;
}
