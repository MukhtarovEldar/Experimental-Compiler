#ifndef COMPILER_FILE_IO_H
#define COMPILER_FILE_IO_H

#include <iostream>
#include <fstream>

char *FileContents(char *path);
std::streamoff fileSize(std::fstream &file);

#endif /* COMPILER_FILE_IO_H */