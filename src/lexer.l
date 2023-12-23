%{
    #include "parser.tab.h"
    #include <string.h>
%}

%%
"valdef"                  { return KW_VALDEF; }
"funcdef"                 { return KW_FUNCDEF; }
"bool"                    { return KW_BOOL; }
"int"                     { return KW_INT; }
"="                       { return ASSIGN; }
"+"                       { return PLUS; }
"-"                       { return MINUS; }
"*"                       { return MULTIPLY; }
"/"                       { return DIVIDE; }
"%"                       { return MODULO; }
"->"                      { return RETURN; }
"("                       { return OPEN_BRACKETS; }
")"                       { return CLOSE_BRACKETS; }
[0-9]*                    { yylval.integer = atoi(yytext); return INTEGER; }
[a-zA-Z_][0-9a-zA-Z_]*    { yylval.identifier = strdup(yytext); return IDENTIFIER; }
[ \n\t]                   { ; }
.                         { fprintf(stderr, "Unexpected Token\n"); return YYUNDEF; }
%%

int yywrap(void) {
    return 1;
}
