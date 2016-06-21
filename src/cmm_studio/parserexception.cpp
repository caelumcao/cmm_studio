#include "parserexception.h"
#include "common.h"
#include <sstream>

ParserException::ParserException(int errNo, Token token) : m_errNo(errNo), m_tokenType(""),  m_token(token)
{
}

ParserException::ParserException(std::string tokenType, Token token) : m_errNo(-1), m_tokenType(tokenType), m_token(token)
{
}

std::string ParserException::message()
{
    std::stringstream tokenStream;
    if (m_errNo == -1)
        tokenStream << "line " << m_token.lineNoValue() << ": \"" << m_token.value() << "\", expected \"" << m_tokenType << "\"";
    else
        tokenStream << "line " << m_token.lineNoValue() << ": \"" << m_token.value() << "\", expected \"" << Token::typeToString(m_errNo) << "\"";
    return tokenStream.str();
}

void ParserException::addToErrorMap()
{
    std::stringstream ss;
    if (m_errNo == -1)
        ss << "expected '" << m_tokenType << "', got '" << m_token.value() << "'";
    else
        ss << "expected '" << Token::typeToString(m_errNo) << "', got '" << m_token.value() << "'";
    errorMap.insert(std::make_pair(m_token.lineNoValue(), ss.str()));
}
