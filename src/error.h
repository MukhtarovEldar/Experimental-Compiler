#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

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

    static Error createError(ErrorType t, const std::string &message) {
        return Error(t, message);
    }

    void prepareError(ErrorType t, const std::string &message) {
        type = t;
        msg = message;
    }
};

void printError(Error err);

extern Error ok;

#endif /* COMPILER_ERROR_H */