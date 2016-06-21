#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "symbol.h"
#include "funsymbol.h"
#include <vector>
#include <string>

class TreeNode;

class SymbolTable
{
public:
    SymbolTable();

    void clearVec();
    void registerSymbol(const Symbol & symbol);
    void registerFunSymbol(const FunSymbol & funSymbol);
    void deregisterSymbol(int level);

    std::string getNewTempSymbolName();
    Symbol getSymbol(const std::string & name, int & index, int & dereference);
    int getSymbolType(const std::string & name);
    int getCurFunSymbolType();
    FunSymbol getFunSymbol(const std::string & name, int lineNo);
    int getLastSymbolType();
    void clearTempVec();
    int checkSymbolIsDeclared(const TreeNode * node, int lineNo);

    std::vector<Symbol> symbolVec() const;
    void setSymbolVec(const std::vector<Symbol> &symbolVec);

    std::vector<FunSymbol> funSymbolVec() const;
    void setFunSymbolVec(const std::vector<FunSymbol> &funSymbolVec);

private:
    static const std::string TEMP_PREFIX;

    std::vector<Symbol> m_symbolVec;
    std::vector<FunSymbol> m_funSymbolVec;
    std::vector<Symbol> m_tempSymbolVec;

};

SymbolTable & mySymbolTable();

#endif // SYMBOLTABLE_H
