#include<iostream>
#include<cstdlib>
#include<fstream>

std::streampos file_size(std::ifstream& file);
char *file_contents(char *path);
void print_usage(char **argv);

int main(int argc, char **argv){
    if(argc<2){
        print_usage(argv);
        exit(0);
    }
    
    return 0;
}

void print_usage(char **argv){
    std::cout<<"USAGE: "<<argv[0]<<" <path_to_file_to_compile>";
}

char *file_contents(char *path){
    std::ifstream file(path);
    if(!file.is_open()){
        std::cout<<"Failed to open the file at "<<path<<'\n';
        exit(1);
    }
    std::streampos size = file_size(file); 
}

std::streampos file_size(std::ifstream& file){
    if (!file){
        return 0;
    }
    std::streampos out=0;
    assert(file.seekg(0,std::ios::end).good() && "Could not set file position.");
    out = file.tellg();
    // if(fgetpos(file,&out)!=0){
    //     std::cout<<"fgetpos() failed: "<<errno<<'\n';
    //     return 0;
    // }
    file.seekg(0, std::ios::beg); // Reset file position to the beginning
    // if(fsetpos()!=0)
    return out;
}