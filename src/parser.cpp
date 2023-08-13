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
    token->begin += strspn(token->begin, whitespace);
    token->end = token->begin;
    if (*(token->end) == '\0')
        return err;
    while(commentAtBeginning(*token)){
        token->begin = strpbrk(token->begin, "\n");
        if (!token->begin) {
            token->end = token->begin;
            return err;
        }
        token->begin += strspn(token->begin, whitespace);
        token->end = token->begin;
    }
    if (*(token->end) == '\0')
        return err;
    token->end += strcspn(token->begin, delimiters);
    if (token->end == token->begin) {
        token->end += 1;
    }
    return err;
}

bool tokenStringEqual(const char* string, Token *token){
    if(!string || !token) return false;
    char *beg = token->begin;
    while(*string && token->begin < token->end){
        if(*string != *beg)
            return false;
        string++;
        beg++;
    }
    return true;
}

void printToken(Token tok){
    std::cout << std::string(tok.begin, tok.end - tok.begin);
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
    Node *allocated_child = nodeAllocate();
    assert(allocated_child && "Could not allocate new child Node for AST");
    *allocated_child = *new_child;
    if(parent->children){
        Node *child = parent->children;
        while(child->next_child)
            child = child->next_child;
        child->next_child = allocated_child;
    }
    else
        parent->children = allocated_child;
}

bool nodeCompare(Node *a, Node *b){
    if(!a || !b){
        if(!a && !b)
            return true;
        return false;
    }
    assert(static_cast<int>(NodeType::MAX) == 8 && "nodeCompare() must handle all node types.");
    if(a->type != b->type)
        return false;
    switch(a->type){
        case NodeType::NONE:
            if(b->isNone())
                return true;
            return false;
        case NodeType::INTEGER:
            if(a->value.integer == b->value.integer)
                return true;
            return false;
        case NodeType::SYMBOL:
            if (a->value.symbol && b->value.symbol) {
                if (strcmp(a->value.symbol, b->value.symbol) == false)
                    return true;
                break;
            } else if (!a->value.symbol && !b->value.symbol) {
                return true;
            }
            break;
        case NodeType::BINARY_OPERATOR:
            std::cout << "TODO: nodeCompare() BINARY OPERATOR\n";
            break;
        case NodeType::VARIABLE_REASSIGNMENT:
            std::cout << "TODO: node_compare() VARIABLE REASSIGNMENT\n";
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

Node *nodeInteger(long long value){
    Node *integer = nodeAllocate();
    integer->type = NodeType::INTEGER;
    integer->value.integer = value;
    return integer;
}

Node *nodeSymbol(const char *symbol_string){
    Node *symbol = nodeAllocate();
    symbol->type = NodeType::SYMBOL;
    symbol->value.symbol = strdup(symbol_string);
    return symbol;
}

Node *nodeSymbolFromBuffer(char *buffer, size_t length) {
    assert(buffer && "Can not create AST symbol node from NULL buffer.");
    char *symbol_string = new char[length + 1];
    assert(symbol_string && "Could not allocate memory for symbol string.");
    memcpy(symbol_string, buffer, length);
    symbol_string[length] = '\0';
    Node *symbol = nodeAllocate();
    symbol->type = NodeType::SYMBOL;
    symbol->value.symbol = symbol_string;
    return symbol;
}

void printNode(Node *node, size_t indent_level){
    if(!node)
        return;
    for(size_t i = 0; i < indent_level; i++){
        std::cout << ' ';
    }
    assert(static_cast<int>(NodeType::MAX) == 8 && "printNode() must handle all node types.");
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
    if (root->isSymbol() && root->value.symbol) {
        delete root->value.symbol;
    }
    delete root;
}

Error lexAdvance(Token *token, size_t *token_length, char **end) {
    Error err = ok;
    if(!token || !token_length || !end) {
        err.createError(ErrorType::ARGUMENTS, "lexAdvance(): pointer arguments must not be null!");
        return err;
    }
    err = lex(token->end, token);
    *end = token->end;
    if(err.type != ErrorType::NONE)
        return err;
    *token_length = token->end - token->begin;
    return err;
}

bool parseInteger(Token *token, Node *node){
    if(!token || !node)
        return false;
    char *end;
    /// Remove the condition for 0, if that becomes redundant  
    if(token->end - token->begin == 1 && *(token->begin) == '0'){
        node->type = NodeType::INTEGER;
        node->value.integer = 0;
    } else if((node->value.integer = strtoll(token->begin, &end, 10)) != 0){
        if(end != token->end){
            return false;
        }
        node->type = NodeType::INTEGER;
    }
    else{
        return false;
    }
    return true;
}

void nodeCopy(Node *a, Node *b) {
    if (!a || !b)
        return;
    b->type = a->type;
    switch (a->type) {
    default:
        b->value = a->value;
        break;
    case NodeType::SYMBOL:
        b->value.symbol = strdup(a->value.symbol);
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

parsingContext *parseContextCreate(){
    parsingContext *ctx = new parsingContext();
    assert(ctx && "Could not allocate for parsing context.");
    ctx->types = environmentCreate(nullptr);
    if(environmentSet(ctx->types, nodeSymbol("integer"), nodeInteger(0)) == 0){
        std::cout << "ERROR: Failed to set built-in type in types environment.\n";
    }
    ctx->variables = environmentCreate(nullptr);
    return ctx;
}

Error parseExpr(parsingContext *context, char* source, char **end, Node *result) {
    size_t token_cnt = 0;
    size_t token_length = 0;
    Token current_token;
    current_token.begin = source;
    current_token.end   = source;
    Error err = ok;
    while ((err = lexAdvance(&current_token, &token_length, end)).type == ErrorType::NONE) {
        std::cout << "lexed: ";
        printToken(current_token);
        std::cout << '\n';
        if(token_length == 0)
            return ok;
        if(parseInteger(&current_token, result))
            return ok;
        
        Node *symbol = nodeSymbolFromBuffer(current_token.begin, token_length);

        err = lexAdvance(&current_token, &token_length, end);
        if(err.type != ErrorType::NONE)
            return err;
        if(token_length == 0)
            return ok;
        if(tokenStringEqual(":", &current_token)) {
            err = lexAdvance(&current_token, &token_length, end);
            if(err.type != ErrorType::NONE)
                return err;
            if(token_length == 0)
                break;
            Node *variable_binding = nodeAllocate();
            if(environmentGet(*context->variables, symbol, variable_binding)) {
                if (tokenStringEqual("=", &current_token)) {
                    Node *reassign_expr = nodeAllocate();
                    err = parseExpr(context, current_token.end, end, reassign_expr);
                    if (err.type != ErrorType::NONE) {
                        delete variable_binding;
                        return err;
                    }
                    
                    if (reassign_expr->type != variable_binding->type) {
                        delete variable_binding;
                        err.prepareError(ErrorType::TYPE, "Variable assignment expression has mismatched type.");
                        return err;
                    }
                    Node *var_reassign = nodeAllocate();
                    var_reassign->type = NodeType::VARIABLE_REASSIGNMENT;

                    nodeAddChild(var_reassign, reassign_expr);
                    nodeAddChild(var_reassign, symbol);

                    *result = *var_reassign;
                    delete var_reassign;

                    return ok;
                }
                std::cout << "ID of redefined variable: " << symbol->value.symbol << '\n';
                err.prepareError(ErrorType::GENERIC, "Redefinition of variable!");
                return err;
            } 
            delete variable_binding;
            Node *expected_type_symbol = nodeSymbolFromBuffer(current_token.begin, token_length);
            if(environmentGet(*context->types, expected_type_symbol, result) == 0){
                err.prepareError(ErrorType::TYPE, "Invalid type within variable declaration.");
                std::cout << "\nINVALID TYPE: " << expected_type_symbol->value.symbol << '\n';
                return err;
            }

            Node *var_decl = nodeAllocate();
            var_decl->type = NodeType::VARIABLE_DECLARATION;

            Node *type_node = nodeAllocate();
            type_node->type = result->type;

            nodeAddChild(var_decl, symbol);

            char *old_end = *end;
            lexAdvance(&current_token, &token_length, end);
            if(err.type != ErrorType::NONE)
                return err;
            if(token_length == 0)
                break;
            if(tokenStringEqual("=", &current_token)){
                Node *assigned_expr = nodeAllocate();
                err = parseExpr(context, current_token.end, end, assigned_expr);
                if(err.type != ErrorType::NONE)
                    return err;
                if(assigned_expr->type != type_node->type){
                    delete assigned_expr;
                    err.prepareError(ErrorType::TYPE, "Variable assignment expression has mismatched type.");
                    return err;
                }
                type_node->value = assigned_expr->value;
                delete assigned_expr;
            }else
                *end = old_end;
            
            *result = *var_decl;
            delete var_decl;

            Node *symbol_for_env = nodeAllocate();
            nodeCopy(symbol, symbol_for_env);
            bool status = environmentSet(context->variables, symbol_for_env, type_node);
            if(status != 1){
                err.prepareError(ErrorType::GENERIC, "Failed to define variable!");
                return err;
            }

            return ok;
        
        }
        
        std::cout << "Unrecognized token: ";
        printToken(current_token);
        std::cout << '\n';

        err.prepareError(ErrorType::SYNTAX, "Unrecognized token reached during parsing");
        return err;
    }
    
    return err;
}
