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

struct Node {
    enum class NodeType {
        NODE_TYPE_NONE,
        NODE_TYPE_INTEGER,
        NODE_TYPE_PROGRAM,
        NODE_TYPE_MAX,
    } type;

    union NodeValue {
        long long integer;
    } value;

    std::vector<Node*> children;

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
    std::string id;
    Node* value;
    Binding* next;
};

struct Environment {
    Environment* parent;
    Binding* bind;
};

Error ok = {ErrorType::ERROR_NONE, ""};
#define ERROR_CREATE(n, t, msg)   (n) = { (t), (msg) } 
#define ERROR_PREP(n, t, message) (n).type = (t); (n).msg = (message)

const char *whitespace = " \r\n"; 
const char *delimiters = " \r\n,():"; 

struct Token{
    char *beginning;
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
void printTokens(Token *root);
void deleteTokens(Token *root);

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
    std::putchar('\n');
    if (!err.msg.empty())
        std::cout << "     : " << err.msg << '\n';
}

// void EnvironmentSet(){

// }

Token* tokenCreate() {
    Token* token = new Token;
    assert(token && "Can not allocate memory for token!");
    token->beginning = nullptr;
    token->end = nullptr;
    token->next = nullptr;
    return token;
}

void printTokens(Token *root) {
    Token *tmp = root;
    size_t cnt = 1; 
    while (tmp) {
        std::cout << "Token " << cnt << ": "; 
        if (tmp->beginning && tmp->end)
            std::cout << std::string(tmp->beginning, tmp->end - tmp->beginning);
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

Error lex(char* source, Token *token) {
    Error err = ok;
    if (!source || !token) {
        ERROR_PREP(err, ErrorType::ERROR_ARGUMENTS, "Can not lex empty source!");
        return err;
    }
    token->beginning = source;
    token->beginning += strspn(token->beginning, whitespace);
    token->end = token->beginning;
    if (*(token->end) == '\0') {
        return err;
    }
    token->end += strcspn(token->beginning, delimiters);
    if (token->end == token->beginning) {
        token->end += 1;
    }
    return err;
}

Error parseExpr(char* source, Node* result) {
    Token *tokens = nullptr;
    Token *token_it = tokens;
    Token current_token;
    current_token.next = nullptr;
    current_token.beginning = source;
    current_token.end       = source;
    // char *prev_token = source;
    Error err = ok;
    while ((err = lex(current_token.end, &current_token)).type == ErrorType::ERROR_NONE) {
        if (current_token.end == current_token.beginning)
            break;

        if (tokens) {
            token_it->next = tokenCreate();
            std::memcpy(token_it->next, &current_token, sizeof(Token));
            token_it = token_it->next;
        } else {
            tokens = tokenCreate();
            std::memcpy(tokens, &current_token, sizeof(Token));
            token_it = tokens;
        }

        // Token *rest_of_tokens = tokens;
        // tokens = tokenCreate();
        // std::memcpy(tokens, &current_token, sizeof(Token));
        // tokens->next = rest_of_tokens;

        std::cout << "lexed: " << std::string(current_token.beginning, current_token.end - current_token.beginning) << '\n';
    }

    printTokens(tokens);

    Node *root = new Node;
    std::memset(root, 0, sizeof(root));
    assert(root && "Can not allocate memory for AST Node!");
    token_it = tokens;
    while(token_it){
        token_it = token_it->next;
    }

    deleteTokens(tokens);

    return err;
}