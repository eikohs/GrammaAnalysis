#include "grammar.h"
#include <iostream>

namespace Grammar{
    std::string EMPTY = "[EPSILON]";

    void Grammar::LoadGrammar() {
        // TODO 完成对文法产生式的读取
    }

    std::ostream &operator<<(std::ostream& os, const SymbolSet &symbolSet) {
        for(auto it = symbolSet.begin();it != symbolSet.end();it++)
            os << *it << ' ';
        return os;
    }
}

