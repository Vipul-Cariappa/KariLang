#include "common.h"
#include <stdio.h>
#include <string.h>

IMPLEMENT_HASH_FUNCTION;
DS_TABLE_DEF(ast, AST, NULL);

ast_table_t *ast;
char *filename;

bool cli_interpretation_mode = false;
void interactive_interpretation();

int main(int argc, char *argv[]) {
    if ((argc == 2) && (!strcmp("-i", argv[1]))) {
        interactive_interpretation();
        return 0;
    }

    if (argc != 3) {
        fprintf(stderr, "File and input required to execute the program\n");
        return 1;
    }

    filename = argv[1];

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\"\n", filename);
        return 0;
    }

    yyin = file;

    /* BEGIN */
    // int name_token, value_token = 0;
    // while (0 != (name_token = yylex())) {
    //     printf("NameToken: %d, ValueToken: %d, String: %s\n", name_token,
    //     value_token, yytext);
    // }

    ast = ast_table_new(100);

    if (yyparse()) {
        fclose(file);
        fprintf(stderr, "%s\n", syntax_error_msg);
        return 1;
    }
    /*  END  */

    fclose(file);

    /* Sematic Analysis */
    if (!verify_semantics()) {
        fprintf(stderr, "Semantic Error: %s\n", semantic_error_msg);
        return 1;
    }

    /* Interpreting */
    int output;
    int input = atoi(argv[2]);
    if (!interpret(input, &output)) {
        fprintf(stderr, "Runtime Error: %s\n", runtime_error_msg);
        return 1;
    }

    printf("Input: %d\nOutput: %d\n", input, output);

    return 0;
}

void interactive_interpretation() {
    cli_interpretation_mode = true;
    ast = ast_table_new(100);
    globalBooleans = boolean_table_new(100);
    globalIntegers = integer_table_new(100);
    yyparse();
}
