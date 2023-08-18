#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include <cstddef>
#include "error.h"

typedef struct Environment Environment;

struct Token{
    char *begin;
    char *end;
    Token *next;
};

void printToken(Token tok);
bool commentAtBeginning(Token token);
Error lex(char *source, Token *token);

enum class NodeType {
    NONE = 0,
    INTEGER,
    SYMBOL,
    VARIABLE_DECLARATION,
    VARIABLE_DECLARATION_INITIALIZED,
    VARIABLE_REASSIGNMENT,
    BINARY_OPERATOR,
    PROGRAM,
    MAX
};

struct Node {
    NodeType type;

    union NodeValue {
        long long integer;
        char *symbol;
    } value;

    Node *children;
    Node *next_child;

    bool isNone() const {
        return type == NodeType::NONE;
    }

    bool isInteger() const {
        return type == NodeType::INTEGER;
    }

    bool isSymbol() const {
        return type == NodeType::SYMBOL;
    }
};

void nodeAddChild(Node *parent, Node *new_child);
Node *nodeAllocate();
bool nodeCompare(Node *a, Node *b);
Node *nodeInteger(long long value);
Node *nodeSymbol(const char *symbol_string);
Node *nodeSymbolFromBuffer(char *buffer, size_t length);
void printNode(Node *node, size_t indent_level);
void deleteNode(Node *root);
void nodeCopy(Node *a, Node *b);
bool tokenStringEqual(const char *string, Token *token);

struct ExpectReturnValue {
    Error err;
    char found;
    char done;

    Error expect(const char *expected_string, Token current, size_t current_length, char **end);
};

bool parseInteger(Token *token, Node *node);
ExpectReturnValue lexExpect(char *expected, Token *current, size_t *current_length, char **end);

struct parsingContext{
    Environment *types;
    Environment *variables;
};

parsingContext *parseContextCreate();
Error parseExpr(parsingContext *context, char* source, char **end, Node* result);

#endif /* COMPILER_PARSER_H */