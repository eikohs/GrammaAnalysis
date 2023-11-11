#include "grammar.h"
#include <iostream>
#include <sstream>

namespace Grammar{
    std::string EMPTY = "[EPSILON]";
    std::ifstream GrammarFile;

    /*下面重载4个<<运算符，以完成对文法的输出*/
    std::ostream &operator<<(std::ostream& os, const SymbolSet &symbolSet) {
        auto it = symbolSet.begin();
        os << *(it++);
        while(it != symbolSet.end()){
            os << "  " << *(it++);
        }
        return os;
    }
    std::ostream &operator<<(std::ostream &os, const ProductionRight &prodr) {
        auto it = prodr.begin();
        os << *(it++);
        while(it != prodr.end()){
            os << ' ' << *(it++);
        }
        return os;
    }
    std::ostream &operator<<(std::ostream &os, const ProductionSet &prod) {
        for(const auto & it : prod){
            os << "\n\t" << it.first << " -> ";
            auto rit = it.second.begin();
            os << *(rit++);
            while(rit != it.second.end()){
                os << " | " << *(rit++);
            }
        }
        return os;
    }
    std::ostream &operator<<(std::ostream &os, const GrammarCla &gra) {
        std::cout << "[NonTerminal]:\n\t" << gra.NonTerminal << std::endl;
        std::cout << "[Terminal]:\n\t" << gra.Terminal << std::endl;
        std::cout << "[Productions]" << gra.Productions << std::endl;
        std::cout << "[StartSymbol]:\n\t" << gra.StartSymbol << std::endl;
        return os;
    }

    void GrammarCla::LoadGrammar() {
        std::string token;
        std::vector<std::string> vec_token;
        std::getline(GrammarFile, token);
        if(token == "#NONTERMINAL"){
            ///<开始读入非终结符
            std::getline(GrammarFile, token);
            vec_token = StringSplit(token, ' ');
            for(auto & it : vec_token){
                NonTerminal.insert(it);
            }
        }
        //std::cout << NonTerminal;
        std::getline(GrammarFile, token);
        if(token == "#TERMINAL"){
            ///<开始读入终结符
            std::getline(GrammarFile, token);
            vec_token = StringSplit(token, ' ');
            for(auto & it : vec_token){
                Terminal.insert(it);
            }
        }
        //std::cout << Terminal;
        std::getline(GrammarFile, token);
        if(token == "#PRODUCTIONS"){
            ///<开始读入产生式
            while((std::getline(GrammarFile, token)) && token != "#STARTSYMBOL"){
                vec_token = StringSplit(token, ' ');
                auto it = vec_token.begin();
                if(NonTerminal.find(*it) == NonTerminal.end()){
                    ///<产生式左边的符号不是非终结符，报错
                    debug_Out("Error", "产生式左边的符号不是非终结符")
                    exit(EXIT_FAILURE);
                }
                Symbol left = *(it++);
                if(Productions.find(left) == Productions.end()){
                    ///<产生式集合中没有以该非终结符为起始符的产生式
                    Productions[left] = {}; // 为该键值建立对应的值
                }
                if(*it == "->")
                    it++;
                ProductionRight proRight;
                while(it != vec_token.end()){
                    if(NonTerminal.find(*it) == NonTerminal.end() && Terminal.find(*it) == Terminal.end()){
                        if(*it == "|"){
                            ///<产生式中的分隔符
                            Productions[left].push_back(proRight);
                            proRight.clear();
                            it++;
                            continue;
                        }
                        else if (*it == EMPTY){
                            ///<产生式推导出空串，不做处理
                        }
                        else{
                            ///<一个未被定义的文法符号
                            debug_Out("Error", std::string("一个未被定义的文法符号: " + *it))
                            exit(EXIT_FAILURE);
                        }
                    }
                    proRight.push_back(*(it++));
                }
                Productions[left].push_back(proRight);
            }
        }
        ///<最后读入起始符
        std::getline(GrammarFile, token);
        vec_token = StringSplit(token, ' ');
        StartSymbol = *vec_token.begin();

        //std::cout << *this;
    }

    std::vector<std::string> GrammarCla::StringSplit(std::string &str, const char split) {
        std::vector<std::string> RetVal;
        std::istringstream iss(str); // 输入流
        std::string token; // 接受缓冲区
        while(std::getline(iss, token, split)){
            RetVal.push_back(token);
        }
        return RetVal;
    }

    void GrammarCla::EliminateMultipleProd() {
        ProductionRight PublicFront; // 记录公共前缀
        std::vector<std::list<ProductionRight>::iterator> IterSet; // 存储有公共前缀的产生式右侧
        for(auto Production = Productions.begin();Production != Productions.end();Production++){
            ///<对于每个非终结符号的产生式，在时间复杂度为O(n^2)的情况下两两比较每个产生式右部，确认是否有公共前缀
            for(auto iter1 = Production->second.begin();iter1 != Production->second.end();iter1++){
                for(auto iter2 = iter1; iter2 != Production->second.end();iter2++){
                    if((*iter1)[0] != (*iter2)[0] || iter1 == iter2){
                        ///<没有公共的前缀，不做处理，跳过
                        continue;
                    }
                    PublicFront.clear();
                    IterSet.clear();
                    ///<生成新的非终结符号
                    Symbol newSymbol = Production->first + "'";
                    while(NonTerminal.find(newSymbol) != NonTerminal.end()){
                        newSymbol.append("'");
                    }
                    // 添加新的非终结符号到产生式左侧及非终结符号集中
                    NonTerminal.insert(newSymbol);
                    Productions[newSymbol] = {};
                    debug_Out("Normal", std::string("消除多重产生式，生成新的非终结符号: " + newSymbol))
                    ///<继续遍历产生式，找到有相同公共前缀的产生式右侧，加入集合一起处理
                    auto newIter = iter2;
                    IterSet.push_back(iter1);
                    IterSet.push_back(iter2);
                    while(++newIter != Production->second.end()){
                        if((*newIter)[0] == (*iter1)[0]){
                            IterSet.push_back(newIter);
                        }
                    }
                    ///<找到最长的公共前缀
                    bool FlagMaxPub = true;
                    do{
                        PublicFront.push_back((**IterSet.begin())[0]);
                        for(auto & i : IterSet){
                            i->pop_front();
                        }
                        if(!(**IterSet.begin()).empty()){
                            Symbol tmp = (**IterSet.begin())[0];
                            for(auto & i : IterSet){
                                if((*i).empty() || (*i)[0] != tmp){
                                    FlagMaxPub = false;
                                    break;
                                }
                            }
                        }
                        else{
                            FlagMaxPub = false;
                        }
                    }while(FlagMaxPub);
                    ///<完成修改工作：删除有公共前缀的产生式右侧，替换为一个新的产生式右侧
                    for(auto & i : IterSet){
                        Productions[newSymbol].push_back(*i);
                        Production->second.erase(i);
                    }
                    PublicFront.push_back(newSymbol);
                    Production->second.push_back(PublicFront);
                    ///<添加了新产生式，从头开始查询
                    Production = Productions.begin();
                    iter1 = Production->second.begin();
                    iter1--;
                    break;
                }
            }
        }
        //std::cout << *this;
    }

    void GrammarCla::EliminateLeftRecursion() {
        for(auto Production = Productions.begin();Production != Productions.end();Production++){
            for(auto iter = Production->second.begin(); iter != Production->second.end(); iter++){
                if((*iter)[0] == Production->first){
                    ///<开始消除左递归
                    ///<生成新的非终结符号
                    Symbol newSymbol = Production->first + "'";
                    while(NonTerminal.find(newSymbol) != NonTerminal.end()){
                        newSymbol.append("'");
                    }
                    // 添加新的非终结符号到产生式左侧及非终结符号集中
                    NonTerminal.insert(newSymbol);
                    Productions[newSymbol] = {};
                    debug_Out("Normal", std::string("消除左递归，生成新的非终结符号: " + newSymbol))
                    ///<继续查询接下来的产生式有无左递归,生成新的产生式并删除原来的含递归产生式
                    ProductionRight newProdR;
                    while(iter != Production->second.end()){
                        if((*iter)[0] == Production->first){
                            iter->pop_front();
                            newProdR = *iter;
                            newProdR.push_back(newSymbol);
                            Productions[newSymbol].push_back(newProdR);
                            iter = Production->second.erase(iter);
                        }
                        else{
                            iter++;
                        }
                    }
                    newProdR.clear();newProdR.push_back(EMPTY);
                    Productions[newSymbol].push_back(newProdR);
                    ///<修改原来的产生式
                    for(auto & tmp : Production->second){
                        tmp.push_back(newSymbol);
                    }
                    ///<修改了产生式，重新开始循环
                    Production = Productions.begin();
                    iter = Production->second.begin();
                    iter--;
                    continue;
                }
            }
        }
        //std::cout << *this;
    }
}

