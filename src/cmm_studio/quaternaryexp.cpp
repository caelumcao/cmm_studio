#include "quaternaryexp.h"
#include "common.h"

int QuaternaryExp::m_index = -1;
const std::string QuaternaryExp::JMP = "jmp";
const std::string QuaternaryExp::READ = "read";
const std::string QuaternaryExp::WRITE = "write";
const std::string QuaternaryExp::IN = "in";
const std::string QuaternaryExp::OUT = "out";
const std::string QuaternaryExp::INT = "int";
const std::string QuaternaryExp::REAL = "real";
const std::string QuaternaryExp::CHAR = "char";
const std::string QuaternaryExp::INT_POINT = "int*";
const std::string QuaternaryExp::REAL_POINT = "real*";
const std::string QuaternaryExp::CHAR_POINT = "char*";
const std::string QuaternaryExp::ASSIGN = "assign";
const std::string QuaternaryExp::PLUS = "+";
const std::string QuaternaryExp::MINUS = "-";
const std::string QuaternaryExp::MUL = "*";
const std::string QuaternaryExp::DIV = "/";
const std::string QuaternaryExp::GT = ">";
const std::string QuaternaryExp::LT = "<";
const std::string QuaternaryExp::GET = ">=";
const std::string QuaternaryExp::LET = "<=";
const std::string QuaternaryExp::EQ = "==";
const std::string QuaternaryExp::NEQ = "<>";
const std::string QuaternaryExp::ADDR = "&";
const std::string QuaternaryExp::CALL = "call";
const std::string QuaternaryExp::CALLFH = "callfh";
const std::string QuaternaryExp::VOID = "void";

QuaternaryExp::QuaternaryExp(const std::string &first, const std::string &second, const std::string &third, const std::string &forth, int lineNo)
                            : m_first(first), m_second(second), m_third(third), m_forth(forth), m_lineNo(lineNo)
{
    m_index++;
}

std::string QuaternaryExp::first() const
{
    return m_first;
}

void QuaternaryExp::setFirst(const std::string &first)
{
    m_first = first;
}
std::string QuaternaryExp::second() const
{
    return m_second;
}

void QuaternaryExp::setSecond(const std::string &second)
{
    m_second = second;
}
std::string QuaternaryExp::third() const
{
    return m_third;
}

void QuaternaryExp::setThird(const std::string &third)
{
    m_third = third;
}
std::string QuaternaryExp::forth() const
{
    return m_forth;
}

void QuaternaryExp::setForth(const std::string &forth)
{
    m_forth = forth;
}

int QuaternaryExp::index()
{
    return m_index;
}

void QuaternaryExp::setIndex(int index)
{
    m_index = index;
}
int QuaternaryExp::lineNo() const
{
    return m_lineNo;
}

void QuaternaryExp::setLineNo(int lineNo)
{
    m_lineNo = lineNo;
}

std::string QuaternaryExp::toString()
{
    return "line " + itos(m_lineNo) + ": (" + m_first + ", " + m_second + ", " + m_third + ", " + m_forth + ")";
}







