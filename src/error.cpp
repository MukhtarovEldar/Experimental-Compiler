
#include "error.h"

#include <iostream>
#include <cstddef>
#include <cassert>

Error ok = {ErrorType::NONE, ""};

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