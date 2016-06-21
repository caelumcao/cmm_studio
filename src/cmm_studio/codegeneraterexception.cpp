#include "codegeneraterexception.h"
#include "common.h"
#include <sstream>

CodeGeneraterException::CodeGeneraterException(int lineNo, const std::string &errorStr)
    : m_lineNo(lineNo), m_errorStr(errorStr)
{
}

std::string CodeGeneraterException::message()
{
    std::stringstream ss;
    ss << "line " << m_lineNo << ": " << m_errorStr;
    return ss.str();
}

void CodeGeneraterException::addToErrorMap()
{
    errorMap.insert(std::make_pair(m_lineNo, m_errorStr));
}

