## <CENTER>实验报告——词法分析程序的设计与实现</CENTER>

<CENTER><FONT FACE="楷体">程萬哲 2021211311班 2021211267</FONT></CENTER>

***

### 一、实验内容与要求
1. 编写语法分析程序，实现对算术表达式的语法分析。要求所分析算数表达式由如下的文法产生。
  - E→ E+T | E–T | T
  - T→ T*F | T/F | F
  - F→ (E) | num
2. 在对输入的算术表达式进行分析的过程中，依次输出所采用的产生式。

### 二、存储文法的数据结构
```c++
    /*符号，通常为一个字符*/
    using Symbol = std::string;
    /*符号集合，一群符号集合在一起，形成非终结符号集与终结符号集*/
    using SymbolSet = std::unordered_set<Symbol>;
    /*产生式右端，用双端队列定义，内容是符号，即某一非终结符号的产生式的一个右端*/
    using ProductionRight = std::deque<Symbol>; // deque: 双端队列，与vector的用法无大区别但头插的效率更高
    /*产生式集合，用无序图定义，键值为符号，某一非终结符号能对应一个或多个产生式右端*/
    using ProductionSet = std::unordered_map<Symbol, std::list<ProductionRight>>;

    class GrammarCla{
    private:
        SymbolSet NonTerminal; ///<非终结符号集
        SymbolSet Terminal; ///<终结符号集
        ProductionSet Productions; ///<产生式集合
        Symbol StartSymbol; ///<起始符
    ...
    };
```
- 用到的变量类型与解释见代码注释部分
#### 能够被读取的文法存储格式
```txt
#NONTERMINAL
E T F
#TERMINAL
num + - * / ( )
#PRODUCTIONS
E -> E + T | E - T | T
T -> T * F | T / F | F
F -> ( E ) | num
#STARTSYMBOL
E
```
- 如上方所示，需要输入文法的四要素：非终结符、终结符、产生式、起始符
- 在输入前输入以‘#’开头的一串字符代表下一行或多行是四要素的某一个
- 每个符号之间用空格分隔，产生式支持‘|’符号
- 程序会使用`void LoadGrammar();`函数进行文法读取，它在grammar.h文件中声明，在grammar.cpp文件中定义，在此不做展示。

### 三、LL(1)语法分析程序的设计说明
#### 数据结构定义
```c++
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
    ...
    }
```
- 如上述代码所示，对每个数据结构的说明请参见注释
#### 消除左递归的算法
```c++
    /*没有消除间接左递归*/
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
    }
```
- 生成新符号的方法：通过在某一符号后方不断添加“'”字符直到它是一个新符号为止
- 对于一个非终结符的多个产生式，如果有左递归，那么我们会通过遍历以及`(*iter)[0] == Production->first`找到第一处左递归发生的位置
  接下来，我们继续遍历并对于每一个符合条件的产生式右侧（包括第一处左递归）进行如下处理：
  - 将它去除第一个符号，并将剩下的产生式右侧加上新符号本身来添加为新符号的产生式右侧
- 遍历完成之后，为该非终结符剩余的每个产生式右侧的末尾添加上新符号
- 并让新符号推导出空
- 如上操作后，消除了一个非终结符产生的左递归，一直进行如上操作直到没有修改出现，就是我们的消除左递归算法
#### 消除多重产生式的算法
```cpp
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
    }
```
- 同样的，我们通过遍历以及`(*iter1)[0] != (*iter2)[0] || iter1 == iter2`来找出发生多重产生式的最初两个产生式右侧并加入集合中
- 之后以同样的方法生成新符号
- 然后继续遍历剩余的产生式右侧，将每个有相同的首符号的产生式右侧加入到集合中
- 然后找出最长的公共前缀，将每个集合中的产生式右侧删除，将他们出公共前缀的部分添加为新符号推导出的产生式并将一个公共前缀+新符号添加为
原非终结符的产生式
- 重复上述操作直到没有修改发生
#### 生成First集合的算法
```cpp
    /*添加至First、Follow集合中并判断是否成功*/
    static void AddToFSet(SymbolSet &Set, const Symbol &sym, bool &Changed);
    /*从依据First表获取一个符号的First集*/
    void GetFirstSet(SymbolSet &set, const Symbol &sym);

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
    }
```
- 即：不断遍历文法的产生式，求出非终结符生成的产生式右侧第一个符号的First集合，并添加到该非终结符的First集合中
- 对于右侧第一个符号为非终结符的情况，调用它的First集合即可
- 重复执行上述操作直到没有修改发生
#### 生成Follow集合的算法
```cpp
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
    }
```
- 不断遍历文法的产生式右侧，出现一个非终结符时，查看它的后继符号
  - 如果没有或后继符号的First集合有空，将推导出该产生式右侧的非终结符的Follow集合添加到该非终结符的Follow集合中
  - 否则求出后继符号的First集合，添加到该非终结符的Follow集合中
- 重复执行上述操作直到没有修改发生
#### 生成LL(1)分析表的函数
```CPP
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
```
- 依据First集合以及Follow集合构造LL(1)分析表
- 自行根据注释与代码进行理解
#### 进行LL(1)分析的函数
```CPP
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
```
### 四、LR(1)语法分析程序的设计说明
#### 数据结构定义
```CPP
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
    ...
    }
```
- 数据结构的说明见注释
#### 拓展文法的算法
```CPP
    void GrammarCla::ExpandGrammar() {
        ///<生成一个新的起始符
        Symbol newStart = StartSymbol + "'";
        while(NonTerminal.find(newStart) != NonTerminal.end()){
            newStart.append("'");
        }
        NonTerminal.insert(newStart);
        ///<修改产生式，拓广整个文法
        ProductionRight prodR = {StartSymbol};
        Productions[newStart] = {prodR};
        StartSymbol = newStart;
        //std::cout << *this;
    }
```
- 该算法较简单，跳过不分析
#### 构造有效项目集闭包的算法
```CPP
    /*尝试忘某个闭包插入一条产生式，成功确定修改后至Modified为true*/
    static void InsertProdSet(bool& Modified, LR1ProdSet& SrcSet, LR1ProdFront& prod);
    
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
```
- 依照教材上所给的算法，构造某一状态的有效项目集闭包
#### 构造节点GOTO函数以及ACTION函数的算法
```CPP
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
```
- 依照教材上所给的算法，构造GOTO函数以及ACTION函数
#### 构造识别有效项目集及活前缀的DFA的算法
```CPP
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
    }S
```
- 根据上面的若干算法生成DFA，较简单，不予分析
#### 构造LR(1)分析表的函数
```CPP
    void SyntaxLR::ConstructLR1() {
        ...
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
    }
```
#### 进行LR(1)分析的函数
```cpp
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
            SrcStr = token; // 拷贝字符串，用于输出结果
            token.append(STACKBOTTOM); // 栈底符号入栈
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
```
### 五、测试
- 针对所要求的文法，对两种语法分析方式分别进行了五次测试，三次输入正确的表达式，两次输入错误的表达式
- 这五次测试的输入展示如下
```TXT
(1+1)
1/2/3/4/5/6/7
(1*((((2+3)*4)+5/6)-7*(8+9))/0)
(1+2)*/3
((1+2)/3
```
- LL(1)分析的测试结果如下
```TXT
PS C:\Users\Admin\Desktop\SyntaxAnalysis\bin> .\syntax.exe -ll .\gram\Grammar1.txt
Start Syntax Analyze...
Please input string, program will analyze whether it belongs to
the language of the grammar(Double Enter To Exit):
(1+1)
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 1
OutPut: T' -> [EPSILON]
OutPut: E' -> + T E'
Analyzed Symbol: +
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 1
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: $
String: "(1+1)" do belong to the grammar!

Start Syntax Analyze...
Please input string, program will analyze whether it belongs to
the language of the grammar(Double Enter To Exit):
1/2/3/4/5/6/7
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 1
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 2
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 3
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 4
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 5
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 6
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 7
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: $
String: "1/2/3/4/5/6/7" do belong to the grammar!

Start Syntax Analyze...
Please input string, program will analyze whether it belongs to
the language of the grammar(Double Enter To Exit):
(1*((((2+3)*4)+5/6)-7*(8+9))/0)
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 1
OutPut: T' -> * F T'
Analyzed Symbol: *
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 2
OutPut: T' -> [EPSILON]
OutPut: E' -> + T E'
Analyzed Symbol: +
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 3
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> * F T'
Analyzed Symbol: *
OutPut: F -> num
Analyzed Symbol: 4
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> [EPSILON]
OutPut: E' -> + T E'
Analyzed Symbol: +
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 5
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 6
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> [EPSILON]
OutPut: E' -> - T E'
Analyzed Symbol: -
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 7
OutPut: T' -> * F T'
Analyzed Symbol: *
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 8
OutPut: T' -> [EPSILON]
OutPut: E' -> + T E'
Analyzed Symbol: +
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 9
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 0
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: $
String: "(1*((((2+3)*4)+5/6)-7*(8+9))/0)" do belong to the grammar!

Start Syntax Analyze...
Please input string, program will analyze whether it belongs to
the language of the grammar(Double Enter To Exit):
(1+2)*/3
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 1
OutPut: T' -> [EPSILON]
OutPut: E' -> + T E'
Analyzed Symbol: +
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 2
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> * F T'
Analyzed Symbol: *
Wrong StackTop Symbol: F
Or Wrong Analyzing Symbol: /
Sting: "(1+2)*/3" does not belong to the grammar!

Start Syntax Analyze...
Please input string, program will analyze whether it belongs to
the language of the grammar(Double Enter To Exit):
((1+2)/3
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> ( E )
Analyzed Symbol: (
OutPut: E -> T E'
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 1
OutPut: T' -> [EPSILON]
OutPut: E' -> + T E'
Analyzed Symbol: +
OutPut: T -> F T'
OutPut: F -> num
Analyzed Symbol: 2
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Analyzed Symbol: )
OutPut: T' -> / F T'
Analyzed Symbol: /
OutPut: F -> num
Analyzed Symbol: 3
OutPut: T' -> [EPSILON]
OutPut: E' -> [EPSILON]
Wrong StackTop Symbol: )
Or Wrong Analyzing Symbol: $
Sting: "((1+2)/3" does not belong to the grammar!
```
- LR(1)的测试结果如下
```TXT
PS C:\Users\Admin\Desktop\SyntaxAnalysis\bin> .\syntax.exe -lr .\gram\Grammar1.txt
Start Syntax Analyze...
Please input string, program will analyze whether it belongs to
the language of the grammar(Double Enter To Exit):
(1+1)
States 0
Symbol $
input: 1+1)$
output: Shift 2

States 0     2
Symbol $     (
input: +1)$
output: Shift 6

States 0     2     6
Symbol $     (     num
input: +1)$
output: Reduce by F -> num

States 0     2     8
Symbol $     (     F
input: +1)$
output: Reduce by T -> F

States 0     2     9
Symbol $     (     T
input: +1)$
output: Reduce by E -> T

States 0     2     10
Symbol $     (     E
input: 1)$
output: Shift 19

States 0     2     10    19
Symbol $     (     E     +
input: )$
output: Shift 6

States 0     2     10    19    6
Symbol $     (     E     +     num
input: )$
output: Reduce by F -> num

States 0     2     10    19    8
Symbol $     (     E     +     F
input: )$
output: Reduce by T -> F

States 0     2     10    19    29
Symbol $     (     E     +     T
input: )$
output: Reduce by E -> E + T

States 0     2     10
Symbol $     (     E
input: $
output: Shift 20

States 0     2     10    20
Symbol $     (     E     )
input: $
output: Reduce by F -> ( E )

States 0     3
Symbol $     F
input: $
output: Reduce by T -> F

States 0     4
Symbol $     T
input: $
output: Reduce by E -> T

States 0     5
Symbol $     E
input: $
output: Reduce by E' -> E
Accept!

String: "(1+1)" do belong to the grammar!
...
测试输出过多，展示第一个的，其余可自行验证
```
### 六、分析测试结果
1. 该程序实现了两种不同的语法分析方法，且两种方法都能正确的进行
2. 该程序能够正确分析所输入字符串是否属于程序存储的文法，针对错误的字符串能够正确的判断
3. 该程序有着清晰的输出方式，较好地完成了题目的要求