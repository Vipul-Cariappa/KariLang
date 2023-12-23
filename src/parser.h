#include <stdio.h>
#include <stdlib.h>

// FIXME: Check if memory allocations fail

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
};

struct _Expression {
    ExpressionType type;
    ExpressionValue value;
};

static inline const char *const Type_to_string(Type type) {
    // FIXME: should not be static inlined?
    switch (type) {
    case BOOL:
        return "bool";
    case INT:
        return "int";
    default:
        return "UNDEFINED TYPE";
    }
}

static inline Function *make_function() {
    return calloc(1, sizeof(Function) + sizeof(Argument));
}

static inline Function *set_function_name(Function *func,
                                          const char *funcname) {
    func->funcname = funcname;
    return func;
}

static inline Function *set_function_return_value(Function *func, Type type,
                                                  Expression *exp) {
    func->return_type = type;
    func->expression = exp;
    return func;
}

static inline Function *add_function_argument(Function *func,
                                              const char *argname, Type type) {
    if (func->arglen == 0) {
        func->args[0] = (Argument){.type = type, .name = argname};
        func->arglen = 1;
        return func;
    }
    func =
        realloc(func, sizeof(Function) + sizeof(Argument) * (func->arglen + 1));
    func->args[func->arglen] = (Argument){.type = type, .name = argname};
    func->arglen += 1;
    return func;
}

static inline Variable *make_variable(const char *varname, Type type,
                                      Expression *exp) {
    Variable *result = malloc(sizeof(Variable));
    *result = (Variable){.type = type, .name = varname, .expression = exp};
    return result;
}

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

static inline Expression *make_if_expression(Expression *condition,
                                             Expression *yes, Expression *no) {
    Expression *result = malloc(sizeof(Expression));
    *result = (Expression){.type = IF_EXPRESSION,
                           .value.if_statement.condition = condition,
                           .value.if_statement.yes = yes,
                           .value.if_statement.no = no};
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
    case IF_EXPRESSION:
        printf("Conditions: ");
        print_expression(value.if_statement.condition);
        printf("\n\tIfTrue: ");
        print_expression(value.if_statement.yes);
        printf("\n\tIfFalse: ");
        print_expression(value.if_statement.no);
        break;
    default:
        printf("Found Undefined Expression Type");
    }
}

static inline void print_variable(Variable *var) {
    // FIXME: should not be static inlined?
    printf("VariableName: %s, VariableType: %s, Expression: ",
           Type_to_string(var->type), var->name);
    print_expression(var->expression);
}

static inline void print_arguments(Argument *args, size_t arglen) {
    for (int i = arglen - 1; i >= 0; i--) {
        printf("\tArgumentName: %s, ArgumentType: %s\n", args[i].name,
               Type_to_string(args[i].type));
    }
}

static inline void print_function(Function *func) {
    // FIXME: should not be static inlined?
    printf("FunctionName: %s, FunctionReturnType: %s\n", func->funcname,
           Type_to_string(func->return_type));
    print_arguments(func->args, func->arglen);
    printf("\t\tExpression: ");
    print_expression(func->expression);
}

static inline void clear_expression(Expression *exp) {
    // TODO: implement
}

static inline void clear_variable(Expression *exp) {
    // TODO: implement
}
