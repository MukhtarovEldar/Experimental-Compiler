#ifndef COMPILER_CODEGEN_H
#define COMPILER_CODEGEN_H

#include "error.h"
#include "parser.h"

enum class CodegenOutputFormat {
    DEFAULT = 0,
    x86_64_AT_T_ASM,
};

Error codegen_program(CodegenOutputFormat format, ParsingContext *context, Node *program);

#endif /* COMPILER_CODEGEN_H */
