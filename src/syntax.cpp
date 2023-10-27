#include "../include/syntax.h"
#include "../include/grammar.h"
#include <cstring>
#include <iostream>
#include <cstdlib>

using namespace Syntax;
using namespace std;

char Syntax::OPTION[2][4] = {"-ll", "-lr"};

int Syntax::getProgramType(char * argv_1){
    for(int i = 0;i < 2;i++){
        if(strcmp(argv_1, OPTION[i]) == 0)
            return i;
    }
    return -1;
}

void Syntax::usageOfProgram(char * argv_0){
    std::cout << "Usage: " << argv_0 << " -<option>" << std::endl;
    std::cout << "-<option>:" << std::endl;
        std::cout << "\t-ll: 使用LL(1)语法分析程序" << std::endl;
        std::cout << "\t-lr: 使用LR(1)语法分析程序" << std::endl;
    exit(EXIT_SUCCESS);
}