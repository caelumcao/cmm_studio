#include "symboltable.h"
#include "codegeneraterexception.h"
#include "codeexecuteexception.h"
#include "common.h"
#include <sstream>
#include <stdlib.h>
#include "treenode.h"
#include "value.h"
#include <QDebug>

const std::string SymbolTable::TEMP_PREFIX = "#T";

SymbolTable::SymbolTable()
{
}

void SymbolTable::clearVec()
{
    m_symbolVec.clear();
    m_tempSymbolVec.clear();
    m_funSymbolVec.clear();
}

void SymbolTable::registerSymbol(const Symbol &symbol)
{
    for (int i = 0; i < m_symbolVec.size(); ++i)
    {
        if (m_symbolVec.at(i).name() == symbol.name() && m_symbolVec.at(i).level() == symbol.level())
        {
            std::stringstream ss;
            ss << "'" << symbol.name() << "' previously declared here";
            throw CodeGeneraterException(symbol.lineNo(), ss.str());
        }
    }
    m_symbolVec.push_back(symbol);
}

void SymbolTable::registerFunSymbol(const FunSymbol &funSymbol)
{
    for (int i = 0; i < m_funSymbolVec.size(); ++i)
    {
        if (m_funSymbolVec.at(i).name() == funSymbol.name())
        {
            std::stringstream ss;
            ss << "'" << funSymbol.name() << "' previously declared here!!!!!";
            throw CodeGeneraterException(funSymbol.lineNo(), ss.str());
        }
    }
    m_funSymbolVec.push_back(funSymbol);
}

void SymbolTable::deregisterSymbol(int level)
{
    while (!m_symbolVec.empty() && m_symbolVec.back().level() == level)
        m_symbolVec.pop_back();
}

std::string SymbolTable::getNewTempSymbolName()
{
    std::string temp = "";
    for (int i = 1; ; ++i)
    {
        std::stringstream ss;
        ss << TEMP_PREFIX << i;
        temp = ss.str();
        bool isExisted = false;
        for (int j = 0; j < m_tempSymbolVec.size(); ++j)
        {
            if (m_tempSymbolVec.at(j).name() == temp)
            {
                isExisted = true;
                break;
            }
        }
        if (isExisted)
            continue;
        Symbol s(temp, Symbol::TEMP, -1, -1);
        m_tempSymbolVec.push_back(s);
        return s.name();
    }
}

Symbol SymbolTable::getSymbol(const std::string &name, int &index, int &dereference)
{
    for (int i = m_symbolVec.size() - 1; i >= 0; --i)
    {
        Symbol tempSymbol = m_symbolVec.at(i);      //var
//        if (tempSymbol.name() == name)
//            return tempSymbol;
//        else if(name.size() > tempSymbol.name().size() && name.find(tempSymbol.name()) ==  0 && tempSymbol.elementNum() > 0)     //var[]
//        {
//            if (tempSymbol.elementNum() <= index || index < 0)
//                throw CodeExecuteException(curExecuteStmtLineNo, "数组越界");
//            return tempSymbol;
//        } else if (name.size() > tempSymbol.name().size() && name.find(tempSymbol.name()) == 0 && tempSymbol.elementNum() < 0) {
//            if (index < 0)
//                throw CodeExecuteException(curExecuteStmtLineNo, "非法的指针运算");
//            return tempSymbol;
//        } else if (name.at(0) == '*' && name.substr(1, name.size() - 1) == tempSymbol.name()) {     //*var
//            dereference = 1;
//            return tempSymbol;
//        } else if (name.at(0) == '*' && name.substr(1, name.size() - 1).find(tempSymbol.name()) && tempSymbol.elementNum() > 0) {
//            dereference = 1;
//            if (tempSymbol.elementNum() <= index || index < 0)
//                throw CodeExecuteException(curExecuteStmtLineNo, "数组越界");
//            return tempSymbol;
//        }
        if (tempSymbol.name() == name) {  //var
            return tempSymbol;
        } else if (name.find(tempSymbol.name()) == 0 && name.at(tempSymbol.name().size()) == '[') { //var[x]
            if (tempSymbol.elementNum() > 0)
            {
                if (tempSymbol.elementNum() <= index || index < 0)
                    throw CodeExecuteException(curExecuteStmtLineNo, "数组越界");
                return tempSymbol;
            } else if (tempSymbol.elementNum() < 0) {
                return tempSymbol;
            }
        } else if (name.at(0) == '*' && name.substr(1, name.size() - 1) == tempSymbol.name()) {     //*var
            dereference = 1;
            return tempSymbol;
        } else if (name.at(0) == '*' && name.find(tempSymbol.name()) == 1 && name.at(tempSymbol.name().size() + 1) == '[') {    //*var[x]
            dereference = 1;
            if (tempSymbol.elementNum() <= index || index < 0)
                throw CodeExecuteException(curExecuteStmtLineNo, "数组越界");
            return tempSymbol;
        }
    }
    if (name.at(0) == '#')
        return Symbol(name);
}

int SymbolTable::getSymbolType(const std::string &name)
{
    int index = -1;
    int dereference = 0;
    return getSymbol(name, index, dereference).type();
}

int SymbolTable::getCurFunSymbolType()
{
    return m_funSymbolVec.back().returnType();
}

FunSymbol SymbolTable::getFunSymbol(const std::string &name, int lineNo)
{
    for (int i = 0; i < m_funSymbolVec.size(); ++i)
    {
        FunSymbol funSymbol = m_funSymbolVec.at(i);
        if (funSymbol.name() == name)
            return funSymbol;
    }
    throw CodeGeneraterException(lineNo, "该函数未声明");
}

int SymbolTable::getLastSymbolType()
{
    return m_symbolVec.back().type();
}

void SymbolTable::clearTempVec()
{
    m_tempSymbolVec.clear();
}

//检查变量是否被定义，如果被定义则返回变量的类型(包含数组运算)
int SymbolTable::checkSymbolIsDeclared(const TreeNode *node, int lineNo)
{
    for (int i = 0; i < m_symbolVec.size(); ++i)
    {
        Symbol tempSymbol = m_symbolVec.at(i);
        if (tempSymbol.name() == node->value())
        {
            if (tempSymbol.elementNum() == 0)
            {
                if (node->left() == NULL)
                    return 0;   //普通变量
                else
                    throw CodeGeneraterException(lineNo, "'" + node->value() + "' 不是数组，无法进行下标操作" );
            } else if (tempSymbol.elementNum() < 0) {
                if (node->left() == NULL)
                    return -1;  //指针
                else
                    return 0;   //相当于普通变量
            } else {
                switch (tempSymbol.type()) {
                case Symbol::ARRAY_CHAR:
                case Symbol::ARRAY_INT:
                case Symbol::ARRAY_REAL:
                    if (node->left() == NULL)
                    {
                        if (tempSymbol.type() == Symbol::ARRAY_CHAR)
                            return 3;   //char数组 cin时用
                        else
                            return 1;   //数组地址
                    } else {
                        return 0;   //相当于普通变量
                    }
                default:
                    if (node->left() == NULL)
                        return 2;  //指针数组
                    else
                        return -1;      //相当于指针
                }
            }
        }
    }
    throw CodeGeneraterException(lineNo, "'" + node->value() + "' was not declared in this scope");
}
std::vector<Symbol> SymbolTable::symbolVec() const
{
    return m_symbolVec;
}

void SymbolTable::setSymbolVec(const std::vector<Symbol> &symbolVec)
{
    m_symbolVec = symbolVec;
}
std::vector<FunSymbol> SymbolTable::funSymbolVec() const
{
    return m_funSymbolVec;
}

void SymbolTable::setFunSymbolVec(const std::vector<FunSymbol> &funSymbolVec)
{
    m_funSymbolVec = funSymbolVec;
}




SymbolTable &mySymbolTable()
{
    static SymbolTable st;
    return st;
}
