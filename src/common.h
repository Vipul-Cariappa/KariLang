#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "DS.h"

extern FILE *yyin;
extern int yylex(void);
extern int yyparse(void);
extern int yywrap(void);
extern void yyerror(char const *s);
extern int yylineno;
extern char *yytext;

typedef enum {
    UNDEFINED,
    INTEGER_EXPRESSION,
    VARIABLE_EXPRESSION,
    BOOLEAN_EXPRESSION,
    PLUS_EXPRESSION,
    MINUS_EXPRESSION,
    MULTIPLY_EXPRESSION,
    DIVIDE_EXPRESSION,
    MODULO_EXPRESSION,
    AND_EXPRESSION,
    OR_EXPRESSION,
    NOT_EXPRESSION,
    EQUALS_EXPRESSION,
    NOT_EQUALS_EXPRESSION,
    GREATER_EXPRESSION,
    GREATER_EQUALS_EXPRESSION,
    LESSER_EXPRESSION,
    LESSER_EQUALS_EXPRESSION,
    IF_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
} ExpressionType;

typedef enum {
    BOOL,
    INT,
} Type;

typedef union _ExpressionValue ExpressionValue;
typedef struct _Expression Expression;
typedef struct _Variable Variable;
typedef struct _Function Function;

struct _Variable {
    Type type;
    const char *name;
    Expression *expression;
};

typedef struct {
    const char *name;
    Type type;
} Argument;

struct _Function {
    const char *funcname;
    Type return_type;
    Expression *expression;
    size_t arglen;
    Argument args[];
};

union _ExpressionValue {
    int integer;
    const char *variable;
    bool boolean;
    struct {
        Expression *fst;
    } unary;
    struct {
        Expression *fst;
        Expression *snd;
    } binary;
    struct {
        Expression *condition;
        Expression *yes;
        Expression *no;
    } if_statement;
    struct {
        const char *funcname;
        size_t arglen;
        Expression **args;
    } function_call;
};

struct _Expression {
    ExpressionType type;
    ExpressionValue value;
};

/* AST for Semantic Analysis and Evaluation */

typedef enum {
    AST_VARIABLE,
    AST_FUNCTION,
} AST_TYPE;

struct _AST {
    AST_TYPE type;
    union {
        Function *func;
        Variable *var;
    } value;
};

typedef struct _AST AST;

DS_TABLE_DEC(ast, AST);

extern ast_table_t *ast;
