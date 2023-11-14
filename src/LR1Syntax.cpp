#include "../include/syntax.h"

#include <iostream>
#include <cstdlib>

namespace Syntax{
    extern std::ostream &operator<<(std::ostream& os, const SymbolSet &symbolSet);
    extern std::ostream &operator<<(std::ostream &os, const ProductionRight &prodr);
    extern std::ostream &operator<<(std::ostream &os, FTable &set);
    extern std::ostream &operator<<(std::ostream &os, LR1ProdFront& LProdF);
    extern std::ostream &operator<<(std::ostream &os, LR1GoTo& lr1gt);
    extern std::ostream &operator<<(std::ostream &os, CLOSURE& clo);
    extern std::ostream &operator<<(std::ostream &os, LRTable& lrt);
    extern std::ostream &operator<<(std::ostream &os, LR1AnaStack& sta);

    void SyntaxLR::AddToFSet(SymbolSet &Set, const Symbol &sym, bool &Changed) {
        if(Set.find(sym) == Set.end()){
            // 没有该符号，插入并成功返回
            Set.insert(sym);
            if(!Changed){
                Changed = true;
            }
        }
    }

    void SyntaxLR::GetFirstSet(SymbolSet &set, const Symbol &sym) {
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

    void SyntaxLR::ConstructFirst() {
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
    }

    void SyntaxLR::InsertProdSet(bool &Modified, LR1ProdSet &SrcSet, LR1ProdFront &prod) {
        for(auto & SrcProd : SrcSet){
            if(SrcProd.first == prod.first){
                ///<找到相同的产生式，比较向前看符号
                for(auto & sym : prod.second){
                    if(SrcProd.second.find(sym) == SrcProd.second.end()){
                        ///<添加一个向前看符号
                        SrcProd.second.insert(sym);
                        Modified = true;
                    }
                }
                return ;
            }
        }
        ///<没找到相同的产生式，添加该产生式
        SrcSet.push_back(prod);
        Modified = true;
    }

    int SyntaxLR::ConstructClosure(bool &Changed, LR1ProdSet &StartSet) {
        int Seq = 0;
        /*查找是否已经有这样的闭包*/
        while(Seq < StartProd.size())
            if(StartProd[Seq] == StartSet)
                return Seq; ///< 有这样的闭包，返回状态号
            else
                Seq++;
        /*没有这样的闭包，建立一个*/
        Changed = true;
        DFA.push_back({StartSet, {}});
        StartProd.push_back(StartSet);
        /*建立整个有效项目集*/
        bool Modified;
        do{
            Modified = false;
            for(auto & iter : DFA[Seq].first){
                auto dotIter = iter.first.second.begin();
                while(dotIter != iter.first.second.end() && *dotIter != DOTFLAG)
                    dotIter++;
                if(++dotIter != iter.first.second.end() && gram.GetNonTerminal().find(*dotIter) != gram.GetNonTerminal().end()){
                    ///< 点后的符号为非终结符，尝试插入新闭包
                    ///< 首先获取first
                    Symbol nonT = *dotIter;
                    SymbolSet tmpS;

                    if(++dotIter != iter.first.second.end()){
                        GetFirstSet(tmpS, *(dotIter));
                    }
                    else{
                        tmpS = iter.second;
                    }
                    ///< 然后尝试插入该集
                    for(auto & prodR : gram.GetProductions().at(nonT)){
                        ProductionRight tmpProdR = prodR;
                        tmpProdR.push_front(DOTFLAG);
                        LR1ProdFront tmpProdF = {{nonT, tmpProdR}, tmpS};
                        InsertProdSet(Modified, DFA[Seq].first, tmpProdF);
                    }
                }
            }
        } while (Modified);
        return Seq;
    }

    void SyntaxLR::ConstructGoTo(int Seq) {
        std::unordered_map<Symbol, LR1ProdSet> ConvertStore;
        for(auto & prodF : DFA[Seq].first){
            ///<确认点的位置
            auto dotIter = prodF.first.second.begin();
            while(dotIter != prodF.first.second.end() && *dotIter != DOTFLAG)
                dotIter++;
            if((++dotIter) != prodF.first.second.end()){
                ///<点后有符号，存储起来
                *(dotIter-1) = *dotIter;
                *dotIter = DOTFLAG;
                ConvertStore[*(dotIter-1)].push_back(prodF);
                //LOGFILE << prodF << std::endl;
                *dotIter = *(dotIter-1);
                *(dotIter-1) = DOTFLAG;
            }
            else{
                ///<点后无符号，构建Accept函数
                Acc[Seq].push_back(prodF);
            }
        }
        ///<根据存储的转换条件构造GOTO函数
        for(auto & pair : ConvertStore){
            bool Changed = false;
            int tmp = ConstructClosure(Changed, pair.second);
            //LOGFILE << "Seq: " << Seq << pair.first << std::endl << DFA[Seq];
            DFA[Seq].second[pair.first] = tmp;
        }
    }

    void SyntaxLR::ConstructDFA() {
        /*初始闭包生成*/
        bool Changed;
        Symbol Start = gram.GetStartSymbol();
        ProductionRight SProdR = gram.GetProductions().at(Start).front();
        SProdR.push_front(DOTFLAG);
        LR1ProdSet StrSet = {{{Start, SProdR}, {STACKBOTTOM}}};
        ConstructClosure(Changed, StrSet);
        /*构造其余项目*/
        do{
            Changed = false;
            for(int i = 0;i < DFA.size();i++){
                if(DFA[i].second.empty() && Acc.find(i) == Acc.end()){
                    ///< 没有构建GOTO和Accept函数，开始构建
                    ConstructGoTo(i);
                    Changed = true;
                    // 重新开始循环
                    break;
                }
            }
        } while (Changed);
    }

    void SyntaxLR::ConstructLR1() {
        LOGFILE.open("LR1Log.txt", std::ios::out);
        if(!LOGFILE){
            perror("Open GrammarFile Error");
            exit(EXIT_FAILURE);
        }
        /*第一步：处理文法*/
        gram.LoadGrammar();
        LOGFILE << "成功导入文法，展示如下" << std::endl << gram;
        gram.ExpandGrammar();
        LOGFILE << "成功拓广了文法，展示如下" << std::endl << gram;
        std::string tmp;
        ConstructFirst();
        tmp = "NonTerminal";tmp.resize(25, ' ');
        LOGFILE << "First集合构建完成，展示如下" << std::endl << tmp << "First集合" << std::endl << First << std::endl;
        /*第二步：构建识别文法有效项目集及活前缀的DFA*/
        ConstructDFA();
        LOGFILE << "成功构建识别文法有效项目集及活前缀的DFA，展示如下: " << std::endl;
        /*第三步：建立LR(1)分析表*/
        for(int i = 0;i < DFA.size();i++){
            ///< 在输出的同时构建分析表
            LOGFILE << 'I' << i << ":\n";
            LOGFILE << DFA[i];
            LRTLine tmpLine;
            for(auto & pair : DFA[i].second){
                ///< 移进操作
                tmpLine[pair.first] = {pair.second, {}};
            }
            if(Acc.find(i) != Acc.end()){
                LOGFILE << "Accept Function:\n";
                for(auto & prodF : Acc[i]){
                    LOGFILE << '\t' << prodF << std::endl << std::endl;
                    ///< 规约操作
                    for(auto & sym : prodF.second){
                        LR1Production tmpProd = prodF.first;
                        tmpProd.second.pop_back();
                        tmpLine[sym] = {-1, tmpProd};
                    }
                }
            }
            Table.push_back(tmpLine);
        }
        LOGFILE << "成功构建LR(1)分析表，展示如下: " << Table << std::endl;
    }

    void SyntaxLR::StartLR1Analyze() {
        /*构建LR1分析表*/
        ConstructLR1();
        /*进行分析*/
        std::string token;
        std::string SrcStr;
        LR1AnaStack sta;
        Symbol nowAnaTer;
        bool SucFlag;
        do{
            std::cout << "Start Syntax Analyze..." << std::endl;
            std::cout << "Please input string, program will analyze whether it belongs to\nthe language of the grammar(Double Enter To Exit): " << std::endl;
            std::getline(std::cin, token);
            if(token.empty())
                break;
            while(!sta.empty())
                sta.pop_back();
            sta.emplace_back(0, STACKBOTTOM); // 初始状态入栈
            SrcStr = token; // 拷贝字符串，用于输出状态
            token.append(STACKBOTTOM); // 初始状态入栈
            while(!sta.empty()){
                char c = token[0];
                if(c >= '0' && c <= '9'){
                    nowAnaTer = "num"; // 明确num符号含义
                }
                else{
                    nowAnaTer = c;
                }
                int seq = sta.back().first;
                std::cout << sta;
                if(Table[seq].find(nowAnaTer) == Table[seq].end()){
                    ///< 分析表未定义的行为，报错
                    debug_Out("Error", "分析字符串过程中出错")
                    exit(EXIT_FAILURE);
                }
                else{
                    LR1TFunction tmpF = Table[seq][nowAnaTer];
                    if(tmpF.first >= 0){
                        ///< 移进操作
                        sta.emplace_back(tmpF.first, nowAnaTer);
                        token.erase(0, 1);
                        std::cout << "input: " << token << std::endl;
                        std::cout << "output: " << "Shift " << tmpF.first << std::endl << std::endl;
                    }
                    else{
                        ///< 规约操作
                        ProductionRight prod = tmpF.second.second;
                        for(auto symIter = prod.rbegin(); symIter != prod.rend(); symIter++){
                            if(*symIter == sta.back().second){
                                sta.pop_back();
                            }
                            else{
                                ///< 分析表未定义的行为，报错
                                debug_Out("Error", "分析字符串过程中出错")
                                SucFlag = false;
                                break;
                            }
                        }
                        seq = sta.back().first;
                        if(sta.size() == 1 && seq == 0 && tmpF.second.first == gram.GetStartSymbol()){
                            while(!sta.empty()){
                                sta.pop_back();
                            }
                            ///< 规约成功
                            SucFlag = true;
                        }
                        else{
                            seq = Table[seq][tmpF.second.first].first;
                            sta.emplace_back(seq , tmpF.second.first);
                        }
                        std::cout << "input: " << token << std::endl;
                        std::cout << "output: " << "Reduce by " << tmpF.second.first << " -> ";
                        for(auto & sym : tmpF.second.second){
                            std::cout << sym << " ";
                        }
                        if(SucFlag){
                            std::cout << std::endl << "Accept!" << std::endl << std::endl;
                        }
                        else
                            std::cout << std::endl << std::endl;
                    }
                }
            }
            if(SucFlag){
                std::cout << "String: \"" << SrcStr << "\" do belong to the grammar!" << std::endl << std::endl;
            } else{
                std::cout << "Sting: \"" << SrcStr << "\" does not belong to the grammar!" << std::endl << std::endl;
            }
        }while(true);
    }

}