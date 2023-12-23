%{
    #include "common.h"
    #include "parser.h"
%}

%union {
    int integer;
    char *identifier;
    struct _Expression *expression;
}

%token KW_VALDEF 
%token KW_FUNCDEF
%token KW_BOOL
%token KW_INT
%token ASSIGN
%token PLUS
%token MINUS
%token MULTIPLY
%token DIVIDE
%token MODULO
%token OPEN_BRACKETS
%token CLOSE_BRACKETS
%token RETURN
%token <integer> INTEGER
%token <identifier> IDENTIFIER

%type <expression> arithmetic_expression;
%type <expression> binary_arithmetic_operator_call;
%type <expression> unary_arithmetic_operator_call;

%%
input: %empty | arithmetic_expression { print_expression($1); printf("\n"); };

arithmetic_expression: INTEGER { $$ = make_integer_expression($1); }
                     | IDENTIFIER { $$ = make_variable_expression($1); }
                     | PLUS binary_arithmetic_operator_call { $$ = set_expression_type($2, PLUS_EXPRESSION); }
                     | MULTIPLY binary_arithmetic_operator_call { $$ = set_expression_type($2, MULTIPLY_EXPRESSION); }
                     | DIVIDE binary_arithmetic_operator_call { $$ = set_expression_type($2, DIVIDE_EXPRESSION); }
                     | MODULO binary_arithmetic_operator_call { $$ = set_expression_type($2, MODULO_EXPRESSION); }
                     | MINUS unary_arithmetic_operator_call { $$ = set_expression_type($2, MINUS_EXPRESSION); }
                     | OPEN_BRACKETS arithmetic_expression CLOSE_BRACKETS { $$ = $2; };


unary_arithmetic_operator_call: arithmetic_expression { $$ = make_unary_expression($1); };
binary_arithmetic_operator_call: arithmetic_expression arithmetic_expression { $$ = make_binary_expression($1, $2); };

%%

void yyerror(char const *s) {
  fprintf(stderr, "PARSER ERROR: %s\n", s);
}
