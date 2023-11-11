#include "../include/syntax.h"
#include "../include/grammar.h"
#include <iostream>

using namespace Syntax;
using namespace Grammar;

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
        // 测试文法类工作情况
        Grammar::GrammarCla gram;
        gram.LoadGrammar();
        gram.EliminateMultipleProd();
        gram.EliminateLeftRecursion();

        switch(programType){
            case LLPRO:{
                // LL(1)语法分析程序
                debug_Out("Normal", "LL语法分析")
            }break;
            case LRPRO:{
                // LR(1)语法分析程序
                debug_Out("Normal", "LR语法分析")
            }break;
            default: break;
        }
    }
}
