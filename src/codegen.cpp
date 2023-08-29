#include "codegen.h"

#include "error.h"
#include "environment.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream>

const char *codegen_header = "Header file";

//================================================================ BEG FILE HELPERS

Error fwrite_line(const char *bytestring, std::ofstream &file){
    Error err = ok;
    err.createError(ErrorType::GENERIC, "fwrite_line(): Could not write line");
    if (!file.is_open())
        return err;
    size_t length = strlen(bytestring);
    file.write(bytestring, length);
    if(!file)
        return err;
    file.write("\n", 1);
    if(!file)
        return err;
    return ok;
}

Error fwrite_bytes(const char *bytestring, std::ofstream &file){
    Error err = ok;
    err.createError(ErrorType::GENERIC, "fwrite_bytes(): Could not write bytes");
    if(!file.is_open())
        return err;
    size_t length = strlen(bytestring);
    file.write(bytestring, length);
    if(!file)
        return err;
    return ok;
}

constexpr size_t FWRITE_INT_STRING_BUFFER_SIZE = 21;

Error fwrite_integer(long long integer, std::ofstream &file) {
    Error err = ok;
    err.createError(ErrorType::GENERIC, "fwrite_integer(): Could not write integer");
    if(!file.is_open())
        return err;
    // Create a stringstream to convert the integer to a string
    std::ostringstream oss;
    oss << integer; // Convert the integer to a string
    
    err = fwrite_bytes(oss.str().c_str(), file);
    return err;
}

//================================================================ END FILE HELPERS

//================================================================ BEG x86_64 AT&T ASM

Error codegen_program_x86_64_att_asm_data_section(ParsingContext *context, std::ofstream &code) {
    Error err = ok;

    err = fwrite_line(".section .data", code);
    if (err.type != ErrorType::NONE)
        return err;

    Node *type_info = nodeAllocate();
    Binding *it = context->variables->bind;
    while (it) {
        Node *var_id = it->id;
        Node *type_id = it->value;
        it = it->next;
        
        environmentGet(*context->types, type_id, type_info);

        err = fwrite_bytes(var_id->value.symbol, code);
        if (err.type != ErrorType::NONE)
            return err;
        err = fwrite_bytes(": .space ", code);
        if (err.type != ErrorType::NONE)
            return err;
        err = fwrite_integer(type_info->children->value.integer, code);
        if (err.type != ErrorType::NONE)
            return err;
        err = fwrite_bytes("\n", code);
        if (err.type != ErrorType::NONE)
            return err;
    }
    delete type_info;
    return err;
}


Error codegen_program_x86_64_att_asm(ParsingContext *context, Node *program){
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

    err = fwrite_bytes(";;#; ", code);
    if(err.type != ErrorType::NONE)
        return err;
    fwrite_line((char *)codegen_header, code);
    if(err.type != ErrorType::NONE)
        return err;

    codegen_program_x86_64_att_asm_data_section(context, code);

    fwrite_line(".section .text", code);

    fwrite_line("_start:", code);
    fwrite_line("push %rbp", code);
    fwrite_line("mov %rsp, %rbp", code);
    fwrite_line("sub $32, %rsp", code);

    Node *expression = program->children;
    while(expression){
        switch(expression->type) {
            default:
                break;
            case NodeType::VARIABLE_REASSIGNMENT:
                fwrite_bytes("lea ", code);
                fwrite_bytes(expression->children->value.symbol, code);
                fwrite_line("(%rip), %rax", code);
                fwrite_bytes("movq $", code);
                fwrite_integer(expression->children->next_child->value.integer, code);
                fwrite_line(", (%rax)", code);
                break;
        }
        expression = expression->next_child;
    }

    fwrite_line("add $32, %rsp", code);
    fwrite_line("movq (%rax), %rax",code);
    fwrite_line("pop %rbp", code);
    fwrite_line("ret", code);

    code.close();
    return ok;
}

//================================================================ END x86_64 AT&T ASM

Error codegen_program(CodegenOutputFormat format, ParsingContext *context, Node *program) {
    Error err = ok;
    if(!context){
        err.createError(ErrorType::ARGUMENTS, "codegen_program() must be passed a non-NULL context.");
        return err;
    }
    switch(format){
        case CodegenOutputFormat::DEFAULT:
        case CodegenOutputFormat::x86_64_AT_T_ASM:
            return codegen_program_x86_64_att_asm(context, program);
    }
    return ok;
}
