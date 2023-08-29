#include <string>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "codegen.h"
#include "error.h"
#include "file_io.h"
#include "environment.h"
#include "parser.h"

void displayUsage(char **argv) {
    std::cout << "Usage: " << argv[0] << " <file_path>";
}

int main(int argc, char **argv) {
    if (argc < 2) {
        displayUsage(argv);
        return 0;
    }

    Node *program = nodeAllocate();
    ParsingContext *context = parseContextDefaultCreate();
    Error err = parseProgram(argv[1], context, program);

    printNode(program, 0);
    std::cout << '\n';

    if(err.type != ErrorType::NONE) {
        printError(err);
        return 1;
    }

    err = codegen_program(CodegenOutputFormat::DEFAULT, context, program);
    if(err.type != ErrorType::NONE) {
        printError(err);
        return 2;
    }

    deleteNode(program);

    return 0;
}

