# pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <vector>
#include <list>
#include <fstream>

/*定义Debug操作*/
#define DEBUG_SWITCH 1 //0为关，1为开
#define debug_Out(Opt, Msg)\
    if(DEBUG_SWITCH){\
    std::cout << "\nDebug:[File:" << __FILE__ << "|Line:" << __LINE__ <<"]" << std::endl;\
    std::cout << "Level: " << Opt << std::endl;\
    std::cout << "Message: "<< Msg << std::endl << std::endl;}

namespace Grammar{

    /*符号，通常为一个字符*/
    using Symbol = std::string;
    /*符号集合，一群符号集合在一起，形成非终结符号集与终结符号集*/
    using SymbolSet = std::unordered_set<Symbol>;
    /*产生式右端，用双端队列定义，内容是符号，即某一非终结符号的产生式的一个右端*/
    using ProductionRight = std::deque<Symbol>; // deque: 双端队列，与vector的用法无大区别但头插的效率更高
    /*产生式集合，用无序图定义，键值为符号，某一非终结符号能对应一个或多个产生式右端*/
    using ProductionSet = std::unordered_map<Symbol, std::list<ProductionRight>>;

    extern std::string EMPTY; //用特殊字符串代指空
    extern std::ifstream GrammarFile; //存储文法的文件流

    class GrammarCla{
    private:
        SymbolSet NonTerminal; ///<非终结符号集
        SymbolSet Terminal; ///<终结符号集
        ProductionSet Productions; ///<产生式集合
        Symbol StartSymbol; ///<起始符
    public:
        /*返回私有的变量*/
        const SymbolSet & GetNonTerminal() const
        {return NonTerminal;}
        const SymbolSet & GetTerminal() const
        {return Terminal;}
        const ProductionSet & GetProductions() const
        {return Productions;}
        const Symbol & GetStartSymbol() const
        {return StartSymbol;}

        /*重载<<运算符，简化输出*/
        friend std::ostream &operator<<(std::ostream& os, const SymbolSet& symb);
        friend std::ostream &operator<<(std::ostream& os, const ProductionRight& prodr);
        friend std::ostream &operator<<(std::ostream& os, const ProductionSet& prod);
        friend std::ostream &operator<<(std::ostream& os, const GrammarCla& gra);

        /*实现一个分割字符串的函数*/
        static std::vector<std::string> StringSplit(std::string& str, char split);
        /*获取文法*/
        void LoadGrammar();
        /*消除文法的多重产生式*/
        void EliminateMultipleProd();
        /*消除文法的左递归*/
        void EliminateLeftRecursion();
    };
};