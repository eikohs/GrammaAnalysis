#include "../include/syntax.h"
#include <cstring>
#include <iostream>
#include <cstdlib>

namespace Syntax{
    char OPTION[2][4] = {"-ll", "-lr"};

    int getProgramType(char * argv_1){
        for(int i = 0;i < 2;i++){
            if(strcmp(argv_1, OPTION[i]) == 0)
                return i;
        }
        return -1;
    }

    void usageOfProgram(char * argv_0){
        std::cout << "Usage: " << argv_0 << " -<option> [GrammarFileName]" << std::endl;
        std::cout << "-<option>:" << std::endl;
        std::cout << "\t-ll: 使用LL(1)语法分析程序" << std::endl;
        std::cout << "\t-lr: 使用LR(1)语法分析程序" << std::endl;
        std::cout << "[GrammarFileName]" << std::endl;
        std::cout << "\t存储文法的文件，按照本项目的标准格式写入以正确的读取" << std::endl;
        exit(EXIT_SUCCESS);
    }

    std::ostream &operator<<(std::ostream &os, std::unordered_map<Symbol, SymbolSet> &set) {
        std::string tmpStr;
        for(auto & it : set){
            tmpStr = it.first;
            tmpStr.resize(25, ' ');
            os << tmpStr;
            for(auto & iter : it.second){
                os << iter << "  ";
            }
            os << std::endl;
        }
        return os;
    }

    void SyntaxLL::ConstructFirst() {
        bool Changed;
        do{
            Changed = false;
            for(auto & it : gram.GetProductions()){
                Symbol left = it.first;
                if(First.find(left) == First.end()){
                    ///<没出现过的非终结符，建立pair对
                    First[left] = {};
                }
                for(auto & iter : it.second){
                    if(gram.GetNonTerminal().find(iter[0]) == gram.GetNonTerminal().end()){
                        ///<产生式第一个符号是终结符，加入First集合中
                        if(First[left].find(iter[0]) == First[left].end()){
                            First[left].insert(iter[0]);
                            Changed = true;
                        }
                    }
                    else{
                        ///<产生式第一个符号是非终结符，调用其First集合
                        for(auto & iter2 : First[iter[0]]){
                            if(First[left].find(iter2) == First[left].end()){
                                First[left].insert(iter2);
                                Changed = true;
                            }
                        }
                    }
                }
            }
        }while(Changed);
        // std::cout << First;
    }

    void SyntaxLL::ConstructFollow() {
        bool Changed;
        do{
            Changed = false;
            for(auto & it : gram.GetProductions()){
                Symbol left = it.first;
                if(First.find(left) == First.end()){
                    ///<没出现过的非终结符，建立pair对
                    First[left] = {};
                }
                for(auto & iter : it.second){

                }
            }
        }while(Changed);
    }

    void SyntaxLL::ConstructLL() {
        /*第一步：处理文法*/
        gram.LoadGrammar();
        std::cout << "成功导入文法，展示如下" << std::endl << gram;
        gram.EliminateLeftRecursion();
        std::cout << "成功消除左递归，展示如下" << std::endl << gram;
        gram.EliminateMultipleProd();
        std::cout << "成功消除多重产生式，展示如下" << std::endl << gram;
        /*第二步：生成LL(1)分析表*/
        ConstructFirst();
        ConstructFollow();
    }

}