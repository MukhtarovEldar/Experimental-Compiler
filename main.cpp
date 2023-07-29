#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>

enum class ErrorType {
    ERROR_NONE = 0,
    ERROR_TYPE,
    ERROR_ARGUMENTS,
    ERROR_TODO,
    ERROR_GENERIC,
    ERROR_SYNTAX,
    ERROR_MAX,
};

struct Error {
    ErrorType type;
    std::string msg;
};

enum class NodeType {
        NODE_TYPE_NONE,
        NODE_TYPE_INTEGER,
        NODE_TYPE_PROGRAM,
        NODE_TYPE_MAX,
};

struct Node {
    NodeType type;
    union NodeValue {
        long long integer;
    } value;

    Node *children;
    Node *next_child;

    bool isNone() const {
        return type == NodeType::NODE_TYPE_NONE;
    }

    bool isInteger() const {
        return type == NodeType::NODE_TYPE_INTEGER;
    }
};

struct Program {
    Node* root;
};

struct Binding {
    Node id;
    Node value;
    Binding* next;
};

struct Environment {
    Environment *parent;
    Binding *bind;
};

Error ok = {ErrorType::ERROR_NONE, ""};
#define ERROR_CREATE(n, t, msg)   (n) = { (t), (msg) } 
#define ERROR_PREP(n, t, message) (n).type = (t); (n).msg = (message)

const char *whitespace = " \r\n"; 
const char *delimiters = " \r\n,():"; 

struct Token{
    char *begin;
    char *end;
    Token *next;
};

long fileSize(std::fstream &file);
void displayUsage(char **argv);
char *FileContents(char *path);
void printError(Error err);
Error lex(char *source, char **beg, char **end);
Error parseExpr(char *source, Node *result );
Token *tokenCreate();
void printToken(Token tok);
void printTokens(Token *root);
void deleteTokens(Token *root);
bool tokenStringEqual(char *string, Token *token);
void deleteNode(Node *root);
bool parseInteger(Token *token, Node *node);
void printNodeImpl(Node *node);
void printNode(Node *node, size_t indent_level);
bool nodeCompare(Node *a, Node *b);
void environmentSet(Environment env, Node id, Node value);
Node environmentGet(Environment env, Node id);
Environment *environmentCreate(Environment *parent);

int main(int argc, char **argv) {
    if (argc < 2) {
        displayUsage(argv);
        std::exit(0);
    }
    char *path = argv[1];
    char *contents = FileContents(path);
    if (contents) {
        // std::cout << "Contents of " << path << ":\n---\n" << contents << "\n---\n";
        Node expression;
        Error err = parseExpr(contents, &expression);
        printError(err);
        delete[] contents;
    }
    return 0;
}

void displayUsage(char **argv)
{
    std::cout << "Usage: " << argv[0] << " <file_path>";
}

char *FileContents(char *path) {
    std::fstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::cout << "Failed to open the file at " << path << '\n';
        std::exit(0);
    }
    long size = fileSize(file);
    char *contents = new char[size + 1];
    file.read(contents, size);
    if (!file.is_open()) {
        std::cout << "Error while reading!";
        delete[] contents;
        std::exit(0);
    }
    contents[size] = '\0';
    return contents;
}

long fileSize(std::fstream &file) {
    if (!file.is_open()) {
        std::exit(0);
    }
    std::streampos original = file.tellg();
    file.seekg(0, std::ios::end);
    long out = file.tellg();
    file.seekg(original);
    return out;
}

void printError(Error err) {
    if (err.type == ErrorType::ERROR_NONE)
        return;
    std::cout << "ERROR: ";
    assert(ErrorType::ERROR_MAX == ErrorType::ERROR_MAX);
    switch (err.type) {
        default:
            std::cout << "Error type not recognized!";
            break;
        case ErrorType::ERROR_TODO:
            std::cout << "Error to be implemented.";
            break;
        case ErrorType::ERROR_SYNTAX:
            std::cout << "Invalid syntax!";
            break;
        case ErrorType::ERROR_TYPE:
            std::cout << "Mismathced types!";
            break;
        case ErrorType::ERROR_ARGUMENTS:
            std::cout << "Invalid arguments!";
            break;
        case ErrorType::ERROR_GENERIC:
            break;
    }
    std::cout << '\n';
    if (!err.msg.empty())
        std::cout << "     : " << err.msg << '\n';
}

Token* tokenCreate() {
    Token* token = new Token;
    assert(token && "Could not allocate memory for token!");
    token->begin = nullptr;
    token->end = nullptr;
    token->next = nullptr;
    return token;
}

void printToken(Token tok){
    std::cout << std::string(tok.begin, tok.end - tok.begin);
}

void printTokens(Token *root) {
    Token *tmp = root;
    size_t cnt = 1; 
    while (tmp) {
        std::cout << "Token " << cnt << ": "; 
        if (tmp->begin && tmp->end)
            std::cout << std::string(tmp->begin, tmp->end - tmp->begin);
        std::cout << '\n';
        tmp = tmp->next;
        cnt++;
    }
}

void deleteTokens(Token *root) {
    while (root) {
        Token *current_token = root;
        root = root->next;
        delete current_token;
    }
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

void deleteNode(Node *root){
    if(!root)
        return;
    Node *child = root->children;
    Node *next_child = nullptr;
    while(child){
        next_child = child->next_child;
        deleteNode(child);
        child = child->next_child;
    }
    delete root;
}

bool nodeCompare(Node *a, Node *b){
    if(!a || !b){
        if(!a && !b)
            return true;
        return false;
    }
    assert(static_cast<int>(NodeType::NODE_TYPE_MAX) == 3 && "nodeCompare() must handle all node types.");
    if(a->type != b->type)
        return false;
    switch(a->type){
        case NodeType::NODE_TYPE_NONE:
            if(b->isNone())
                return true;
            return false;
            break;
        case NodeType::NODE_TYPE_INTEGER:
            if(a->value.integer == b->value.integer)
                return true;
            return false;
            break;
        case NodeType::NODE_TYPE_PROGRAM:
            break;
    }
    // TO-DO
}

void printNode(Node *node, size_t indent_level){
    if(!node)
        return;
    for(size_t i = 0; i < indent_level; i++){
        std::cout << ' ';
    }
    assert(static_cast<int>(NodeType::NODE_TYPE_MAX) == 3 && "printNode() must handle all node types.");
    switch(node->type){
        default:
            std::cout << "UNKNOWN";
            break;
        case NodeType::NODE_TYPE_NONE:
            std::cout << "NONE";
            break;
        case NodeType::NODE_TYPE_INTEGER:
            std::cout << "INT:" << node->value.integer;
            break;
        case NodeType::NODE_TYPE_PROGRAM:
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

Environment *environmentCreate(Environment *parent){
    Environment *env = new Environment;
    assert(env && "Could not allocate memory for new environment.");
    env->parent = parent;
    env->bind = nullptr;
    return env;
}

void environmentSet(Environment env, Node id, Node value){
    Binding *binding = new Binding;
    assert(binding && "Could not allocate new binding for the environment.");
    binding->id = id;
    binding->value = value;
    binding->next = env.bind;
    env.bind = binding;
}

Node environmentGet(Environment env, Node id){
    Binding *binding_it = env.bind;
    while(binding_it){
        if(nodeCompare(&binding_it->id, &id))
            return binding_it->value;
        binding_it = binding_it->next;
    }
    Node value;
    value.type = NodeType::NODE_TYPE_NONE;
    value.children = nullptr;
    value.next_child = nullptr;
    value.value.integer = 0;
    return value;
}

Error lex(char* source, Token *token) {
    Error err = ok;
    if (!source || !token) {
        ERROR_PREP(err, ErrorType::ERROR_ARGUMENTS, "Could not lex empty source!");
        return err;
    }
    token->begin = source;
    token->begin += strspn(token->begin, whitespace);
    token->end = token->begin;
    if (*(token->end) == '\0') {
        return err;
    }
    token->end += strcspn(token->begin, delimiters);
    if (token->end == token->begin) {
        token->end += 1;
    }
    return err;
}

bool parseInteger(Token *token, Node *node){
    if(!token || !node)
        return false;
    if(token->end - token->begin == 1 && *(token->begin) == '0'){
        node->type = NodeType::NODE_TYPE_INTEGER;
        node->value.integer = 0;
    } else if((node->value.integer = strtoll(token->begin, nullptr, 10)) != 0){
        node->type = NodeType::NODE_TYPE_INTEGER;
        // std::cout << "Found integer " << node->value.integer << "!\n";
    }
    else{
        return false;
    }
    return true;
}

Error parseExpr(char* source, Node* result) {
    size_t token_cnt = 0;
    Token current_token;
    current_token.next = nullptr;
    current_token.begin = source;
    current_token.end   = source;
    Error err = ok;

    Node *root = new Node{};
    assert(root && "Could not allocate memory for AST Root.");
    root->type = NodeType::NODE_TYPE_PROGRAM;

    Node working_node;
    while ((err = lex(current_token.end, &current_token)).type == ErrorType::ERROR_NONE) {
        working_node.children   = nullptr;
        working_node.next_child = nullptr;
        working_node.type = NodeType::NODE_TYPE_NONE;
        working_node.value.integer = 0;
        size_t token_length = current_token.end - current_token.begin; 
        if(token_length == 0)
            break;
        if(parseInteger(&current_token, &working_node)){
            Token integer;
            memcpy(&integer,&current_token,sizeof(Token));
            err = lex(current_token.end, &current_token);
            if(err.type != ErrorType::ERROR_NONE)
                return err;
        }else{
            std::cout << "Unrecognized token: ";
            printToken(current_token);
            std::cout << '\n';
        }
        std::cout << "Found node: ";
        printNode(&working_node, 0);
        std::cout << '\n';
    }

    return err;
}