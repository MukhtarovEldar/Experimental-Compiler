#include "codegen.h"

#include "error.h"
#include "environment.h"
#include "parser.h"
#include <iostream>
#include <fstream>

Error codegen_program_x86_64_att_asm(parsingContext *context, Node *program){
    Error err = ok;
    if(!program || program->type != NodeType::PROGRAM){
        err.prepareError(ErrorType::ARGUMENTS, "codegen_program() requires a program!");
        return err;
    }

    std::ofstream code("code.S", std::ios::binary);
    if (!code.is_open()) {
        err.prepareError(ErrorType::GENERIC, "codegen_program() could not open code file.");
        return err;
    }

    Node *expression = program->children;
    Node *tmpnode1 = nodeAllocate();
    while(expression){
        switch(expression->type) {
            default:
                break;
            case NodeType::VARIABLE_DECLARATION:
                environmentGet(*context->variables, expression->children, tmpnode1);
                environmentGet(*context->types, tmpnode1, tmpnode1);
                printNode(tmpnode1, 0);
                break;
        }
        expression = expression->next_child;
    }
    code.close();
    return ok;
}

Error codegen_program(CodegenOutputFormat format, parsingContext *context, Node *program) {
    Error err = ok;
    if(!context){
        err.createError(ErrorType::ARGUMENTS, "codegen_program() must be passed a non-NULL context.");
        return err;
    }
    switch(format){
        case CodegenOutputFormat::OUTPUT_FMT_DEFAULT:
        case CodegenOutputFormat::OUTPUT_FMT_x86_64_AT_T_ASM:
            return codegen_program_x86_64_att_asm(context, program);
    }
    return ok;
}
