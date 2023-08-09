#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>

enum class ErrorType {
    NONE = 0,
    TYPE,
    ARGUMENTS,
    TODO,
    GENERIC,
    SYNTAX,
    MAX
};

struct Error {
    ErrorType type;
    std::string msg;
    
    Error():
        type(ErrorType::NONE),
        msg("")
    {}

    Error(ErrorType t, const std::string &message): 
        type(t),
        msg(message)
    {}

    Error createError(ErrorType t, const std::string &message) {
        return Error(t, message);
    }

    void prepareError(ErrorType t, const std::string &message) {
        type = t;
        msg = message;
    }
};

Error ok = {ErrorType::NONE, ""};

enum class NodeType {
    NONE,
    INTEGER,
    SYMBOL,
    VARIABLE_DECLARATION,
    VARIABLE_DECLARATION_INITIALIZED,
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

struct Binding {
    Node *id;
    Node *value;
    Binding *next;
};

struct Environment {
    Environment *parent;
    Binding *bind;
};

const char *whitespace = " \r\n"; 
const char *delimiters = " \r\n,():"; 

struct Token{
    char *begin;
    char *end;
    Token *next;
};

struct parsingContext{
    // struct parsingContext *parent;
    Environment *types;
    Environment *variables;
};

void displayUsage(char **argv);
char *FileContents(char *path);
std::streamoff fileSize(std::fstream &file);
void printError(Error err);
void printToken(Token tok);
bool tokenStringEqual(const char *string, Token *token);
void deleteNode(Node *root);
Node *nodeAllocate();
void nodeAddChild(Node *parent, Node *new_child);
bool nodeCompare(Node *a, Node *b);
Node *nodeInteger(long long value);
Node *nodeSymbol(const char *symbol_string);
Node *nodeSymbolFromBuffer(char *buffer, size_t length);
void printNode(Node *node, size_t indent_level);
Environment *environmentCreate(Environment *parent);
int environmentSet(Environment *env, Node *id, Node *value);
bool environmentGet(Environment *env, Node *id, Node *result);
bool environmentGetBySymbol();
Error lex(char *source, Token *token);
bool parseInteger(Token *token, Node *node);
parsingContext *parseContextCreate();
Error parseExpr(parsingContext *context, char* source, char **end, Node* result);

int main(int argc, char **argv) {
    if (argc < 2) {
        displayUsage(argv);
        std::exit(0);
    }
    char *path = argv[1];
    char *contents = FileContents(path);
    assert(contents && "Could not allocate buffer for file contents.");
    if (contents) {
        // std::cout << "Contents of " << path << ":\n---\n" << contents << "\n---\n";
        parsingContext *context = parseContextCreate();
        Node *program = nodeAllocate();
        program->type = NodeType::PROGRAM;
        Node *expression = nodeAllocate();
        memset(expression, 0, sizeof(Node));
        char *contents_it = contents;
        char *last_contents_it = nullptr;
        Error err = parseExpr(context, contents_it, &contents_it, expression);
        nodeAddChild(program, expression);
        std::cout << '\n';

        printError(err);

        printNode(program, 0);

        deleteNode(program);
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
    std::streamoff size = fileSize(file);
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

std::streamoff fileSize(std::fstream &file) {
    if (!file.is_open()) {
        std::exit(0);
    }
    std::streampos original = file.tellg();
    file.seekg(0, std::ios::end);
    std::streamoff out = file.tellg();
    file.seekg(original);
    return out;
}

void printError(Error err) {
    if (err.type == ErrorType::NONE)
        return;
    std::cout << "ERROR: ";
    assert(ErrorType::MAX == ErrorType::MAX);
    switch (err.type) {
        default:
            std::cout << "Error type not recognized!";
            break;
        case ErrorType::TODO:
            std::cout << "Error to be implemented.";
            break;
        case ErrorType::SYNTAX:
            std::cout << "Invalid syntax!";
            break;
        case ErrorType::TYPE:
            std::cout << "Mismathced types!";
            break;
        case ErrorType::ARGUMENTS:
            std::cout << "Invalid arguments!";
            break;
        case ErrorType::GENERIC:
            break;
    }
    std::cout << '\n';
    if (!err.msg.empty())
        std::cout << "     : " << err.msg << '\n';
}

void printToken(Token tok){
    std::cout << std::string(tok.begin, tok.end - tok.begin);
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
        child = next_child;
    }
    if (root->isSymbol() && root->value.symbol) {
        delete root->value.symbol;
    }
    delete root;
}

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
    assert(static_cast<int>(NodeType::MAX) == 3 && "nodeCompare() must handle all node types.");
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
                return false;
            } else if (!a->value.symbol && !b->value.symbol) {
                return true;
            }
            return false;
        case NodeType::BINARY_OPERATOR:
            printf("TODO: nodeCompare() BINARY OPERATOR\n");
            break;
        case NodeType::VARIABLE_DECLARATION:
            printf("TODO: nodeCompare() VARIABLE DECLARATION\n");
            break;
        case NodeType::VARIABLE_DECLARATION_INITIALIZED:
            printf("TODO: nodeCompare() VARIABLE DECLARATION INITIALIZED\n");
            break;
        case NodeType::PROGRAM:
            printf("TODO: Compare two programs.\n");
            break;
        }
    return false;
}

Node *nodeInteger(long long value){
    Node *integer = nodeAllocate();
    integer->type = NodeType::INTEGER;
    integer->value.integer = value;
    integer->children = nullptr;
    integer->next_child = nullptr;
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
    assert(static_cast<int>(NodeType::MAX) == 7 && "printNode() must handle all node types.");
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

Environment *environmentCreate(Environment *parent){
    Environment *env = new Environment;
    assert(env && "Could not allocate memory for new environment.");
    env->parent = parent;
    env->bind = nullptr;
    return env;
}

/**
 * @retval 0 Failure.
 * @retval 1 Creation of new binding.
 * @retval 2 Existing binding value overwrite (ID unused).
 */
int environmentSet(Environment *env, Node *id, Node *value){
    // Over-writing an existing value
    if (!env || !id || !value) {
        return 0;
    }
    Binding *binding_it = env->bind;
    while(binding_it){
        if(nodeCompare(binding_it->id, id)){
            binding_it->value = value;
            return 2;
        }
        binding_it = binding_it->next;
    }
    
    // Creating a new binding
    Binding *binding = new Binding;
    assert(binding && "Could not allocate new binding for the environment.");
    binding->id = id;
    binding->value = value;
    binding->next = env->bind;
    env->bind = binding;
    return 1;
}

bool environmentGet(Environment env, Node *id, Node *result){
    Binding *binding_it = env.bind;
    while(binding_it){
        if(nodeCompare(binding_it->id, id)){
            *result = *binding_it->value;
            return true;
        }
        binding_it = binding_it->next;
    }
    return false;
}

bool environmentGetSymbol(Environment env, char *symbol, Node *result) {
    Node *symbol_node = nodeSymbol(symbol);
    bool status = environmentGet(env, symbol_node, result);
    delete symbol_node;
    return status;
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
    Token current_token;
    current_token.begin = source;
    current_token.end   = source;
    Error err = ok;

    while ((err = lex(current_token.end, &current_token)).type == ErrorType::NONE) {
        *end = current_token.end;
        size_t token_length = current_token.end - current_token.begin; 
        if(token_length == 0)
            break;
        if(parseInteger(&current_token, result)){
            Node lhs_integer = *result;
            err = lex(current_token.end, &current_token);
            if(err.type != ErrorType::NONE)
                return err;
            *end = current_token.end;
        } else{
            Node *symbol = nodeSymbolFromBuffer(current_token.begin, token_length);

            err = lex(current_token.end, &current_token);
            if(err.type != ErrorType::NONE)
                return err;
            *end = current_token.end;
            size_t token_length = current_token.end - current_token.begin;
            if(token_length == 0)
                break;
            if(tokenStringEqual(":", &current_token)) {
                err = lex(current_token.end, &current_token);
                if(err.type != ErrorType::NONE)
                    return err;
                *end = current_token.end;
                size_t token_length = current_token.end - current_token.begin;
                if (token_length == 0)
                    break;
                Node *expected_type_symbol = nodeSymbolFromBuffer(current_token.begin, token_length);
                bool status = environmentGet(*context->types, expected_type_symbol, result);
                if(status == 0) {
                    err.prepareError(ErrorType::TYPE, "Invalid type within variable declaration.");
                    std::cout << "\nINVALID TYPE: " << expected_type_symbol->value.symbol << '\n';
                    return err;
                }else{
                    Node *var_decl = nodeAllocate();
                    var_decl->type = NodeType::VARIABLE_DECLARATION;

                    Node *type_node = nodeAllocate();
                    type_node->type = result->type;

                    nodeAddChild(var_decl, type_node);
                    nodeAddChild(var_decl, symbol);

                    *result = *var_decl;
                    delete var_decl;

                    return ok;
                }
            }
            
            std::cout << "Unrecognized token: ";
            printToken(current_token);
            std::cout << '\n';
            return err;
        }
        std::cout << "Intermediate node: ";
        printNode(result, 0);
        std::cout << '\n';
    }
    
    return err;
}