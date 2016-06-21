#include "symbol.h"

Symbol::Symbol()
{

}

Symbol::Symbol(const std::string &name, int type) : m_name(name), m_type(type)
{

}

Symbol::Symbol(int valueIndex, int type) : m_valueIndex(valueIndex), m_type(type)
{

}

Symbol::Symbol(const std::string &name, int type, int lineNo, int level, int elementNum)
              : m_name(name), m_type(type), m_lineNo(lineNo), m_level(level), m_elementNum(elementNum)
{
    switch (type) {
    case SINGLE_INT:
    case ARRAY_INT:
    case POINT_CHAR:
    case POINT_INT:
    case POINT_REAL:
        m_dataSize = 4;
        break;
    case SINGLE_REAL:
    case ARRAY_REAL:
        m_dataSize = 8;
        break;
    case SINGLE_CHAR:
    case ARRAY_CHAR:
        m_dataSize = 1;
        break;
    default:
        break;
    }
}

int Symbol::type() const
{
    return m_type;
}

void Symbol::setType(int type)
{
    m_type = type;
}
int Symbol::dataSize() const
{
    return m_dataSize;
}

void Symbol::setDataSize(int dataSize)
{
    m_dataSize = dataSize;
}
int Symbol::level() const
{
    return m_level;
}

void Symbol::setLevel(int level)
{
    m_level = level;
}
std::string Symbol::name() const
{
    return m_name;
}

void Symbol::setName(const std::string &name)
{
    m_name = name;
}
int Symbol::elementNum() const
{
    return m_elementNum;
}

void Symbol::setElementNum(int elementNum)
{
    m_elementNum = elementNum;
}
int Symbol::lineNo() const
{
    return m_lineNo;
}

void Symbol::setLineNo(int lineNo)
{
    m_lineNo = lineNo;
}
int Symbol::valueIndex() const
{
    return m_valueIndex;
}

void Symbol::setValueIndex(int valueIndex)
{
    m_valueIndex = valueIndex;
}









