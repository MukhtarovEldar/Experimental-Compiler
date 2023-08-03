# Experimental C++ Compiler

This is an experimental C++ compiler written in C++. The purpose of this project is to explore the basics of compiler design and gain a better understanding of how programming languages are parsed and translated into machine code.

## Status

**Work in Progress**

This project is still under development, and there's quite a lot to finish. Some of the planned features are yet to be implemented, and there might be known issues that need to be addressed.

## Features and Plans

- [X] Lexical Analysis: Tokenizing the input source code.
- [X] Syntax Analysis: Parsing the tokens to build an Abstract Syntax Tree (AST).
- [ ] Semantic Analysis: Checking for semantic correctness and building a symbol table.
- [ ] Intermediate Code Generation: Converting the AST into intermediate code.
- [ ] Optimization: Implementing basic optimizations on the intermediate code.
- [ ] Code Generation: Translating the optimized intermediate code into target machine code (x86, ARM, etc.).

## Getting Started

### Prerequisites

- C++ compiler (supporting C++11 or higher)
- CMake (for building the project)

### Building the Compiler

1. Clone this repository to your local machine:

   ```bash
   git clone https://github.com/MukhtarovEldar/Experimental-Compiler.git
   cd Experimental-Compiler

2. Build the project using CMake:

    ```bash
    cmake -B build --build build

### Contributing

Contributions are welcome! If you find any bugs, have suggestions, or want to add new features, feel free to open an issue or submit a pull request.

### License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### Resources

This project was inspired by the following repository: [Intercept](https://github.com/LensPlaysGames/Intercept.git). I am writing this compiler in C++ with insights and ideas drawn from that repository.
