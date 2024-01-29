%language "C++"
%require "3.2"


%define api.value.type variant
%define api.token.constructor
%define api.token.prefix {TOK_}
%define parse.error verbose
%define parse.trace

%locations

%param { int& num_errors }

%code provides {
    #define YY_DECL \
        yy::parser::symbol_type yylex(int& num_errors)

    YY_DECL;
}

%code requires {
    #include <iostream>
}

%code {
    
}

%printer { yyo << $$; } <int>;
%printer { yyo << $$; } <std::string>;

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

/* %type <function> function_definition;
%type <function> function_definition_arguments;
%type <variable> value_definition;
%type <expression> expression;
%type <expression> function_call_arguments; */
%type function_definition;
%type function_definition_arguments;
%type value_definition;
%type expression;
%type function_call_arguments;

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
     | input expression STATEMENT_END {}
     | input value_definition {}
     | input function_definition {};

function_definition: KW_FUNCDEF IDENTIFIER function_definition_arguments RETURN KW_BOOL ASSIGN expression STATEMENT_END {}
                   | KW_FUNCDEF IDENTIFIER function_definition_arguments RETURN KW_INT ASSIGN expression STATEMENT_END {};

function_definition_arguments: IDENTIFIER TYPE_OF KW_BOOL {}
                             | IDENTIFIER TYPE_OF KW_INT {}
                             | IDENTIFIER TYPE_OF KW_BOOL function_definition_arguments {}
                             | IDENTIFIER TYPE_OF KW_INT function_definition_arguments {}
                             | OPEN_BRACKETS function_definition_arguments CLOSE_BRACKETS {};

value_definition: KW_VALDEF IDENTIFIER TYPE_OF KW_BOOL ASSIGN expression STATEMENT_END {}
                | KW_VALDEF IDENTIFIER TYPE_OF KW_INT ASSIGN expression STATEMENT_END {};

basic_expression: IDENTIFIER {}
                | INTEGER {}
                | KW_TRUE {}
                | KW_FALSE {}
                | OPEN_BRACKETS expression CLOSE_BRACKETS {};

expression: basic_expression {}
          | AND basic_expression basic_expression {}
          | OR basic_expression basic_expression {}
          | NOT basic_expression {}
          | PLUS basic_expression basic_expression {}
          | MULTIPLY basic_expression basic_expression {}
          | DIVIDE basic_expression basic_expression {}
          | MODULO basic_expression basic_expression {}
          | MINUS basic_expression {}
          | EQUALS basic_expression basic_expression {}
          | NOT_EQUALS basic_expression basic_expression {}
          | GREATER basic_expression basic_expression {}
          | GREATER_EQUALS basic_expression basic_expression {}
          | LESSER basic_expression basic_expression {}
          | LESSER_EQUALS basic_expression basic_expression {}
          | KW_IF expression KW_THEN expression KW_ELSE expression {}
          | IDENTIFIER function_call_arguments {};

function_call_arguments: basic_expression {}
                       | basic_expression function_call_arguments {};
%%
void yy::parser::error(const location_type& loc, const std::string& s) {
    std::cerr << loc << ": " << s << '\n';
}

int parse() {
    auto num_errors = 0;
    yy::parser parser(num_errors);
    auto status = parser.parse();
    return num_errors;
}
