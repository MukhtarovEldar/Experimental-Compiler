#include <iostream>
#include <fstream>
#include <cstdio>
#include <cerrno>

std::streampos file_size(std::FILE *file);
void print_usage(char **argv);
char *file_contents(char *path);

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv);
        exit(0);
    }

    return 0;
}

void print_usage(char **argv) {
    std::cout << "USAGE: " << argv[0] << " <path_to_file_to_compile>";
}

char* file_contents(char *path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open the file at " << path << '\n';
        exit(1);
    }
    std::streampos size = file_size(file);
    // Rest of the code for reading file contents
    // ...

    return nullptr; // Replace with the appropriate return value
}

std::streampos file_size(std::FILE *file) {
    if (!file) {
        return 0;
    }
    std::streampos original = 0;
    if (std::fgetpos(file, &original) != 0) {
        std::cout << "fgetpos() failed: " << errno << '\n';
        return 0;
    }
    std::fseek(file, 0, SEEK_END);
    std::streampos size = std::ftell(file);
    if (std::fsetpos(file, &original) != 0) {
        std::cout << "fsetpos() failed: " << errno << '\n';
    }

    return size; // return original;
}
