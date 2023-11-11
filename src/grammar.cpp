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
        std::cout << "[NonTerminal]:\t" << gra.NonTerminal << std::endl;
        std::cout << "[Terminal]:\t" << gra.Terminal << std::endl;
        std::cout << "[Productions]" << gra.Productions << std::endl;
        std::cout << "[StartSymbol]:\t" << gra.StartSymbol << std::endl;
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
                        else{
                            ///<一个未被定义的文法符号
                            std::string outstr = "一个未被定义的文法符号: " + *it;
                            debug_Out("Error", outstr)
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
    }

    std::vector<std::string> GrammarCla::StringSplit(std::string &str, const char split) {
        std::vector<std::string> RetVal;
        std::istringstream iss(str); // 输入流
        std::string token; //接受缓冲区
        while(std::getline(iss, token, split)){
            RetVal.push_back(token);
        }
        return RetVal;
    }
}

