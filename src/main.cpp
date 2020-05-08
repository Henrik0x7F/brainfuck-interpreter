#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include "bf_interpreter.hpp"


static void print_usage()
{
    std::cout << "usage bf [-edh] [file...]\n" \
              << "\t-e \trun code directly\n" \
              << "\t-d \tdump intermediate code representation\n" \
              << "\t-h \tshow help message\n";
}

int main(int argc, char** argv)
{
    std::vector<char> code;
    std::string bf_file;
    bool disassemble{};

    if(argc < 2)
    {
        print_usage();
        return 0;
    }

    for(std::size_t i{}; i < argc; ++i)
    {
        if(strcmp(argv[i], "-d") == 0)
        {
            disassemble = true;
        }
        if(strcmp(argv[i], "-h") == 0)
        {
            print_usage();
            return 0;
        }
        if(strcmp(argv[i], "-e") == 0 && argc > i + 1)
        {
            code.resize(strlen(argv[i + 1]));
            std::copy_n(argv[i + 1], code.size(), code.begin());
        }
        else
        {
            bf_file = argv[i];
        }
    }

    if(code.empty())
    {
        std::ifstream code_file(bf_file, std::ios::ate);
        if(!code_file)
        {
            std::clog << "Failed to open code file " << std::quoted(bf_file) << '\n';
            return 1;
        }
        code.resize(code_file.tellg());
        code_file.seekg(0);
        code_file.read(code.data(), code.size());
        if(!code_file.good())
        {
            std::clog << "Failed to read code file\n";
            return 2;
        }
    }

    BF_Interpreter bf;
    if(!bf.load_code(code.data(), code.size()))
        return 3;

    if(disassemble)
    {
        std::clog << "Generated code: " << bf.code_size() << " bytes.\n\n";
        std::cout << bf.disassemble() << '\n';
        return 0;
    }

    return static_cast<int>(bf.execute_all());
}
