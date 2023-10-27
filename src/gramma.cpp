#include "../include/gramma.h"
#include <cstring>
#include <iostream>
#include <cstdlib>

using namespace Syntax;
using namespace std;

char Syntax::OPTION[3][4] = {"-re", "-ll", "-lr"};

int Syntax::getPrgramType(char * argv_1){
    for(int i = 0;i < 3;i++){
        if(strcmp(argv_1, OPTION[i]) == 0)
            return i;
    }
    return -1;
}

void Syntax::usageOfProg(char * argv_0){
    std::cout << "Usage: " << argv_0 << " -<option>" << std::endl;
        std::cout << "-<option>:" << std::endl;
        std::cout << "\t-re: 使用递归调用程序" << std::endl;
        std::cout << "\t-ll: 使用LL(1)语法分析程序" << std::endl;
        std::cout << "\t-lr: 使用LR(1)语法分析程序" << std::endl;
        exit(EXIT_SUCCESS);
}