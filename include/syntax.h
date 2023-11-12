# pragma once

#include "grammar.h"

using namespace Grammar;

namespace Syntax{
    /*枚举三种语法分析实现方式*/
    enum {LLPRO, LRPRO};
    extern char OPTION[2][4]; //可能出现的操作名
    extern std::string STACKBOTTOM; // 栈底符号

    /*定义符号类型*/
    using FTable = std::unordered_map<Symbol, SymbolSet>;
    using LLLine = std::unordered_map<Symbol, std::list<ProductionRight>>;
    using LLTable = std::unordered_map<Symbol, LLLine>;

    /*获取要调用的程序类型
        返回int型，说明欲调用的程序类型，共有三种
        若无匹配项则返回-1*/
    int getProgramType(char * argv_1);
    /*输出程序使用帮助*/
    void usageOfProgram(char * argv_0);

    class SyntaxLL{
    private:
        GrammarCla gram; // 文法相关类
        FTable First; // First表
        FTable Follow; // Follow表
        LLTable LL1; // LL(1)分析表

    private:
        /*添加至First、Follow集合中并判断是否成功*/
        static void AddToFSet(SymbolSet &Set, const Symbol &sym, bool &Changed);
        /*构造First集合*/
        void ConstructFirst();
        /*构造Follow集合*/
        void ConstructFollow();
        /*构造LL(1)分析表*/
        void ConstructLL();
    public:
        /*重载<<运算符，方便输出First、Follow集合及LL(1)分析表*/
        friend std::ostream &operator<<(std::ostream & os, FTable & set);
        friend std::ostream &operator<<(std::ostream & os, LLTable & ll1);

        /*启动LL(1)分析程序*/
        void StartLL1Analyse();
    };

    // TODO 完成lr文法调用程序的类，构造识别活前缀的DFA与LR分析表并使用分析表预测分析程序
};