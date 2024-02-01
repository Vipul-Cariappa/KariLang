#include <iostream>
#include <string>

int parse(std::string filename);

int main(int argc, char *argv[]) {
    if (argc == 3) {
        return parse(argv[1]);
    }
    if (argc != 1) {
        std::cerr << "File and input required to execute the program\n";
        return 1;
    }
    return parse("");
}
