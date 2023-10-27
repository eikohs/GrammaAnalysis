# pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <vector>

/*定义Debug操作*/
#define DEBUG_SWITCH 1 //0为关，1为开
#define debug_Out(Opt, Msg)\
    if(DEBUG_SWITCH){\
    std::cout << "\nDebug:[File:" << __FILE__ << "|Line:" << __LINE__ <<"]" << std::endl;\
    std::cout << "Level: " << Opt << std::endl;\
    std::cout << "Message: "<< Msg << std::endl;}

namespace Grammar{

    using Symbol = std::string;
    using SymbolSet = std::unordered_set<Symbol>;
    using ProductionRight = std::deque<std::string>; // deque: 双端队列，与vector的用法无大区别但头插的效率更高
    using ProductionSet = std::unordered_map<Symbol, std::vector<ProductionRight>>;

    extern std::string EMPTY; //用特殊字符串代指空

    class Grammar{
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
        friend std::ostream &operator<<(std::ostream& os, const SymbolSet& symbolSet);

        /*获取文法*/
        void LoadGrammar();


        // TODO 完成一个文法产生式的类，负责对文法产生式进行处理(包括消除左递归、多重产生式)
    };
};