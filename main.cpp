#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>

typedef enum ErrorType {
    ERROR_NONE = 0,
    ERROR_TYPE,
    ERROR_ARGUMENTS,
    ERROR_TODO,
    ERROR_GENERIC,
    ERROR_SYNTAX,
    ERROR_MAX,
} ErrorType;

typedef struct Error {
    ErrorType type;
    const char* msg;
} Error;

typedef struct Node{
    enum NodeType {
        NODE_TYPE_NONE,
        NODE_TYPE_INTEGER,
        NODE_TYPE_MAX,
    } type;
    union NodeValue{
        long long integer;
    } value;
    struct Node *children[3];
    
} Node;

#define nonep(node) ((node).type == NODE_TYPE_NONE)
#define integerp(node) ((node).type == NODE_TYPE_INTEGER)

typedef struct Binding{
    std::string id;
    Node *value;
    struct Binding *next;
} Binding;

typedef struct Environement{
    struct Environment *parent;
    Binding *bind;
} Environement;

Error ok = {ERROR_NONE, nullptr};
#define ERROR_CREATE(n, t, msg)   (n) = { (t), (msg) } 
#define ERROR_PREP(n, t, message) (n).type = (t); (n).msg = (message)

const char *whitespace = " \r\n"; 
const char *delimiters = " \r\n,():"; 

long fileSize(std::fstream &file);
void displayUsage(char **argv);
char *FileContents(char *path);
void printError(Error err);
Error lex(char *source, char **beg, char **end);
Error parseExpr(char *source);

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

void printError(Error err){
    if(err.type == ERROR_NONE)
        return;
    std::cout<<"ERROR: ";
    assert(ERROR_MAX == 6);
    switch(err.type){
        default:
            std::cout<<"Error type not recognized!";
            break;
        case ERROR_TODO:
            std::cout<<"Error to be implemented.";
            break;
        case ERROR_SYNTAX:
            std::cout<<"Invalid syntax!";
            break;
        case ERROR_TYPE:
            std::cout<<"Mismathced types!";
            break;
        case ERROR_ARGUMENTS:
            std::cout<<"Invalid arguments!";
            break;
        case ERROR_GENERIC:
            break;
    }
    std::putchar('\n');
    if(err.msg)
        std::cout<<"     : "<<err.msg<<'\n';
}

Error lex(char *source, char **beg, char **end){
    Error err = ok;
    if(!source || !beg || !end){
        ERROR_PREP(err, ERROR_ARGUMENTS, "Can not lex empty source!");
        return err;
    }
    *beg = source;
    *beg += strspn(*beg, whitespace);
    *end = *beg;
    if(**end == '\0'){
        return err;
    }
    *end += strspn(*beg, delimiters);
    if(*end == *beg ){
        *end += 1;
    }
    return err;
}

void EnvironmentSet(){

}

Error parseExpr(char *source, Node *result ){
    char *beg = source, *end = source;
    // char *prev_token = source;
    Error err = ok;
    while((err = lex(end, &beg, &end)).type == ERROR_NONE){
        if(end - beg == 0)
            break;
        std::cout<<"lexed: "<< end - beg << beg;
    }
    return err;
}