%{
    #include "common.h"
    #include "cli_interpreter.h"

    #define ERROR_MSG_LEN 500
    char syntax_error_msg[ERROR_MSG_LEN];
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
%token STATEMENT_END
%token TYPE_OF
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
%type <expression> basic_expression;
%type <expression> function_call_arguments;

/* %precedence KW_ELSE
%left AND OR
%left EQUALS NOT_EQUALS GREATER GREATER_EQUALS LESSER LESSER_EQUALS
%left PLUS
%left MULTIPLY DIVIDE
%left MODULO
%precedence MINUS
%precedence NOT */

%%
input: %empty
     | input expression STATEMENT_END {
            if (cli_interpretation_mode) {
                cli_interpret((AST){.type = AST_EXPRESSION, .value.exp = $2});
            }
            else {
                yyerror("Standalone expression are not allowed\n");
                // ???: Don't exit here
                return 1;
            }
        }
     | input value_definition { 
            if (cli_interpretation_mode) {
                cli_interpret((AST){.type = AST_VARIABLE, .value.var = $2});
            }
            else {
                assert(ast_table_insert(ast, ($2)->name, (AST){.type = AST_VARIABLE, .value.var = $2}));
            }
        }
     | input function_definition { 
            if (cli_interpretation_mode) {
                cli_interpret((AST){.type = AST_FUNCTION, .value.func = $2});
            } else {
                assert(ast_table_insert(ast, ($2)->funcname, (AST){.type = AST_FUNCTION, .value.func = $2})); 
            }
        };

function_definition: KW_FUNCDEF IDENTIFIER function_definition_arguments RETURN KW_BOOL ASSIGN expression STATEMENT_END { $$ = set_function_return_value(set_function_name($3, $2), BOOL, $7); }
                   | KW_FUNCDEF IDENTIFIER function_definition_arguments RETURN KW_INT ASSIGN expression STATEMENT_END { $$ = set_function_return_value(set_function_name($3, $2), INT, $7);};

function_definition_arguments: IDENTIFIER TYPE_OF KW_BOOL { $$ = add_function_argument(make_function(), $1, BOOL); }
                             | IDENTIFIER TYPE_OF KW_INT { $$ = add_function_argument(make_function(), $1, INT); }
                             | IDENTIFIER TYPE_OF KW_BOOL function_definition_arguments { $$ = add_function_argument($4, $1, BOOL); }
                             | IDENTIFIER TYPE_OF KW_INT function_definition_arguments { $$ = add_function_argument($4, $1, INT); }
                             | OPEN_BRACKETS function_definition_arguments CLOSE_BRACKETS { $$ = $2; };

value_definition: KW_VALDEF IDENTIFIER TYPE_OF KW_BOOL ASSIGN expression STATEMENT_END { $$ = make_variable($2, BOOL, $6); }
                | KW_VALDEF IDENTIFIER TYPE_OF KW_INT ASSIGN expression STATEMENT_END { $$ = make_variable($2, INT, $6); };

basic_expression: IDENTIFIER { $$ = make_variable_expression($1); }
                | INTEGER { $$ = make_integer_expression($1); }
                | KW_TRUE { $$ = make_boolean_expression(true); }
                | KW_FALSE { $$ = make_boolean_expression(false); }
                | OPEN_BRACKETS expression CLOSE_BRACKETS { $$ = $2; };

expression: basic_expression { $$ = $1; }
          | AND basic_expression basic_expression { $$ = make_binary_expression($2, $3, AND_EXPRESSION); }
          | OR basic_expression basic_expression { $$ = make_binary_expression($2, $3, OR_EXPRESSION); }
          | NOT basic_expression { $$ = make_unary_expression($2, NOT_EXPRESSION); }
          | PLUS basic_expression basic_expression { $$ = make_binary_expression($2, $3, PLUS_EXPRESSION); }
          | MULTIPLY basic_expression basic_expression { $$ = make_binary_expression($2, $3, MULTIPLY_EXPRESSION); }
          | DIVIDE basic_expression basic_expression { $$ = make_binary_expression($2, $3, DIVIDE_EXPRESSION); }
          | MODULO basic_expression basic_expression { $$ = make_binary_expression($2, $3, MODULO_EXPRESSION); }
          | MINUS basic_expression { $$ = make_unary_expression($2, MINUS_EXPRESSION); }
          | EQUALS basic_expression basic_expression { $$ = make_binary_expression($2, $3, EQUALS_EXPRESSION); }
          | NOT_EQUALS basic_expression basic_expression { $$ = make_binary_expression($2, $3, NOT_EQUALS_EXPRESSION); }
          | GREATER basic_expression basic_expression { $$ = make_binary_expression($2, $3, GREATER_EXPRESSION); }
          | GREATER_EQUALS basic_expression basic_expression { $$ = make_binary_expression($2, $3, GREATER_EQUALS_EXPRESSION); }
          | LESSER basic_expression basic_expression { $$ = make_binary_expression($2, $3, LESSER_EXPRESSION); }
          | LESSER_EQUALS basic_expression basic_expression { $$ = make_binary_expression($2, $3, LESSER_EQUALS_EXPRESSION); }
          | KW_IF expression KW_THEN expression KW_ELSE expression { $$ = make_if_expression($2, $4, $6); }
          | IDENTIFIER function_call_arguments { $$ = set_function_call_name_expression($2, $1); };

function_call_arguments: basic_expression { $$ = add_function_call_argument_expression(make_function_call_expression(), $1); }
                       | basic_expression function_call_arguments { $$ = add_function_call_argument_expression($2, $1); };
%%

void yyerror(char const *str) {
    snprintf(syntax_error_msg, ERROR_MSG_LEN,
              "ERROR: %s in %s:%d:%d", str, filename, yylineno, column);
}
