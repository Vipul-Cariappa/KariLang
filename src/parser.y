%{
    #include "common.h"
    #include "parser.h"
%}

%union {
    int integer;
    char *identifier;
    struct _Expression *expression;
    struct _Variable *variable;
}

%token KW_VALDEF 
%token KW_FUNCDEF
%token KW_BOOL
%token KW_INT
%token KW_TRUE
%token KW_FALSE
%token ASSIGN
%token PLUS
%token MINUS
%token MULTIPLY
%token DIVIDE
%token MODULO
%token OPEN_BRACKETS
%token CLOSE_BRACKETS
%token AND
%token OR
%token NOT
%token EQUALS
%token NOT_EQUALS
%token GREATER
%token GREATER_EQUALS
%token LESSER
%token LESSER_EQUALS
%token <integer> INTEGER
%token <identifier> IDENTIFIER

%type <variable> value_definition;
%type <expression> expression;
%type <expression> arithmetic_expression;
%type <expression> boolean_expression;
%type <expression> comparison_expression;
%type <expression> binary_arithmetic_operator_call;
%type <expression> unary_arithmetic_operator_call;
%type <expression> unary_boolean_operator_call;
%type <expression> binary_boolean_operator_call;

%%
input: %empty
     | input expression { print_expression($2); printf("\n"); }
     | input value_definition { print_variable($2); printf("\n"); };

value_definition: KW_VALDEF KW_BOOL IDENTIFIER ASSIGN expression { $$ = make_variable($3, BOOL, $5); }
                | KW_VALDEF KW_INT IDENTIFIER ASSIGN expression { $$ = make_variable($3, INT, $5); };

expression: boolean_expression { $$ = $1; }
          | arithmetic_expression { $$ = $1; }
          | comparison_expression { $$ = $1; };

boolean_expression: KW_TRUE { $$ = make_boolean_expression(true); }
                  | KW_FALSE { $$ = make_boolean_expression(false); }
                  | IDENTIFIER { $$ = make_variable_expression($1); }
                  | AND binary_boolean_operator_call { $$ = set_expression_type($2, AND_EXPRESSION); }
                  | OR binary_boolean_operator_call { $$ = set_expression_type($2, OR_EXPRESSION); }
                  | NOT unary_boolean_operator_call { $$ = set_expression_type($2, NOT_EXPRESSION); }
                  | OPEN_BRACKETS boolean_expression CLOSE_BRACKETS { $$ = $2; };

unary_boolean_operator_call: boolean_expression { $$ = make_unary_expression($1); }
                           | comparison_expression { $$ = make_unary_expression($1); };

binary_boolean_operator_call: boolean_expression boolean_expression { $$ = make_binary_expression($1, $2); }
                            | boolean_expression comparison_expression { $$ = make_binary_expression($1, $2); };
                            | comparison_expression boolean_expression { $$ = make_binary_expression($1, $2); };
                            | comparison_expression comparison_expression { $$ = make_binary_expression($1, $2); };

comparison_expression: EQUALS binary_arithmetic_operator_call { $$ = set_expression_type($2, EQUALS_EXPRESSION); }
                     | NOT_EQUALS binary_arithmetic_operator_call { $$ = set_expression_type($2, NOT_EQUALS_EXPRESSION); }
                     | GREATER binary_arithmetic_operator_call { $$ = set_expression_type($2, GREATER_EXPRESSION); }
                     | GREATER_EQUALS binary_arithmetic_operator_call { $$ = set_expression_type($2, GREATER_EQUALS_EXPRESSION); }
                     | LESSER binary_arithmetic_operator_call { $$ = set_expression_type($2, LESSER_EXPRESSION); }
                     | LESSER_EQUALS binary_arithmetic_operator_call { $$ = set_expression_type($2, LESSER_EQUALS_EXPRESSION); };
                     | OPEN_BRACKETS comparison_expression CLOSE_BRACKETS { $$ = $2; };

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
