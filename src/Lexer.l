%option noyywrap

%{
    #include "Parser.tab.hh"
    yy::parser::location_type loc;
    #define YY_USER_ACTION \
        do { \
            loc.columns(yyleng); \
        } while (false);
%}

%%
"exit"                    { exit(0); }
"valdef"                  { return yy::parser::make_KW_VALDEF(loc); }
"funcdef"                 { return yy::parser::make_KW_FUNCDEF(loc); }
"bool"                    { return yy::parser::make_KW_BOOL(loc); }
"int"                     { return yy::parser::make_KW_INT(loc); }
"true"                    { return yy::parser::make_KW_TRUE(loc); }
"false"                   { return yy::parser::make_KW_FALSE(loc); }
"if"                      { return yy::parser::make_KW_IF(loc); }
"then"                    { return yy::parser::make_KW_THEN(loc); }
"else"                    { return yy::parser::make_KW_ELSE(loc); }
";"                       { return yy::parser::make_STATEMENT_END(loc); }
":"                       { return yy::parser::make_TYPE_OF(loc); }
","                       { return yy::parser::make_COMMA(loc); }
"+"                       { return yy::parser::make_PLUS(loc); }
"-"                       { return yy::parser::make_MINUS(loc); }
"*"                       { return yy::parser::make_MULTIPLY(loc); }
"/"                       { return yy::parser::make_DIVIDE(loc); }
"%"                       { return yy::parser::make_MODULO(loc); }
"&&"                      { return yy::parser::make_AND(loc); }
"||"                      { return yy::parser::make_OR(loc); }
"!"                       { return yy::parser::make_NOT(loc); }
"=="                      { return yy::parser::make_EQUALS(loc); }
"!="                      { return yy::parser::make_NOT_EQUALS(loc); }
">"                       { return yy::parser::make_GREATER(loc); }
"<"                       { return yy::parser::make_LESSER(loc); }
">="                      { return yy::parser::make_GREATER_EQUALS(loc); }
"<="                      { return yy::parser::make_LESSER_EQUALS(loc); }
"->"                      { return yy::parser::make_RETURN(loc); }
"("                       { return yy::parser::make_OPEN_BRACKETS(loc); }
")"                       { return yy::parser::make_CLOSE_BRACKETS(loc); }
"="                       { return yy::parser::make_ASSIGN(loc); }
[0-9]*                    { return yy::parser::make_INTEGER(std::stoi(yytext), loc); }
[a-zA-Z_][0-9a-zA-Z_]*    { return yy::parser::make_IDENTIFIER(yytext, loc); }
[ \t]+                    { loc.step(); }
[\n]                      { loc.lines(yyleng); loc.step(); }
\/\/.+                    { loc.step(); }
.                         { std::cerr << "unexpected character: " << yytext << '\n'; num_errors += 1; }
<<EOF>>                   { return yy::parser::make_EOF(loc); }
%%
