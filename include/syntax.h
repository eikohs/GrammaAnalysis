# pragma once

#include "grammar.h"

using namespace Grammar;

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
    class SyntaxLL{
    private:
        GrammarCla gram; // 文法相关类
        std::unordered_map<Symbol, SymbolSet> First;
        std::unordered_map<Symbol, SymbolSet> Follow;
    private:
        /*构造First集合*/
        void ConstructFirst();
        /*构造Follow集合*/
        void ConstructFollow();
    public:
        /*重载<<运算符，方便输出First、Follow集合及LL(1)分析表*/
        friend std::ostream &operator<<(std::ostream & os, std::unordered_map<Symbol, SymbolSet> & set);

        /*构造LL(1)分析表*/
        void ConstructLL();
    };

    // TODO 完成lr文法调用程序的类，构造识别活前缀的DFA与LR分析表并使用分析表预测分析程序
};