%language "C++"
%require "3.2"


%define api.value.type variant
%define api.token.constructor
%define api.token.prefix {TOK_}
%define parse.error verbose
%define parse.trace

%locations

%param { int& num_errors }
%param { bool interpret }
%param { std::unordered_map<std::string, std::unique_ptr<FunctionDef>> &functions_ast }
%param { std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast }

%code provides {
    #define YY_DECL \
        yy::parser::symbol_type yylex( \
            int& num_errors, \
            bool compile, \
            std::unordered_map<std::string, std::unique_ptr<FunctionDef>> &functions_ast, \
            std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast \
        )

    YY_DECL;

    #define PROMPT std::endl << ">>> "
}

%code requires {
    #include <iostream>
    #include <variant>
    #include <unordered_map>
    #include "AST.hh"
}

%code {
    #include <stdio.h>
    extern FILE *yyin;
}

%printer { yyo << $$; } <int>;
%printer { yyo << $$; } <std::string>;
%printer { yyo << $$; } <std::unique_ptr<Expression>>;

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
%token <int> INTEGER
%token <std::string> IDENTIFIER

%token EOF 0 "end-of-file"

%type <std::unique_ptr<Expression>> basic_expression;
%type <std::unique_ptr<Expression>> expression;
%type <FunctionCall>function_call_arguments;
%type <std::unique_ptr<ValueDef>> value_definition;
%type <std::unique_ptr<FunctionDef>> function_definition;
%type <std::unique_ptr<FunctionDef>> function_definition_arguments;

%precedence KW_ELSE
%left AND OR
%left EQUALS NOT_EQUALS GREATER GREATER_EQUALS LESSER LESSER_EQUALS
%left PLUS
%left MULTIPLY DIVIDE
%left MODULO
%precedence MINUS
%precedence NOT

%%
input: %empty
     | input expression STATEMENT_END { 
            if (interpret) {
                std::unordered_map<std::string, std::variant<bool, int>> interpret_context;
                std::unordered_map<std::string, TYPE> semantics_context;
                /* TODO: use `semantics_verified` and `semantics_correct` */
                if (!((($2)->verify_semantics(INT_T, functions_ast, globals_ast, semantics_context)) || (($2)->verify_semantics(BOOL_T, functions_ast, globals_ast, semantics_context)))) {
                    std::cerr << "Invalid semantics for the given expression\n" << PROMPT;
                }
                else {
                    std::variant<bool, int> res = ($2)->interpret(functions_ast, globals_ast, interpret_context);
                    if (std::holds_alternative<int>(res)) {
                        std::cout << std::get<int>(res) << PROMPT;
                    } else {
                        std::cout << (std::get<bool>(res) ? "true" : "false") << PROMPT;
                    }
                }
            } else {
                std::cerr << "Cannot have expressions at top level\n";
            }
        }
     | input value_definition { if (interpret) std::cout << ($2) << PROMPT; globals_ast.insert({($2)->name, std::move($2)}); }
     | input function_definition { if (interpret) std::cout << ($2) << PROMPT; functions_ast.insert({($2)->name, std::move($2)}); };

function_definition: KW_FUNCDEF IDENTIFIER function_definition_arguments RETURN KW_BOOL ASSIGN expression STATEMENT_END { ($3)->set_info($2, BOOL_T, std::move($7)); $$ = std::move($3); }
                   | KW_FUNCDEF IDENTIFIER function_definition_arguments RETURN KW_INT ASSIGN expression STATEMENT_END { ($3)->set_info($2, INT_T, std::move($7)); $$ = std::move($3); };

function_definition_arguments: IDENTIFIER TYPE_OF KW_BOOL { $$ = FunctionDef::from(); ($$)->add_argument(BOOL_T, $1); }
                             | IDENTIFIER TYPE_OF KW_INT { $$ = FunctionDef::from(); ($$)->add_argument(INT_T, $1); }
                             | IDENTIFIER TYPE_OF KW_BOOL function_definition_arguments { ($4)->add_argument(BOOL_T, $1); $$ = std::move($4); }
                             | IDENTIFIER TYPE_OF KW_INT function_definition_arguments { ($4)->add_argument(INT_T, $1); $$ = std::move($4); }
                             | OPEN_BRACKETS function_definition_arguments CLOSE_BRACKETS { $$ = std::move($2); };

value_definition: KW_VALDEF IDENTIFIER TYPE_OF KW_BOOL ASSIGN expression STATEMENT_END { $$ = ValueDef::from(BOOL_T, $2, std::move($6)); }
                | KW_VALDEF IDENTIFIER TYPE_OF KW_INT ASSIGN expression STATEMENT_END { $$ = ValueDef::from(INT_T, $2, std::move($6)); };

basic_expression: IDENTIFIER { $$ = Expression::from($1); }
                | INTEGER { $$ = Expression::from($1); }
                | KW_TRUE { $$ = Expression::from(true); }
                | KW_FALSE { $$ = Expression::from(false); }
                | OPEN_BRACKETS expression CLOSE_BRACKETS { $$ = std::move($2); };

expression: basic_expression { $$ = std::move($1); }
          | AND basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), AND_OP); }
          | OR basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), OR_OP); }
          | NOT basic_expression { $$ = Expression::from(std::move($2), NOT_OP); }
          | PLUS basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), ADD_OP); }
          | MULTIPLY basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), MUL_OP); }
          | DIVIDE basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), DIV_OP); }
          | MODULO basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), MOD_OP); }
          | MINUS basic_expression { $$ = Expression::from(std::move($2), NEG_OP); }
          | EQUALS basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), EQS_OP); }
          | NOT_EQUALS basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), NEQ_OP); }
          | GREATER basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), GT_OP); }
          | GREATER_EQUALS basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), GTE_OP); }
          | LESSER basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), LT_OP); }
          | LESSER_EQUALS basic_expression basic_expression { $$ = Expression::from(std::move($2), std::move($3), LTE_OP); }
          | KW_IF expression KW_THEN expression KW_ELSE expression { $$ = Expression::from(std::move($2), std::move($4), std::move($6)); }
          | IDENTIFIER function_call_arguments { ($2).set_function_name($1); $$ = Expression::from(std::move($2)); };

function_call_arguments: basic_expression { $$ = FunctionCall(); ($$).add_argument(std::move($1)); }
                       | basic_expression function_call_arguments { ($2).add_argument(std::move($1)); $$ = std::move($2); };
%%

void yy::parser::error(const location_type& loc, const std::string& s) {
    std::cerr << loc << ": " << s << '\n';
}

int parse(
    std::string filename, bool interpret,
    std::unordered_map<std::string, std::unique_ptr<FunctionDef>> &functions_ast,
    std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast) {
    if (filename != "") {
        FILE *file = fopen(filename.c_str(), "r");
        if (file == NULL) {
            std::cerr << "Could not open file " << filename << "\n";
            return 1;
        }
        yyin = file;
    }

    auto num_errors = 0;
    yy::parser parser(num_errors, interpret, functions_ast, globals_ast);
    auto status = parser.parse();
    return status;
}
