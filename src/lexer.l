%{
    #include "parser.tab.h"
    #include <string.h>
    #include <ctype.h>

    static int next_column = 1;
    int column = 1;

    #define HANDLE_COLUMN               \
        column = next_column;           \
        next_column += strlen(yytext)
%}

%option noyywrap noinput nounput yylineno

%%
"valdef"                  { HANDLE_COLUMN; return KW_VALDEF; }
"funcdef"                 { HANDLE_COLUMN; return KW_FUNCDEF; }
"bool"                    { HANDLE_COLUMN; return KW_BOOL; }
"int"                     { HANDLE_COLUMN; return KW_INT; }
"true"                    { HANDLE_COLUMN; return KW_TRUE; }
"false"                   { HANDLE_COLUMN; return KW_FALSE; }
"if"                      { HANDLE_COLUMN; return KW_IF; }
"then"                    { HANDLE_COLUMN; return KW_THEN; }
"else"                    { HANDLE_COLUMN; return KW_ELSE; }
";"                       { HANDLE_COLUMN; return STATEMENT_END; }
":"                       { HANDLE_COLUMN; return TYPE_OF; }
","                       { HANDLE_COLUMN; return COMMA; }
"+"                       { HANDLE_COLUMN; return PLUS; }
"-"                       { HANDLE_COLUMN; return MINUS; }
"*"                       { HANDLE_COLUMN; return MULTIPLY; }
"/"                       { HANDLE_COLUMN; return DIVIDE; }
"%"                       { HANDLE_COLUMN; return MODULO; }
"&&"                      { HANDLE_COLUMN; return AND; }
"||"                      { HANDLE_COLUMN; return OR; }
"!"                       { HANDLE_COLUMN; return NOT; }
"=="                      { HANDLE_COLUMN; return EQUALS; }
"!="                      { HANDLE_COLUMN; return NOT_EQUALS; }
">"                       { HANDLE_COLUMN; return GREATER; }
"<"                       { HANDLE_COLUMN; return LESSER; }
">="                      { HANDLE_COLUMN; return GREATER_EQUALS; }
"<="                      { HANDLE_COLUMN; return LESSER_EQUALS; }
"->"                      { HANDLE_COLUMN; return RETURN; }
"("                       { HANDLE_COLUMN; return OPEN_BRACKETS; }
")"                       { HANDLE_COLUMN; return CLOSE_BRACKETS; }
"="                       { HANDLE_COLUMN; return ASSIGN; }
[0-9]*                    { HANDLE_COLUMN; yylval.integer = atoi(yytext); return INTEGER; }
[a-zA-Z_][0-9a-zA-Z_]*    { HANDLE_COLUMN; yylval.identifier = strdup(yytext); return IDENTIFIER; }
[ \t]+                    { HANDLE_COLUMN; }
[\n]                      { HANDLE_COLUMN; next_column = 1; }
\/\/.+                    { ; }
.                         { HANDLE_COLUMN; /* TODO: handle error */ }
%%
