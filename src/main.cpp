#include <time.h> 
#include <chrono>
#include <fstream>

#include "include/vm.h"
#include "compiler/include/lexer.h"
#include "compiler/include/parser.h"
#include "compiler/include/compiler_main.h"

int main(int argc, char** argv)
{
    ifstream f(argv[1]);

    if (!f.is_open()) {
        cerr << "Cannot run the script" << endl;;
        return 1;
    }

    string s;
    string code;

    while (getline(f, s))
    {
        code += s + " \n";
    }

    f.close();

    Lexer lexer(code, false);
    Parser parser(lexer.make_tokens());
    BlockNode* ast = parser.make_ast(false);

    CompilerMain compiler;
    FemiraVirtualMachine vm;

    compiler.node_to_bytecode(ast);
    vector<Instruction> bytecode = compiler.get_generated_bytecode();

    string show_bytecode = argc > 2 ? argv[2] : "";

    vm.runf_bytecode(bytecode, show_bytecode == "yes" ? true : false);

    return 0;
}