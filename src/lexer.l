%{
    #include "parser.tab.h"
    #include <string.h>
%}

%%
"valdef"                  { return KW_VALDEF; }
"funcdef"                 { return KW_FUNCDEF; }
"bool"                    { return KW_BOOL; }
"int"                     { return KW_INT; }
"true"                    { return KW_TRUE; }
"false"                   { return KW_FALSE; }
"if"                      { return KW_IF; }
"then"                    { return KW_THEN; }
"else"                    { return KW_ELSE; }
"+"                       { return PLUS; }
"-"                       { return MINUS; }
"*"                       { return MULTIPLY; }
"/"                       { return DIVIDE; }
"%"                       { return MODULO; }
"&&"                      { return AND; }
"||"                      { return OR; }
"!"                       { return NOT; }
"=="                      { return EQUALS; }
"!="                      { return NOT_EQUALS; }
">"                       { return GREATER; }
"<"                       { return LESSER; }
">="                      { return GREATER_EQUALS; }
"<="                      { return LESSER_EQUALS; }
"->"                      { return RETURN; }
"("                       { return OPEN_BRACKETS; }
")"                       { return CLOSE_BRACKETS; }
"="                       { return ASSIGN; }
[0-9]*                    { yylval.integer = atoi(yytext); return INTEGER; }
[a-zA-Z_][0-9a-zA-Z_]*    { yylval.identifier = strdup(yytext); return IDENTIFIER; }
[ \n\t]                   { ; }
.                         { fprintf(stderr, "Unexpected Token\n"); return YYUNDEF; }
%%

int yywrap(void) {
    return 1;
}
