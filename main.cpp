#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

long file_size(std::FILE *file);
void print_usage(char **argv);
char *file_contents(char *path);

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv);
        exit(0);
    }
    char *path = argv[1];
    char *contents = file_contents(path);
    if(contents){
        std::cout<<"Contents of "<<path<<":\n---\n"<<contents<<"\n---\n";
        delete[] contents;
    }
    return 0;
}

void print_usage(char **argv) {
    std::cout << "USAGE: " << argv[0] << " <path_to_file_to_compile>";
}

char *file_contents(char *path) {
    std::FILE* file = std::fopen(path,"r");
    if (!file) {
        std::cout << "Failed to open the file at " << path << '\n';
        return nullptr;
    }
    long size = file_size(file);
    char *contents = new char[size + 1];
    char *write_it = contents;
    std::size_t bytes_read = 0;
    while(bytes_read < size){
        std::size_t bytes_read_this_iteration = std::fread(write_it, 1, size - bytes_read, file);
        bytes_read += bytes_read_this_iteration;
        write_it += bytes_read_this_iteration;
    }
    if (bytes_read < size) {
        delete[] contents;
        return nullptr;
    }
    contents[size] = '\0';
    std::fclose(file); // remember to close
    return contents;
}

long file_size(std::FILE *file) {
    if (!file) {
        return 0;
    }
    std::fpos_t original = 0;
    if (std::fgetpos(file, &original) != 0) {
        std::cout << "fgetpos() failed: " << errno << '\n';
        return 0;
    }
    std::fseek(file, 0, SEEK_END);
    long out = std::ftell(file);
    if (std::fsetpos(file, &original) != 0) {
        std::cout << "fsetpos() failed: " << errno << '\n';
    }
    return out;
}
