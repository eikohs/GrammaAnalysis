#include "../include/syntax.h"

#include <iostream>
#include <cstdlib>
#include <stack>

namespace Syntax{
    /**/
    extern std::ostream &operator<<(std::ostream& os, const SymbolSet &symbolSet);
    extern std::ostream &operator<<(std::ostream &os, const ProductionRight &prodr);
    extern std::ostream &operator<<(std::ostream &os, FTable &set);
    extern std::ostream &operator<<(std::ostream &os, LLTable &ll1);

    void SyntaxLL::GetFirstSet(SymbolSet &set, const Symbol &sym) {
        if(gram.GetNonTerminal().find(sym) == gram.GetNonTerminal().end()){
            ///<产生式第一个符号是终结符，加入First集合中
            set.insert(sym);
        }
        else{
            ///<产生式第一个符号是非终结符，调用其First集合
            for(auto & iter2 : First[sym]){
                set.insert(iter2);
            }
        }
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
                    SymbolSet Tmp;
                    GetFirstSet(Tmp, iter[0]);
                    for(auto & sym : Tmp){
                        AddToFSet(First[left], sym, Changed);
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
        }
        Follow[gram.GetStartSymbol()].insert(STACKBOTTOM);// 对于起始符，添加栈底符号至FOLLOW集合
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
                                else{
                                    SymbolSet tmp;
                                    GetFirstSet(tmp, *next);
                                    for(auto & sym : tmp){
                                        if(sym == EMPTY){
                                            RecurIn = true;
                                        }
                                        else{
                                            AddToFSet(Follow[*symbIter], sym, Changed);
                                        }
                                    }
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
        LOGFILE.open("LL1Log.txt", std::ios::out);
        if(!LOGFILE){
            perror("Open GrammarFile Error");
            exit(EXIT_FAILURE);
        }
        /*第一步：处理文法*/
        gram.LoadGrammar();
        LOGFILE << "成功导入文法，展示如下" << std::endl << gram;
        gram.EliminateLeftRecursion();
        LOGFILE << "成功消除左递归，展示如下" << std::endl << gram;
        gram.EliminateMultipleProd();
        LOGFILE << "成功消除多重产生式，展示如下" << std::endl << gram;
        /*第二步：生成First集与Follow集*/
        std::string tmp;
        ConstructFirst();
        tmp = "NonTerminal";tmp.resize(25, ' ');
        LOGFILE << "First集合构建完成，展示如下" << std::endl << tmp << "First集合" << std::endl << First << std::endl;
        ConstructFollow();
        LOGFILE << "Follow集合构建完成，展示如下" << std::endl << tmp << "Follow集合" << std::endl << Follow << std::endl;
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
                //Line[STACKBOTTOM] = {};
                ///<该非终结符能推导出空，利用follow集构建该行的内容
                ProductionRight tmpProd = {EMPTY};
                for(auto & symIter : Follow[nonIter]){
                    Line[symIter].push_back(tmpProd);
                }
            }
            LL1[nonIter] = Line;
        }
        LOGFILE << "LL(1)分析表构建完成，展示如下" << std::endl << LL1 << std::endl;
    }

    void SyntaxLL::StartLL1Analyse() {
        ConstructLL();
        std::string token; // 读入字符串的缓冲区
        std::stack<Symbol> analyzeStack; // 分析栈
        Symbol nowAnaTer;

        do{
            std::cout << "Start Syntax Analyze..." << std::endl;
            std::cout << "Please input string, program will analyze whether it belongs to\nthe language of the grammar(Double Enter To Exit): " << std::endl;
            std::getline(std::cin, token);
            if(token.empty())
                break;
            while(!analyzeStack.empty())
                analyzeStack.pop(); // 清空栈
            analyzeStack.push(STACKBOTTOM); // 栈底符号入栈
            token.append(STACKBOTTOM); // 串尾符号入串
            analyzeStack.push(gram.GetStartSymbol()); // 文法起始符入栈
            for(auto & c : token){
                if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')){
                    nowAnaTer = "id"; //明确id非终结符的含义
                }
                else if(c >= '0' && c <= '9'){
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
                        goto LL1FALIURE; // LINE255 为了跳出多重循环
                    }
                    if(LL1[analyzeStack.top()].find(nowAnaTer) == LL1[analyzeStack.top()].end()){
                        ///< 分析过程出错
                        debug_Out("Error", "分析字符串过程中出错")
                        //exit(EXIT_FAILURE);
                        goto LL1FALIURE; // LINE255 为了跳出多重循环
                    }
                    else{
                        ProductionRight prodR = *(LL1[analyzeStack.top()][nowAnaTer].begin());
                        std::cout << "OutPut:\t" << analyzeStack.top() << " -> " << prodR << std::endl;
                        analyzeStack.pop();
                        for(auto iter = prodR.rbegin();iter != prodR.rend();iter++){
                            if(*iter != EMPTY){
                                analyzeStack.push(*iter);
                            }
                        }
                        if(analyzeStack.top() == nowAnaTer){
                            std::cout << "Analyzed Symbol: " << c << std::endl;
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
                std::cout << "Wrong StackTop Symbol: " << analyzeStack.top() << std::endl;
                std::cout << "Or Wrong Analyzing Symbol: " << nowAnaTer << std::endl;
                std::cout << "Sting: \"" << token << "\" does not belong to the grammar!" << std::endl << std::endl;
            }
            else{
                ///< 分析成功
                std::cout << "String: \"" << token << "\" do belong to the grammar!" << std::endl << std::endl;
            }
        }while(true);
    }
}