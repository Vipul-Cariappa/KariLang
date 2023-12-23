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
%token RETURN
%token <integer> INTEGER
%token <identifier> IDENTIFIER

%type <expression> expression;
%type <expression> binary_operator_call;

%%
input: %empty | expression { print_expression($1); printf("\n"); };

expression: INTEGER { $$ = make_integer_expression($1); }
          | IDENTIFIER { $$ = make_variable_expression($1); }
          | PLUS binary_operator_call { $$ = set_expression_plus($2); };

binary_operator_call: INTEGER INTEGER { $$ = make_binary_expression(make_integer_expression($1), make_integer_expression($2)); };

%%

void yyerror(char const *s) {
  fprintf(stderr, "PARSER ERROR: %s\n", s);
}
