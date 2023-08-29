#include "codegen.h"

#include "error.h"
#include "environment.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstddef>
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


Error codegen_function_x86_64_att_asm_mswin(ParsingContext *context, char *name, Node *function, std::ofstream &code);

Error codegen_expression_list_x86_64_att_asm_mswin(ParsingContext *context, Node *expression, std::ofstream &code) {
    Error err = ok;
    Node *tmpnode = nodeAllocate();
    size_t tmpcount;
    const size_t lambda_symbol_size = 8;
    char lambda_symbol[8];
    for (size_t i = 0; i < lambda_symbol_size; i++) {
        lambda_symbol[i] = (rand() % 26) + 97;
    }
    while(expression){
        tmpcount = 0;
        switch(expression->type){
            default:
                break;
            case NodeType::FUNCTION:
                // Handling a function here means a lambda should be generated, I think.
                // TODO: Generate name from some sort of hashing algorithm or something.
                err = codegen_function_x86_64_att_asm_mswin(context, lambda_symbol, expression, code);
                // If we were to keep track of the name of this function, we could
                // then properly fill in the jump memory label further on in the program.
                if(err.type != ErrorType::NONE){ return err; }
                break;
            case NodeType::FUNCTION_CALL:
                // TODO: Actually codegen argument expressions.
                tmpnode = expression->children->next_child->children;
                while(tmpnode){
                    switch(tmpcount){
                        default:
                            std::cout << "TODO: Codegen stack allocated arguments\n";
                            break;
                        case 0:
                            fwrite_bytes("mov $",code);
                            // TODO:FIXME: This assumes integer type, and is bad bad bad!!!
                            fwrite_integer(tmpnode->value.integer,code);
                            fwrite_line(", %rcx",code);
                            break;
                        case 1:
                            fwrite_bytes("mov $",code);
                            // TODO:FIXME: This assumes integer type, and is bad bad bad!!!
                            fwrite_integer(tmpnode->value.integer,code);
                            fwrite_line(", %rdx",code);
                            break;
                        case 2:
                            fwrite_bytes("mov $",code);
                            // TODO:FIXME: This assumes integer type, and is bad bad bad!!!
                            fwrite_integer(tmpnode->value.integer,code);
                            fwrite_line(", %r8",code);
                            break;
                        case 3:
                            fwrite_bytes("mov $",code);
                            // TODO:FIXME: This assumes integer type, and is bad bad bad!!!
                            fwrite_integer(tmpnode->value.integer,code);
                            fwrite_line(", %r9",code);
                            break;
                    }
                    tmpnode = tmpnode->next_child;
                    tmpcount += 1;
                }
                fwrite_bytes("call ",code);
                fwrite_line(expression->children->value.symbol,code);
                break;
            case NodeType::VARIABLE_REASSIGNMENT:
                // TODO: Find variable binding and keep track of which context it is found in.
                //       If context is top-level, use global variable access, otherwise local.

                // TODO: Evaluate reassignment expression and get return value,
                //       that way we can actually use it!

                if (!context->parent) {
                    fwrite_bytes("lea ",code);
                    fwrite_bytes(expression->children->value.symbol,code);
                    fwrite_line("(%rip), %rax",code);
                    fwrite_bytes("movq $",code);
                    // TODO: FIXME: This assumes integer type, and is bad bad bad!!!
                    fwrite_integer(expression->children->next_child->value.integer,code);
                    fwrite_line(", (%rax)",code);
                } else {

                    // TODO: Get index of argument within function parameter list

                    // TODO: Use index of argument to write to proper address/register

                }

                break;
        }

        expression = expression->next_child;
    }
    delete tmpnode;

    return ok;
}

Error codegen_function_x86_64_att_asm_mswin(ParsingContext *context, char *name, Node *function, std::ofstream &code) {
    Node *parameter = function->children;
    // Nested function execution protection
    fwrite_bytes("jmp after",code);
    fwrite_line(name,code);

    // Function begin memory symbol
    fwrite_bytes(name,code);
    fwrite_line(":",code);

    // Function header
    fwrite_line("push %rbp", code);
    fwrite_line("mov %rsp, %rbp", code);
    fwrite_line("sub $32, %rsp", code);

    // Function body
    context = parseContextCreate(context);
    codegen_expression_list_x86_64_att_asm_mswin(context, function->children->next_child->next_child->children, code);
    // TODO: Free context;
    context = context->parent;

    // Function footer
    fwrite_line("add $32, %rsp", code);
    fwrite_line("pop %rbp", code);
    fwrite_line("ret", code);

    // Nested function execution jump label
    fwrite_bytes("after",code);
    fwrite_bytes(name,code);
    fwrite_line(":",code);

    return ok;
}

/// Emit x86_64 AT&T Assembly with MS Windows function calling convention.
/// Arguments passed in: RCX, RDX, R8, R9 -> stack
Error codegen_program_x86_64_att_asm_mswin(ParsingContext *context, Node *program){
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

    Binding *function_it = context->functions->bind;
    while(function_it){
        Node *function_id = function_it->id;
        Node *function = function_it->value;
        function_it = function_it->next;

        err = codegen_function_x86_64_att_asm_mswin(context, function_id->value.symbol, function, code);
    }

    fwrite_line(".global _start", code);
    fwrite_line("_start:", code);
    fwrite_line("push %rbp", code);
    fwrite_line("mov %rsp, %rbp", code);
    fwrite_line("sub $32, %rsp", code);

    codegen_expression_list_x86_64_att_asm_mswin(context, program->children, code);

    fwrite_line("add $32, %rsp", code);
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
            return codegen_program_x86_64_att_asm_mswin(context, program);
    }
    return ok;
}
