#include <stdio.h>
#include <stdlib.h>

#define RED "\x1B[31m"
#define RESET "\x1B[0m"

int ____karilang_main(int);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "KariLang programs take one integer input and outputs "
                        "an integer value.\n" RED "Error: " RESET
                        "Integer input missing.\n");
        return 1;
    }

    int input = atoi(argv[1]);
    printf("Input: %d\nOutput: %d\n", input, ____karilang_main(input));

    return 0;
}
