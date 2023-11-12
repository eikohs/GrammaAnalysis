#include "../include/syntax.h"
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <stack>

namespace Syntax{
    char OPTION[2][4] = {"-ll", "-lr"};
    std::string STACKBOTTOM = "$";

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

    void SyntaxLL::ConstructFirst() {
        for(auto & i : gram.GetNonTerminal()){
            ///<没出现过的非终结符，建立pair对
            First[i] = {};
        }
        bool Changed;
        do{
            Changed = false;
            for(auto & it : gram.GetProductions()){
                Symbol left = it.first;
                for(auto & iter : it.second){
                    if(gram.GetNonTerminal().find(iter[0]) == gram.GetNonTerminal().end()){
                        ///<产生式第一个符号是终结符，加入First集合中
                        AddToFSet(First[left], iter[0], Changed);
                    }
                    else{
                        ///<产生式第一个符号是非终结符，调用其First集合
                        for(auto & iter2 : First[iter[0]]){
                            AddToFSet(First[left], iter2, Changed);
                        }
                    }
                }
            }
        }while(Changed);
        //std::cout << First;
    }

    void SyntaxLL::ConstructFollow() {
        for(auto & i : gram.GetNonTerminal()){
            ///<没出现过的非终结符，建立pair对
            Follow[i] = {};
            if(i == gram.GetStartSymbol()){
                Follow[i].insert(STACKBOTTOM); // 对于起始符，添加栈底符号至FOLLOW集合
            }
        }
        bool Changed;
        do{
            Changed = false;
            for(auto & it : gram.GetProductions()){
                Symbol left = it.first;
                ///<遍历每个产生式, 试图拓展Follow集合
                for(auto & dequeIter : it.second){
                    for(auto symbIter = dequeIter.begin(); symbIter != dequeIter.end(); symbIter++){
                        if(gram.GetNonTerminal().find(*symbIter) != gram.GetNonTerminal().end()){
                            auto next = symbIter;
                            bool RecurIn;
                            do{
                                next++;
                                RecurIn = false;
                                if(next == dequeIter.end()){
                                    ///<尝试将产生式左端符号的FOLLOW集合添加到symbIter对应的非终结符号集合中
                                    for(auto & folIter : Follow[left]){
                                        AddToFSet(Follow[*symbIter], folIter, Changed);
                                    }
                                }
                                else if(gram.GetNonTerminal().find(*next) != gram.GetNonTerminal().end()){
                                    ///<后接非终结符，添加其First集合至symbIter对应的非终结符号集合中
                                    for(auto & firIter : First[*next]){
                                        if(firIter != EMPTY){
                                            AddToFSet(Follow[*symbIter], firIter, Changed);
                                        }
                                        else{
                                            ///<后接推导为空的非终结符，再向后看一位
                                            RecurIn = true;
                                        }
                                    }
                                }
                                else{
                                    ///<后接终结符，添加到其Follow集合中
                                    AddToFSet(Follow[*symbIter], *next, Changed);
                                }
                            }while(RecurIn);
                        }
                    }
                }
            }
        }while(Changed);
        //std::cout << Follow;
    }

    void SyntaxLL::ConstructLL() {
        /*第一步：处理文法*/
        gram.LoadGrammar();
        std::cout << "成功导入文法，展示如下" << std::endl << gram;
        gram.EliminateLeftRecursion();
        std::cout << "成功消除左递归，展示如下" << std::endl << gram;
        gram.EliminateMultipleProd();
        std::cout << "成功消除多重产生式，展示如下" << std::endl << gram;
        /*第二步：生成First集与Follow集*/
        std::string tmp;
        ConstructFirst();
        tmp = "NonTerminal";tmp.resize(25, ' ');
        std::cout << "First集合构建完成，展示如下" << std::endl << tmp << "First集合" << std::endl << First << std::endl;
        ConstructFollow();
        std::cout << "Follow集合构建完成，展示如下" << std::endl << tmp << "Follow集合" << std::endl << Follow << std::endl;
        /*第三步：生成LL(1)分析表*/
        // 为每个非终结符建立分析表中的一行, 为每一行的每个终结符建立一个单元
        for(auto & nonIter : gram.GetNonTerminal()){
            LLLine Line;
            // 利用first集构建该行的内容
            for(auto & terIter : gram.GetTerminal()){
                if(First[nonIter].find(terIter) != First[nonIter].end()){
                    ///<该非终结符能推导出该终结符，找到这样的产生式并加入分析表
                    Line[terIter] = {};
                    for(auto & prod : gram.GetProductions().at(nonIter)){
                        if(prod[0] == terIter ||
                            (gram.GetNonTerminal().find(prod[0]) != gram.GetNonTerminal().end() &&
                            First[prod[0]].find(terIter) != First[prod[0]].end())
                            )
                        {
                            Line[terIter].push_back(prod);
                        }
                    }
                }
            }
            if(First[nonIter].find(EMPTY) != First[nonIter].end()){
                Line[STACKBOTTOM] = {};
                ///<该非终结符能推导出空，利用follow集构建该行的内容
                ProductionRight tmpProd = {EMPTY};
                for(auto & symIter : Follow[nonIter]){
                    Line[symIter].push_back(tmpProd);
                }
            }
            LL1[nonIter] = Line;
        }
        std::cout << "LL(1)分析表构建完成，展示如下" << std::endl << LL1 << std::endl;
    }

    void SyntaxLL::AddToFSet(SymbolSet &Set, const Symbol &sym, bool &Changed) {
        if(Set.find(sym) == Set.end()){
            // 没有该符号，插入并成功返回
            Set.insert(sym);
            if(!Changed){
                Changed = true;
            }
        }
    }

    void SyntaxLL::StartLL1Analyse() {
        ConstructLL();
        std::cout << "开始进行语法分析..." << std::endl;
        std::cout << "请输入一串字符，程序将分析它是否属于该文法的语言(输入空字符串退出程序): " << std::endl;
        std::string token; // 读入字符串的缓冲区
        std::stack<Symbol> analyzeStack; // 分析栈
        while((std::getline(std::cin, token)) && !token.empty()){
            Symbol nowAnaTer;
            while(!analyzeStack.empty())
                analyzeStack.pop(); // 清空栈
            analyzeStack.push(STACKBOTTOM); // 栈底符号入栈
            token.append(STACKBOTTOM); // 串尾符号入串
            analyzeStack.push(gram.GetStartSymbol()); // 文法起始符入栈
            for(auto & c : token){
                if(c >= '0' && c <= '9'){
                    nowAnaTer = "num"; // 明确num非终结符的含义
                }
                else{
                    nowAnaTer = c;
                }
                bool Analyzed;
                do{
                    Analyzed = false;
                    if(gram.GetNonTerminal().find(analyzeStack.top()) == gram.GetNonTerminal().end()){
                        ///< 分析过程出错
                        debug_Out("Error", "分析字符串过程中出错")
                        //exit(EXIT_FAILURE);
                        goto LL1FALIURE;
                    }
                    if(LL1[analyzeStack.top()].find(nowAnaTer) == LL1[analyzeStack.top()].end()){
                        ///< 分析过程出错
                        debug_Out("Error", "分析字符串过程中出错")
                        //exit(EXIT_FAILURE);
                        goto LL1FALIURE;
                    }
                    else{
                        ProductionRight prodR = *(LL1[analyzeStack.top()][nowAnaTer].begin());
                        std::cout << "输出:\t" << analyzeStack.top() << " -> " << prodR << std::endl;
                        analyzeStack.pop();
                        for(auto iter = prodR.rbegin();iter != prodR.rend();iter++){
                            if(*iter != EMPTY){
                                analyzeStack.push(*iter);
                            }
                        }
                        if(analyzeStack.top() == nowAnaTer){
                            std::cout << "分析出终结符:\t" << c << std::endl;
                            analyzeStack.pop();
                            Analyzed = true;
                        }
                    }
                }while(!Analyzed);
            }
LL1FALIURE:
            token.pop_back();
            if(!analyzeStack.empty()){
                ///< 分析失败
                std::cout << "错误的栈顶符号: " << analyzeStack.top() << std::endl;
                std::cout << "字符串: \"" << token << "\" 不属于该文法" << std::endl;
            }
            else{
                ///< 分析成功
                std::cout << "字符串: \"" << token << "\" 属于该文法" << std::endl;
            }
        }
    }
}