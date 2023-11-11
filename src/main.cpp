#include "../include/syntax.h"
#include <iostream>

using namespace Syntax;

int main(int argc, char * argv[]){
    if(argc != 3){
        usageOfProgram(argv[0]);
    }
    else{
        int programType;
        if((programType = getProgramType(argv[1])) == -1){
            usageOfProgram(argv[0]);
        }

        Grammar::GrammarFile.open(argv[2], std::ios::in);///<打开第三个参数对应的文件
        if(!Grammar::GrammarFile){
            perror("Open GrammarFile Error");
            exit(EXIT_FAILURE);
        }

        switch(programType){
            case LLPRO:{
                // LL(1)语法分析程序
                debug_Out("Normal", "LL语法分析")
                SyntaxLL synLL;
                synLL.ConstructLL();
            }break;
            case LRPRO:{
                // LR(1)语法分析程序
                debug_Out("Normal", "LR语法分析")
            }break;
            default: break;
        }
    }
}
