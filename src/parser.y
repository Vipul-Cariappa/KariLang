%{
    #include "common.h"
    #include "parser.h"
%}

%union {
    int integer;
    char *identifier;
    struct _Expression *expression;
    struct _Variable *variable;
    struct _Function *function;
}

%token KW_VALDEF 
%token KW_FUNCDEF
%token KW_BOOL
%token KW_INT
%token KW_TRUE
%token KW_FALSE
%token KW_IF
%token KW_THEN
%token KW_ELSE
%token COMMA
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
%token RETURN
%token <integer> INTEGER
%token <identifier> IDENTIFIER

%type <function> function_definition;
%type <function> function_definition_arguments;
%type <variable> value_definition;
%type <expression> expression;
%type <expression> function_call_arguments;

%left AND OR
%left EQUALS NOT_EQUALS GREATER GREATER_EQUALS LESSER LESSER_EQUALS
%left PLUS
%left MULTIPLY DIVIDE
%left MODULO
%precedence MINUS
%precedence NOT
%precedence KW_ELSE

%%
input: %empty
     | input value_definition { assert(ast_table_insert(ast, ($2)->name, (AST){.type = AST_VARIABLE, .value.var = $2})); }
     | input function_definition { assert(ast_table_insert(ast, ($2)->funcname, (AST){.type = AST_FUNCTION, .value.func = $2})); };

function_definition: KW_FUNCDEF IDENTIFIER function_definition_arguments RETURN KW_BOOL ASSIGN expression { $$ = set_function_return_value(set_function_name($3, $2), BOOL, $7); }
                   | KW_FUNCDEF IDENTIFIER function_definition_arguments RETURN KW_INT ASSIGN expression { $$ = set_function_return_value(set_function_name($3, $2), INT, $7);};

function_definition_arguments: IDENTIFIER KW_BOOL { $$ = add_function_argument(make_function(), $1, BOOL); }
                             | OPEN_BRACKETS IDENTIFIER KW_BOOL CLOSE_BRACKETS { $$ = add_function_argument(make_function(), $2, BOOL); }
                             | IDENTIFIER KW_INT { $$ = add_function_argument(make_function(), $1, INT); }
                             | OPEN_BRACKETS IDENTIFIER KW_INT CLOSE_BRACKETS { $$ = add_function_argument(make_function(), $2, INT); }
                             | IDENTIFIER KW_BOOL function_definition_arguments { $$ = add_function_argument($3, $1, BOOL); }
                             | OPEN_BRACKETS IDENTIFIER KW_BOOL CLOSE_BRACKETS function_definition_arguments { $$ = add_function_argument($5, $2, BOOL); }
                             | IDENTIFIER KW_INT function_definition_arguments { $$ = add_function_argument($3, $1, INT); }
                             | OPEN_BRACKETS IDENTIFIER KW_INT CLOSE_BRACKETS function_definition_arguments { $$ = add_function_argument($5, $2, INT); };

value_definition: KW_VALDEF KW_BOOL IDENTIFIER ASSIGN expression { $$ = make_variable($3, BOOL, $5); }
                | KW_VALDEF KW_INT IDENTIFIER ASSIGN expression { $$ = make_variable($3, INT, $5); };

expression: IDENTIFIER { $$ = make_variable_expression($1); }
          | INTEGER { $$ = make_integer_expression($1); }
          | KW_TRUE { $$ = make_boolean_expression(true); }
          | KW_FALSE { $$ = make_boolean_expression(false); }
          | expression AND expression { $$ = make_binary_expression($1, $3, AND_EXPRESSION); }
          | expression OR expression { $$ = make_binary_expression($1, $3, OR_EXPRESSION); }
          | NOT expression { $$ = make_unary_expression($2, NOT_EXPRESSION); }
          | expression PLUS expression { $$ = make_binary_expression($1, $3, PLUS_EXPRESSION); }
          | expression MULTIPLY expression { $$ = make_binary_expression($1, $3, MULTIPLY_EXPRESSION); }
          | expression DIVIDE expression { $$ = make_binary_expression($1, $3, DIVIDE_EXPRESSION); }
          | expression MODULO expression { $$ = make_binary_expression($1, $3, MODULO_EXPRESSION); }
          | MINUS expression { $$ = make_unary_expression($2, MINUS_EXPRESSION); }
          | expression EQUALS expression { $$ = make_binary_expression($1, $3, EQUALS_EXPRESSION); }
          | expression NOT_EQUALS expression { $$ = make_binary_expression($1, $3, NOT_EQUALS_EXPRESSION); }
          | expression GREATER expression { $$ = make_binary_expression($1, $3, GREATER_EXPRESSION); }
          | expression GREATER_EQUALS expression { $$ = make_binary_expression($1, $3, GREATER_EQUALS_EXPRESSION); }
          | expression LESSER expression { $$ = make_binary_expression($1, $3, LESSER_EXPRESSION); }
          | expression LESSER_EQUALS expression { $$ = make_binary_expression($1, $3, LESSER_EQUALS_EXPRESSION); }
          | KW_IF expression KW_THEN expression KW_ELSE expression { $$ = make_if_expression($2, $4, $6); }
          | IDENTIFIER OPEN_BRACKETS function_call_arguments CLOSE_BRACKETS { $$ = set_function_call_name_expression($3, $1); }
          | OPEN_BRACKETS expression CLOSE_BRACKETS { $$ = $2; };

function_call_arguments: expression { $$ = add_function_call_argument_expression(make_function_call_expression(), $1); }
                       | expression COMMA function_call_arguments { $$ = add_function_call_argument_expression($3, $1); };
%%

void yyerror(char const *s) {
  fprintf(stderr, "PARSER ERROR: %s\n", s);
}
