#ifndef CODEEXECUTEEXCEPTION_H
#define CODEEXECUTEEXCEPTION_H
#include <string>

class CodeExecuteException
{
public:
    CodeExecuteException(int lineNo, const std::string & errorStr);
    std::string message();

private:
    int m_lineNo;
    std::string m_errorStr;
};

#endif // CODEEXECUTEEXCEPTION_H
