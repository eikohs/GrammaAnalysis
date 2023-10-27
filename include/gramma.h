# pragma once

/*定义Debug操作*/
#define DEBUG_SWITCH 1 //0为关，1为开
#define Debug_Out(Opt, Msg)\
    if(DEBUG_SWITCH){\
    std::cout << "\nDebug:[File:" << __FILE__ << "|Line:" << __LINE__ <<"]" << std::endl;\
    std::cout << "Level: " << Opt << std::endl;\
    std::cout << "Message: "<< Msg << std::endl;}

/*枚举三种语法分析实现方式*/
enum {RECURSIVE, LLPRO, LRPRO};

namespace Syntax{
    extern char OPTION[3][4]; //可能出现的操作名
    /*获取要调用的程序类型
        返回int型，说明欲调用的程序类型，共有三种
        若无匹配项返回-1*/
    int getPrgramType(char * argv_1);
    /*输出程序使用帮助*/
    void usageOfProg(char * argv_0);


};