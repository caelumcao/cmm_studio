#include "funsymbol.h"

FunSymbol::FunSymbol(const std::string &name, int lineNo, int returnType, int stmtIndex)
{
    m_name = name;
    m_lineNo = lineNo;
    m_returnType = returnType;
    m_stmtIndex = stmtIndex;
}

std::string FunSymbol::name() const
{
    return m_name;
}

void FunSymbol::setName(const std::string &name)
{
    m_name = name;
}
int FunSymbol::lineNo() const
{
    return m_lineNo;
}

void FunSymbol::setLineNo(int lineNo)
{
    m_lineNo = lineNo;
}
int FunSymbol::returnType() const
{
    return m_returnType;
}

void FunSymbol::setReturnType(int returnType)
{
    m_returnType = returnType;
}
int FunSymbol::stmtIndex() const
{
    return m_stmtIndex;
}

void FunSymbol::setStmtIndex(int stmtIndex)
{
    m_stmtIndex = stmtIndex;
}




