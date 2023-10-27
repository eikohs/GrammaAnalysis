#include "../include/syntax.h"
#include "../include/grammar.h"
#include <iostream>

using namespace Syntax;

int main(int argc, char * argv[]){
    if(argc != 2){
        usageOfProgram(argv[0]);
    }
    else{
        int programType;
        if((programType = getProgramType(argv[1])) == -1){
            usageOfProgram(argv[0]);
        }
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
