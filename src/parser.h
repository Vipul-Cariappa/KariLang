#include <stdio.h>
#include <stdlib.h>

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
} ExpressionType;

// typedef enum {
//     BOOL,
//     INT,
// } Type;

// typedef struct {
//     Type type;
//     const char *name;
// } Variable;

typedef union _ExpressionValue ExpressionValue;
typedef struct _Expression Expression;

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
};

struct _Expression {
    ExpressionType type;
    ExpressionValue value;
};

// static inline const char *const Type_to_string(Type type) {
//     switch (type) {
//     case BOOL:
//         return "bool";
//     case INT:
//         return "int";
//     default:
//         return "UNDEFINED TYPE";
//     }
// }

static inline Expression *make_integer_expression(int n) {
    Expression *result = malloc(sizeof(Expression));
    *result = (Expression){.type = INTEGER_EXPRESSION, .value.integer = n};
    return result;
}

static inline Expression *make_variable_expression(const char *varname) {
    Expression *result = malloc(sizeof(Expression));
    *result =
        (Expression){.type = VARIABLE_EXPRESSION, .value.variable = varname};
    return result;
}

static inline Expression *make_boolean_expression(bool b) {
    Expression *result = malloc(sizeof(Expression));
    *result = (Expression){.type = BOOLEAN_EXPRESSION, .value.boolean = b};
    return result;
}

static inline Expression *make_binary_expression(Expression *fst,
                                                 Expression *snd) {
    Expression *result = malloc(sizeof(Expression));
    *result = (Expression){.value.binary.fst = fst, .value.binary.snd = snd};
    return result;
}

static inline Expression *make_unary_expression(Expression *fst) {
    Expression *result = malloc(sizeof(Expression));
    *result = (Expression){.value.unary.fst = fst};
    return result;
}

static inline Expression *set_expression_type(Expression *exp,
                                              ExpressionType type) {
    exp->type = type;
    return exp;
}

static inline void print_expression(Expression *exp) {
    // FIXME: should not be static inlined?
    ExpressionType type = exp->type;
    ExpressionValue value = exp->value;

    switch (type) {
    case INTEGER_EXPRESSION:
        printf("%d", value.integer);
        break;
    case VARIABLE_EXPRESSION:
        printf("VariableName: %s", value.variable);
        break;
    case BOOLEAN_EXPRESSION:
        printf("%s", value.boolean ? "true" : "false");
        break;
    case PLUS_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" + ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case MINUS_EXPRESSION:
        printf("(-");
        print_expression(value.unary.fst);
        printf(")");
        break;
    case MULTIPLY_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" * ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case DIVIDE_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" / ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case MODULO_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" %% ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case AND_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" && ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case OR_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" || ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case NOT_EXPRESSION:
        printf("(!");
        print_expression(value.unary.fst);
        printf(")");
        break;
    case EQUALS_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" == ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case NOT_EQUALS_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" != ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case GREATER_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" > ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case GREATER_EQUALS_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" >= ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case LESSER_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" < ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    case LESSER_EQUALS_EXPRESSION:
        printf("(");
        print_expression(value.binary.fst);
        printf(" <= ");
        print_expression(value.binary.snd);
        printf(")");
        break;
    default:
        printf("Found Undefined Expression Type");
    }
}

static inline void clear_expression(Expression *exp) {
    // TODO: implement
}
