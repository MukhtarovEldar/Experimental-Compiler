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

void displayUsage(char **argv);

int main(int argc, char **argv) {
    if (argc < 2) {
        displayUsage(argv);
        std::exit(0);
    }
    char *contents = FileContents(argv[1]);
    assert(contents && "Could not allocate buffer for file contents.");
    if (contents) {
        // std::cout << "Contents of " << path << ":\n---\n" << contents << "\n---\n";
        parsingContext *context = parseContextCreate();
        Node *program = nodeAllocate();
        program->type = NodeType::PROGRAM;
        Node *expression = nodeAllocate();
        char *contents_it = contents;
        for(;;){
            Error err = parseExpr(context, contents_it, &contents_it, expression);
            if (err.type != ErrorType::NONE) {
                printError(err);
                break;
            }
            if (!(*contents_it))
                break;
            Node *child = nodeAllocate();
            nodeCopy(expression, child);
            nodeAddChild(program, child);
        }
        deleteNode(expression);

        printNode(program, 0);
        std::cout << '\n';

        std::cout << "Generating code!\n";

        codegen_program(CodegenOutputFormat::OUTPUT_FMT_DEFAULT, context, program);

        std::cout << "Code generated.\n";

        deleteNode(program);

        delete[] contents;
    }

    return 0;
}

void displayUsage(char **argv)
{
    std::cout << "Usage: " << argv[0] << " <file_path>";
}