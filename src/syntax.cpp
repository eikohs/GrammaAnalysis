#include "../include/syntax.h"
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <stack>

namespace Syntax{
    char OPTION[2][4] = {"-ll", "-lr"};
    std::string STACKBOTTOM = "$";
    std::string DOTFLAG = ".";
    std::ofstream LOGFILE;

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
        std::cout << "\t-ll: Use LL(1) Syntax Analyze Programme" << std::endl;
        std::cout << "\t-lr: Use LR(1) Syntax Analyze Programme" << std::endl;
        std::cout << "[GrammarFileName]" << std::endl;
        std::cout << "\tThe file that stores the grammar" << std::endl;
        exit(EXIT_SUCCESS);
    }

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

    std::ostream &operator<<(std::ostream &os, FTable &set) {
        std::string tmpStr;
        for(auto & it : set){
            tmpStr = it.first;
            tmpStr.resize(25, ' ');
            os << tmpStr << it.second << std::endl;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, LLTable &ll1) {
        for(auto & llIter : ll1){
            os << "非终结符为: " << llIter.first << "的分析表行如下" << std::endl;
            for(auto & lineIter : llIter.second){
                os << "\t终结符号:\t" << lineIter.first << std::endl;
                os << "\t推导的产生式:\t";
                auto prodIter = lineIter.second.begin();
                os << llIter.first << " -> " << *(prodIter++);
                while(prodIter != lineIter.second.end()){
                    // 此时说明LL(1)分析表存在冲突
                    os << " | " << *(prodIter++);
                }
                os << std::endl;
            }
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, LR1ProdFront& LProdF) {
        os << LProdF.first.first << " -> ";
        os << LProdF.first.second << "\t";
        auto iter = LProdF.second.begin();
        os << *iter;
        while(++iter != LProdF.second.end()){
            os << " | " << *iter;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, LR1GoTo& lr1gt) {
        if(!lr1gt.empty())
            os << "Goto Function:" << std::endl;
        for(auto & i : lr1gt){
            os << "\tGoTo(" << i.first << ", " << i.second << ")\n";
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, CLOSURE& clo) {
        os << "Set:" << std::endl;
        for(auto & i : clo.first){
            os << '\t' << i << std::endl;
        }
        os << clo.second;
        return os;
    }

    std::ostream &operator<<(std::ostream &os, LRTable& lrt){
        for(int i = 0;i < lrt.size();i++){
            os << "I" << i << ":\n";
            for(auto & opt : lrt[i]){
                os << "\t符号为: " << opt.first << std::endl;
                os << "\t操作: ";
                if(opt.second.first < 0){
                    // 规约
                    os << "规约 " << opt.second.second.first << " -> ";
                    for(auto & sym : opt.second.second.second){
                        os << sym << " ";
                    }
                }
                else{
                    os << "移进到 I" << opt.second.first;
                }
                os << std::endl;
            }
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, LR1AnaStack& sta){
        std::string tmp;
        os << "States ";
        for(auto & i : sta){
            tmp = std::to_string(i.first);tmp.resize(5, ' ');
             os << tmp << " ";
        }
        os << std::endl << "Symbol ";
        for(auto & i : sta){
            tmp = i.second;tmp.resize(5, ' ');
             os << tmp << " ";
        }
        os << std::endl;
        return os;
    }
}