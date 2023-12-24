#include "common.h"
#include <stdio.h>

IMPLEMENT_HASH_FUNCTION;
DS_TABLE_DEF(ast, AST, NULL);

ast_table_t *ast;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "File and input required to execute the program\n");
        return 1;
    }

    char *filename = argv[1];

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

    yyparse();
    /*  END  */

    fclose(file);

    /* Sematic Analysis */
    if (!verify_semantics()) {
        fprintf(stderr, "\n\nSemantic Error:\n%s\n", semantic_error_msg);
        return 1;
    }

    /* Interpreting */
    int output;
    int input = atoi(argv[2]);
    if (!interpret(input, &output)) {
        fprintf(stderr, "\n\nRuntime Error:\n%s\n", runtime_error_msg);
        return 1;
    }

    printf("Input: %d\nOutput: %d\n", input, output);

    return 0;
}
