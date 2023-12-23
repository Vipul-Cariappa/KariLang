#pragma once

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

extern FILE *yyin;
extern int yylex(void);
extern int yyparse(void);
extern int yywrap(void);
extern void yyerror(char const *s);
extern int yylineno;
extern char* yytext;