#include "parser.h"

#include <iostream>
#include <cassert>
#include "error.h"
#include "environment.h"
#include <cstdlib>
#include <string>
#include <cstring>
#include <cstddef>

// ---------------- LEXER BEGINNING -----------------

const char *comment_delimiters = ";#";
const char *whitespace = " \r\n";
const char *delimiters = " \r\n,():";

bool commentAtBeginning(Token token){
    const char *comment_it = comment_delimiters;
    while(*comment_it){
        if(*(token.begin) == *comment_it){
            return true;
        }
        comment_it++;
    }
    return false;
}

Error lex(char* source, Token *token) {
    Error err = ok;
    if (!source || !token) {
        err.prepareError(ErrorType::ARGUMENTS, "Could not lex empty source!");
        return err;
    }
    token->begin = source;
    token->begin += std::strspn(token->begin, whitespace);
    token->end = token->begin;
    if (*(token->end) == '\0')
        return err;
    while(commentAtBeginning(*token)){
        token->begin = std::strpbrk(token->begin, "\n");
        if (!token->begin) {
            token->end = token->begin;
            return err;
        }
        token->begin += std::strspn(token->begin, whitespace);
        token->end = token->begin;
    }
    if (*(token->end) == '\0')
        return err;
    
    token->end += std::strcspn(token->begin, delimiters);
    if (token->end == token->begin) {
        token->end += 1;
    }
    return err;
}

bool tokenStringEqual(const std::string &string, const Token *token){
    if(string.empty() || !token)
        return false;
    char* beg = token->begin;
    for(char ch: string){
        if(beg >= token->end || ch != *beg)
            return false;
        beg++;
    }
    return beg == token->end;
}

void printToken(Token &tok){
    std::cout << ((tok.end - tok.begin < 1) ?
                "INVALID TOKEN POINTERS" :
                std::string(tok.begin, tok.end - tok.begin));
}

// ----------------- LEXER ENDING ------------------

Node *nodeAllocate(){
    Node *node = new Node();
    assert(node && "Could not allocate memory for new AST node.");
    return node;
}

void nodeAddChild(Node *parent, Node *new_child){
    if(!parent || !new_child)
        return;
    if(parent->children){
        Node *child = parent->children;
        while(child->next_child)
            child = child->next_child;
        child->next_child = new_child;
    }
    else
        parent->children = new_child;
}

bool nodeCompare(Node *a, Node *b){
    if(!a || !b){
        if(!a && !b)
            return true;
        return false;
    }
    assert(static_cast<int>(NodeType::MAX) == 9 && "nodeCompare() must handle all node types.");
    if(a->type != b->type)
        return false;
    switch(a->type){
        case NodeType::NONE:
            if(b->isNone())
                return true;
            break;
        case NodeType::INTEGER:
            if(a->value.integer == b->value.integer)
                return true;
            break;
        case NodeType::SYMBOL:
            if (a->value.symbol && b->value.symbol)
                if (std::strcmp(a->value.symbol, b->value.symbol) == false)
                    return true;
            else if (!a->value.symbol && !b->value.symbol)
                return true;
            break;
        case NodeType::BINARY_OPERATOR:
            std::cout << "TODO: nodeCompare() BINARY OPERATOR\n";
            break;
        case NodeType::FUNCTION:
            std::cout << "TODO: nodeCompare() FUNCTION\n";
            break;
        case NodeType::VARIABLE_REASSIGNMENT:
            std::cout << "TODO: nodeCompare() VARIABLE REASSIGNMENT\n";
            break;
        case NodeType::VARIABLE_DECLARATION:
            std::cout << "TODO: nodeCompare() VARIABLE DECLARATION\n";
            break;
        case NodeType::VARIABLE_DECLARATION_INITIALIZED:
            std::cout << "TODO: nodeCompare() VARIABLE DECLARATION INITIALIZED\n";
            break;
        case NodeType::PROGRAM:
            std::cout << "TODO: Compare two programs.\n";
            break;
        }
    return false;
}

Node *nodeNone(){
    Node *none = nodeAllocate();
    none->type = NodeType::NONE;
    return none;
}

Node *nodeInteger(long long value){
    Node *integer = nodeAllocate();
    integer->type = NodeType::INTEGER;
    integer->value.integer = value;
    return integer;
}

Node *nodeSymbol(const char *symbol_string){
    Node *symbol = nodeAllocate();
    symbol->type = NodeType::SYMBOL;
    size_t symbol_length = strlen(symbol_string);
    symbol->value.symbol = new char[symbol_length + 1];
    std::strcpy(symbol->value.symbol, symbol_string);
    return symbol;
}

Node *nodeSymbolFromBuffer(char *buffer, size_t length) {
    assert(buffer && "Can not create AST symbol node from NULL buffer.");
    char *symbol_string = new char[length + 1];
    assert(symbol_string && "Could not allocate memory for symbol string.");
    std::memcpy(symbol_string, buffer, length);
    symbol_string[length] = '\0';
    Node *symbol = nodeAllocate();
    symbol->type = NodeType::SYMBOL;
    symbol->value.symbol = symbol_string;
    return symbol;
}

Error nodeAddType(Environment *types, NodeType type, Node *type_symbol, long long byte_size) {
    assert(types && "Can not add type to NULL types environment");
    assert(type_symbol && "Can not add NULL type symbol to types environment");
    assert(byte_size >= 0 && "Can not define new type with zero or negative byte size");

    Node *size_node = nodeAllocate();
    size_node->type = NodeType::INTEGER;
    size_node->value.integer = byte_size;

    Node *type_node = nodeAllocate();
    type_node->type = type;
    type_node->children = size_node;

    Error err = ok;

    if(environmentSet(types, type_symbol, type_node) == 1)
        return ok;
    std::cout << "Type that was redefined: " << type_symbol->value.symbol << '\n';
    err.createError(ErrorType::TYPE, "Redefinition of type!");
    return err;
}

void printNode(Node *node, size_t indent_level){
    if(!node)
        return;
    for(size_t i = 0; i < indent_level; i++){
        std::cout << ' ';
    }
    assert(static_cast<int>(NodeType::MAX) == 9 && "printNode() must handle all node types.");
    switch(node->type){
        default:
            std::cout << "UNKNOWN";
            break;
        case NodeType::NONE:
            std::cout << "NONE";
            break;
        case NodeType::INTEGER:
            std::cout << "INT:" << node->value.integer;
            break;
        case NodeType::SYMBOL:
            std::cout << "SYM";
            if (node->value.symbol)
                std::cout << ':' << node->value.symbol;
            break;
        case NodeType::VARIABLE_REASSIGNMENT:
            std::cout << "VARIABLE REASSIGNMENT";
            break;
        case NodeType::BINARY_OPERATOR:
            std::cout << "BINARY OPERATOR";
            break;
        case NodeType::VARIABLE_DECLARATION:
            std::cout << "VARIABLE DECLARATION";
            break;
        case NodeType::VARIABLE_DECLARATION_INITIALIZED:
            std::cout << "VARIABLE DECLARATION INITIALIZED";
            break;
        case NodeType::PROGRAM:
            std::cout << "PROGRAM";
            break;
        case NodeType::FUNCTION:
            std::cout << "FUNCTION";
            break;
    }
    std::cout << '\n';
    Node *child = node->children;
    while(child){
        printNode(child, indent_level + 4);
        child = child->next_child;
    }
}

void deleteNode(Node *root){
    if(!root)
        return;
    Node *child = root->children;
    Node *next_child = nullptr;
    while(child){
        next_child = child->next_child;
        deleteNode(child);
        child = next_child;
    }
    if(root->isSymbol() && root->value.symbol)
        delete root->value.symbol;
    delete root;
}

void nodeCopy(Node *a, Node *b) {
    if(!a || !b)
        return;
    b->type = a->type;
    switch(a->type){
    default:
        b->value = a->value;
        break;
    case NodeType::SYMBOL:
        size_t length = strlen(a->value.symbol);
        b->value.symbol = new char[length + 1];
        std::strcpy(b->value.symbol, a->value.symbol);
        assert(b->value.symbol && "nodeCopy(): Could not allocate memory for new symbol");
        break;
    }
    Node *child = a->children;
    Node *child_it = nullptr;
    while(child){
        Node *new_child = nodeAllocate();
        if(child_it){
            child_it->next_child = new_child;
            child_it = child_it->next_child;
        }else{
            b->children = new_child;
            child_it = new_child;
        }
        nodeCopy(child, child_it);
        child = child->next_child;
    }
}

ParsingContext *parseContextCreate(ParsingContext *parent){
    ParsingContext *ctx = new ParsingContext();
    assert(ctx && "Could not allocate for parsing context.");
    ctx->parent = parent;
    ctx->types = environmentCreate(nullptr);
    ctx->variables = environmentCreate(nullptr);
    ctx->functions = environmentCreate(nullptr);
    return ctx;
}

ParsingContext *parseContextDefaultCreate() {
  ParsingContext *ctx = parseContextCreate(nullptr);
  Error err = nodeAddType(ctx->types, 
                            NodeType::INTEGER,
                            nodeSymbol("integer"),
                            sizeof(long long));
  if(err.type != ErrorType::NONE)
    std::cout << "ERROR: Failed to set built-in integer type in types environment.\n";
  return ctx;
}

Error lexAdvance(Token *token, size_t *token_length, char **end) {
    Error err = ok;
    if(!token || !token_length || !end) {
        err.createError(ErrorType::ARGUMENTS, "lexAdvance(): pointer arguments must not be NULL!");
        return err;
    }
    err = lex(token->end, token);
    *end = token->end;
    if(err.type != ErrorType::NONE)
        return err;
    *token_length = token->end - token->begin;
    return err;
}

ExpectReturnValue lexExpect(const std::string &expected, Token *current, size_t *current_length, char **end){
    ExpectReturnValue out;
    out.done = false;
    out.found = false;
    out.err = ok;
    if(expected.empty() || !current || !current_length || !end){
        out.err.prepareError(ErrorType::ARGUMENTS, "lexExpect() must not be passed NULL pointers!");
        return out;
    }
    Token current_copy = *current;
    size_t current_length_copy = *current_length;
    char *end_value = *end;

    out.err = lexAdvance(&current_copy, &current_length_copy, &end_value);

    if(out.err.type != ErrorType::NONE)
        return out;
    if(current_length_copy == false){
        out.done = true;
        return out;
    }

    if(tokenStringEqual(expected, &current_copy)){
        out.found = true;
        *end = end_value;
        *current = current_copy;
        *current_length = current_length_copy;
        return out;
    }
    return out;
}

Error parseGetType(ParsingContext *context, Node *id, Node *result) {
    Error err = ok;
    while(context){
        bool status = environmentGet(*context->types, id, result);
        if(status)
            return ok;
        context = context->parent;
    }
    result->type = NodeType::NONE;
    err.prepareError(ErrorType::GENERIC, "Type is not found in environment.");
    return err;
}

Error ExpectReturnValue::expect(ExpectReturnValue &expected, const std::string &expected_string, Token &current_token, size_t &current_length, char **end){
    expected = lexExpect(expected_string, &current_token, &current_length, end);
    if(expected.err.type != ErrorType::NONE)
        return expected.err;
    if(expected.done)
        return ok;
    return {ErrorType::NONE, "Continue"};
}

bool parseInteger(Token *token, Node *node){
    if(!token || !node)
        return false;
    char *end = nullptr;
    /// Remove the condition for 0, if that becomes redundant  
    if(token->end - token->begin == 1 && *(token->begin) == '0'){
        node->type = NodeType::INTEGER;
        node->value.integer = 0;
    } else if((node->value.integer = std::strtoll(token->begin, &end, 10)) != 0){
        if(end != token->end)
            return false;
        node->type = NodeType::INTEGER;
    } else
        return false;
    return true;
}

Error parseExpr(ParsingContext *context, char* source, char **end, Node *result) {
    ExpectReturnValue expected;
    size_t token_cnt = 0;
    size_t token_length = 0;
    Token current_token;
    current_token.begin = source;
    current_token.end   = source;
    Error err = ok;

    Node *working_result = result;

    while ((err = lexAdvance(&current_token, &token_length, end)).type == ErrorType::NONE) {
        // std::cout << "lexed: ";
        // printToken(current_token);
        // std::cout << '\n';
        if(token_length == 0)
            return ok;
        if(parseInteger(&current_token, working_result)){
            return ok;
        } else {
            Node *symbol = nodeSymbolFromBuffer(current_token.begin, token_length);
            if(std::strcmp("func", symbol->value.symbol) == 0){
                working_result->type = NodeType::FUNCTION;
                lexAdvance(&current_token, &token_length, end);
                Node *function_name = nodeSymbolFromBuffer(current_token.begin, token_length);

                err = expected.expect(expected, "(", current_token, token_length, end);
                if (err.msg != "Continue") { return err; }
                if(!expected.found){
                    std::cout << "Function Name: " << function_name->value.symbol << '\n';
                    err.prepareError(ErrorType::SYNTAX, "Expected opening parenthesis for parameter list after function name");
                    return err;
                }

                Node *parameter_list = nodeAllocate();

                for(;;){
                    err = expected.expect(expected, ")", current_token, token_length, end);
                    if (err.msg != "Continue") { return err; }
                    if (expected.found) { break; }
                    if (expected.done) {
                        err.prepareError(ErrorType::SYNTAX, "Expected closing parenthesis for parameter list");
                        return err;
                    }

                    err = lexAdvance(&current_token, &token_length, end);
                    if (err.type != ErrorType::NONE) { return err; }
                    Node *parameter_name = nodeSymbolFromBuffer(current_token.begin, token_length);

                    err = expected.expect(expected, ":", current_token, token_length, end);
                    if (err.msg != "Continue") { return err; }
                    if (expected.done || !expected.found) {
                        err.prepareError(ErrorType::SYNTAX, "Parameter declaration requires a type annotation");
                        return err;
                    }

                    lexAdvance(&current_token, &token_length, end);
                    Node *parameter_type = nodeSymbolFromBuffer(current_token.begin, token_length);

                    Node *parameter = nodeAllocate();
                    nodeAddChild(parameter, parameter_name);
                    nodeAddChild(parameter, parameter_type);

                    nodeAddChild(parameter_list, parameter);

                    err = expected.expect(expected, ",", current_token, token_length, end);
                    if (err.msg != "Continue") { return err; }
                    if (expected.found) {
                        continue;
                    }

                    err = expected.expect(expected, ")", current_token, token_length, end);
                    if (err.msg != "Continue") { return err; }
                    if (!expected.found) {
                        err.prepareError(ErrorType::SYNTAX, "Expected closing parenthesis following parameter list");
                        return err;
                    }
                    break;
                }
                nodeAddChild(working_result, parameter_list);

                err = expected.expect(expected, ":", current_token, token_length, end);
                if (err.msg != "Continue") { return err; }
                if (expected.done || !expected.found) {
                    err.prepareError(ErrorType::SYNTAX, "Function definition requires return type annotation following parameter list");
                    return err;
                }

                lexAdvance(&current_token, &token_length, end);
                Node *function_return_type = nodeSymbolFromBuffer(current_token.begin, token_length);
                nodeAddChild(working_result, function_return_type);

                environmentSet(context->functions, function_name, working_result);

                err = expected.expect(expected, "{", current_token, token_length, end);
                if (err.msg != "Continue") { return err; }
                if (expected.done || !expected.found) {
                    err.prepareError(ErrorType::SYNTAX, "Function definition requires body following return type: \"{ a + b }\"");
                    return err;
                }

                context = parseContextCreate(context);
                context->operation = nodeSymbol("func");

                Node *param_it = working_result->children->children;
                environmentSet(context->variables,
                                param_it->children,
                                param_it->children->next_child);

                Node *function_body = nodeAllocate();
                Node *function_first_expression = nodeAllocate();
                nodeAddChild(function_body, function_first_expression);
                nodeAddChild(working_result, function_body);
                working_result = function_first_expression;
                continue;
            } else {
                err = expected.expect(expected, ":", current_token, token_length, end);
                if (err.msg != "Continue") { return err; }
                if (expected.found) {

                    err = expected.expect(expected, "=", current_token, token_length, end);
                    if (err.msg != "Continue") { return err; }
                    if (expected.found) {
                        Node *variable_binding = nodeAllocate();
                        if (!environmentGet(*context->variables, symbol, variable_binding)) {
                            std::cout << "ID of undeclared variable: " << symbol->value.symbol << '\n';
                            err.prepareError(ErrorType::GENERIC, "Reassignment of a variable that has not been declared!");
                            return err;
                        }
                        delete variable_binding;

                        working_result->type = NodeType::VARIABLE_REASSIGNMENT;
                        nodeAddChild(working_result, symbol);
                        Node *reassign_expr = nodeAllocate();
                        nodeAddChild(working_result, reassign_expr);

                        working_result = reassign_expr;
                        continue;
                    }

                    err = lexAdvance(&current_token, &token_length, end);
                    if (err.type != ErrorType::NONE) { return err; }
                    if (token_length == 0) { break; }
                    Node *type_symbol = nodeSymbolFromBuffer(current_token.begin, token_length);
                    Node *type_value = nodeAllocate();
                    parseGetType(context, type_symbol, type_value);
                    if (*type_value->isNone()) {
                        err.prepareError(ErrorType::TYPE, "Invalid type within variable declaration");
                        std::cout << "\nINVALID TYPE: " << type_symbol->value.symbol << '\n';
                        return err;
                    }
                    delete type_value;

                    Node *variable_binding = nodeAllocate();
                    if (environmentGet(*context->variables, symbol, variable_binding)) {
                        std::cout << "ID of redefined variable: " << symbol->value.symbol << '\n';
                        err.prepareError(ErrorType::GENERIC, "Redefinition of variable!");
                        return err;
                    }
                    delete variable_binding;

                    working_result->type = NodeType::VARIABLE_DECLARATION;

                    Node *value_expression = nodeNone();

                    nodeAddChild(working_result, symbol);
                    nodeAddChild(working_result, value_expression);

                    Node *symbol_for_env = nodeAllocate();
                    nodeCopy(symbol, symbol_for_env);
                    bool status = environmentSet(context->variables, symbol_for_env, type_symbol);
                    if (status != false) {
                        std::cout << "Variable: " << symbol_for_env->value.symbol << ", status: " << status << '\n';
                        err.prepareError(ErrorType::GENERIC, "Failed to define variable!");
                        return err;
                    }

                    err = expected.expect(expected, "=", current_token, token_length, end);
                    if (err.msg != "Continue") { return err; }
                    if (expected.found) {
                        working_result = value_expression;
                        continue;
                    }

                    return ok;
                }

                std::cout << "Unrecognized token: ";
                printToken(current_token);
                std::cout << '\n';

                err.prepareError(ErrorType::SYNTAX, "Unrecognized token reached during parsing");
                return err;
            }
        }
        if (!context->parent) { break; }
        Node *operation = context->operation;
        if (operation->type != NodeType::SYMBOL) {
            err.prepareError(ErrorType::TYPE, "Parsing context operation must be symbol. Likely internal error :(");
            return err;
        }
        if (std::strcmp(operation->value.symbol, "func") == 0) {
            err = expected.expect(expected, "}", current_token, token_length, end);
            if (err.msg != "Continue") { return err; }
            if (expected.found) { break; }
        }
        return ok;
    }

    return err;
}

//         err = expected.expect(expected, ":", current_token, token_length, end);
//         if (err.msg != "Continue") { return err; }
//         if(expected.found){
//             err = expected.expect(expected, "=", current_token, token_length, end);
//             if (err.msg != "Continue") { return err; }
//             if(expected.found){
//                 Node *variable_binding = nodeAllocate();
//                 if(!environmentGet(*context->variables, symbol, variable_binding)){
//                     std::cout << "ID of undeclared variable: \"" << symbol->value.symbol << "\"\n";
//                     err.prepareError(ErrorType::GENERIC, "Reassignment of a variable that has not been declared!");
//                     return err;
//                 }
//                 delete variable_binding;
                
//                 working_result->type = NodeType::VARIABLE_REASSIGNMENT;
//                 nodeAddChild(working_result, symbol);

//                 Node *reassign_expr = nodeAllocate();
//                 nodeAddChild(working_result, reassign_expr);

//                 working_result = reassign_expr;
//                 continue;
//             }
//             err = lexAdvance(&current_token, &token_length, end);
//             if(err.type != ErrorType::NONE)
//                 return err;
//             if(token_length == 0)
//                 break;
//             Node *type_symbol = nodeSymbolFromBuffer(current_token.begin, token_length);
//             Node *type_value = nodeAllocate();
//             if(environmentGet(*context->types, type_symbol, type_value) == 0){
//                 err.prepareError(ErrorType::TYPE, "Invalid type within variable declaration.");
//                 std::cout << "\nINVALID TYPE: " << type_symbol->value.symbol << '\n';
//                 return err;
//             }
//             delete type_value;

//             Node *variable_binding = nodeAllocate();
//             if(environmentGet(*context->variables, symbol, variable_binding)){
//                 std::cout << "ID of redefined variable: \"" << symbol->value.symbol << "\"\n";
//                 err.prepareError(ErrorType::GENERIC, "Redefinition of variable!");
//                 return err;
//             }

//             working_result->type = NodeType::VARIABLE_DECLARATION;

//             Node *value_expression = nodeNone();

//             nodeAddChild(working_result, symbol);
//             nodeAddChild(working_result, value_expression);

//             Node *symbol_for_env = nodeAllocate();
//             nodeCopy(symbol, symbol_for_env);
//             bool status = environmentSet(context->variables, symbol_for_env, type_symbol);
//             if(status != 1){
//                 std::cout << "Variable: " << symbol_for_env->value.symbol << ", status: " << status << '\n';
//                 err.prepareError(ErrorType::GENERIC, "Failed to define variable!");
//                 return err;
//             }
//             err = expected.expect(expected, "=", current_token, token_length, end);
//             if (err.msg != "Continue") { return err; }
//             if(expected.found){
//                 working_result = value_expression;
//                 continue;
//             }
//             return ok;
//         }
        
//         std::cout << "Unrecognized token: ";
//         printToken(current_token);
//         std::cout << '\n';

//         err.prepareError(ErrorType::SYNTAX, "Unrecognized token reached during parsing");
//         return err;
//     }
    
//     return err;
// }