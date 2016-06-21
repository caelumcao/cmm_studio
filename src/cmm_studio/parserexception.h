#ifndef PARSEREXCEPTION_H
#define PARSEREXCEPTION_H

#include <string>
#include "token.h"

class ParserException
{
public:
    ParserException(int errNo, Token token);
    ParserException(std::string tokenType, Token token);
    std::string message();
    void addToErrorMap();

private:
    int m_errNo;
    std::string m_tokenType;
    Token m_token;
};

#endif // PARSEREXCEPTION_H
