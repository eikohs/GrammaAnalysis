# pragma once

#include "grammar.h"

using namespace Grammar;

namespace Syntax{
    /*枚举三种语法分析实现方式*/
    enum {LLPRO, LRPRO};
    extern char OPTION[2][4]; //可能出现的操作名
    extern std::string STACKBOTTOM; // 栈底符号
    extern std::string DOTFLAG; //活前缀标识符
    extern std::ofstream LOGFILE; //日志文件



    /*获取要调用的程序类型
        返回int型，说明欲调用的程序类型，共有三种
        若无匹配项则返回-1*/
    int getProgramType(char * argv_1);
    /*输出程序使用帮助*/
    void usageOfProgram(char * argv_0);

    /*定义LL1文法的符号类型*/
    /*First和Follow表的结构，一个非终结符对应一个符号集*/
    using FTable = std::unordered_map<Symbol, SymbolSet>;
    /*LL分析表的一行结构，一个终结符对应一串产生式*/
    using LLLine = std::unordered_map<Symbol, std::list<ProductionRight>>;
    /*LL分析表的结构，一个非终结符对应一行*/
    using LLTable = std::unordered_map<Symbol, LLLine>;

    /*实现LL(1)文法的方法*/
    class SyntaxLL{
    private:
        GrammarCla gram; // 文法相关类
        FTable First; // First表
        FTable Follow; // Follow表
        LLTable LL1; // LL(1)分析表

    private:
        /*添加至First、Follow集合中并判断是否成功*/
        static void AddToFSet(SymbolSet &Set, const Symbol &sym, bool &Changed);
        /*从依据First表获取一个符号的First集*/
        void GetFirstSet(SymbolSet &set, const Symbol &sym);
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
    /*定义LR1文法用到的符号类型*/
    /*LR1有效项目集的产生式结构，一个非终结符对应的一个产生式右侧*/
    using LR1Production = std::pair<Symbol, ProductionRight>;
    /*LR1有效项目集的向前看结构，一个产生式以及一个向前看符号集*/
    using LR1ProdFront = std::pair<LR1Production, SymbolSet>;
    /*LR1有效项目集的GOTO结构, 一个符号对应一个转移的状态*/
    using LR1GoTo = std::unordered_map<Symbol, int>;
    /*LR1有效项目集*/
    using LR1ProdSet = std::list<LR1ProdFront>;
    /*LR1有效项目集的Accept结构*/
    using LR1Accept = std::unordered_map<int, LR1ProdSet>;
    /*LR1闭包的结构*/
    using CLOSURE = std::pair<LR1ProdSet, LR1GoTo>;
    /*以特征标明LR1闭包的结构*/
    using TOUCH = std::vector<LR1ProdSet>;
    /*LR(1)分析表的操作*/
    using LR1TFunction = std::pair<int, LR1Production>; // 左边为负数时说明进行归约操作
    /*LR(1)分析表的一行*/
    using LRTLine = std::unordered_map<Symbol, LR1TFunction>;
    /*LR(1)分析表的结构*/
    using LRTable = std::vector<LRTLine>;
    /*LR(1)分析栈的结构*/
    using LR1AnaStack = std::vector<std::pair<int, Symbol>>;
    /*实现LR1文法的方法*/
    class SyntaxLR{
    private:
        GrammarCla gram; //文法相关类
        FTable First; //First表
        LR1Accept Acc; // Accept函数
        std::vector<CLOSURE> DFA; //识别文法有效项目集的DFA
        LRTable Table; //LR(1)分析表
        TOUCH StartProd; // 特征识别有效项目集的结构
    private:
        /*添加至First、Follow集合中并判断是否成功*/
        static void AddToFSet(SymbolSet &Set, const Symbol &sym, bool &Changed);
        /*从依据First表获取一个符号的First集*/
        void GetFirstSet(SymbolSet &set, const Symbol &sym);
        /*构造First集合*/
        void ConstructFirst();
        /*尝试忘某个闭包插入一条产生式，成功确定修改后至Modified为true*/
        static void InsertProdSet(bool& Modified, LR1ProdSet& SrcSet, LR1ProdFront& prod);
        /*完成一个闭包的构建，成功确定修改后至Changed为true*/
        int ConstructClosure(bool& Changed, LR1ProdSet& StartSet);
        /*构造一个闭包的GoTo函数与Accept函数*/
        void ConstructGoTo(int Seq);
        /*构建识别有效项目集的DFA*/
        void ConstructDFA();
        /*生成LR1分析表*/
        void ConstructLR1();
    public:
        /*重载<<运算符，方便输出First、Follow集合及LL(1)分析表*/
        friend std::ostream &operator<<(std::ostream & os, FTable & set);

        /*进行LR1分析*/
        void StartLR1Analyze();
    };
};