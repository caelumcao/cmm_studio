#include "codeexecuteexception.h"
#include <sstream>

CodeExecuteException::CodeExecuteException(int lineNo, const std::string &errorStr)
    : m_lineNo(lineNo), m_errorStr(errorStr)
{
}

std::string CodeExecuteException::message()
{
    std::stringstream ss;
    ss << "line " << m_lineNo << ": " << m_errorStr;
    return ss.str();
}
