# pragma once

namespace Syntax{
    /*枚举三种语法分析实现方式*/
    enum {LLPRO, LRPRO};
    extern char OPTION[2][4]; //可能出现的操作名

    /*获取要调用的程序类型
        返回int型，说明欲调用的程序类型，共有三种
        若无匹配项则返回-1*/
    int getProgramType(char * argv_1);
    /*输出程序使用帮助*/
    void usageOfProgram(char * argv_0);

    // TODO 完成ll文法调用程序的类,为给定文法自动构造预测分析表并使用分析表预测分析程序

    // TODO 完成lr文法调用程序的类，构造识别活前缀的DFA与LR分析表并使用分析表预测分析程序
};