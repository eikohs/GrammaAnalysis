#include "../include/gramma.h"
#include <iostream>

using namespace Syntax;

int main(int argc, char * argv[]){
    if(argc != 2){
        usageOfProg(argv[0]);
    }
    else{
        int progType;
        if((progType = getPrgramType(argv[1])) == -1){
            usageOfProg(argv[0]);
        }
        switch(progType){
            case RECURSIVE:{
                // 递归调用程序
                Debug_Out("Normal", "递归调用程序")
            }break;
            case LLPRO:{
                // LL(1)语法分析程序
                Debug_Out("Normal", "LL语法分析")
            }break;
            case LRPRO:{
                // LR(1)语法分析程序
                Debug_Out("Normal", "LR语法分析")
            }break;
            default: break;
        }
    }
}