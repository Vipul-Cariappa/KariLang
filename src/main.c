#include "common.h"
#include <stdio.h>
#include <string.h>

IMPLEMENT_HASH_FUNCTION;
DS_TABLE_DEF(ast, AST, NULL);

ast_table_t *ast;
const char *filename;

bool cli_interpretation_mode = false;
int interactive_interpretation();
int file_interpretation(const char *file_name, int input);

int main(int argc, char *argv[]) {
    if (argc == 1) {
        return interactive_interpretation();
    }

    if (argc != 3) {
        fprintf(stderr, "File and input required to execute the program\n");
        return 1;
    }

    return file_interpretation(argv[1], atoi(argv[2]));
}

int interactive_interpretation() {
    cli_interpretation_mode = true;
    ast = ast_table_new(100);
    globalBooleans = boolean_table_new(100);
    globalIntegers = integer_table_new(100);
    yyparse();
    return 0;
}

int file_interpretation(const char *file_name, int input) {
    filename = file_name;

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\"\n", filename);
        return 0;
    }

    /* Initialization of Variables and Functions Table */
    ast = ast_table_new(100);

    /* Parsing */
    yyin = file;
    if (yyparse()) {
        fclose(file);
        fprintf(stderr, "%s\n", syntax_error_msg);
        return 1;
    }

    fclose(file);

    /* Sematic Analysis */
    if (!verify_semantics()) {
        fprintf(stderr, "Semantic Error: %s\n", semantic_error_msg);
        return 1;
    }

    /* Interpreting */
    int output;
    if (!interpret(input, &output)) {
        fprintf(stderr, "Runtime Error: %s\n", runtime_error_msg);
        return 1;
    }

    printf("Input: %d\nOutput: %d\n", input, output);

    return 0;
}
