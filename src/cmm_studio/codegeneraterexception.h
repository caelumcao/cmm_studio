#ifndef CODEGENERATEREXCEPTION_H
#define CODEGENERATEREXCEPTION_H

#include <string>

class CodeGeneraterException
{
public:
    CodeGeneraterException(int lineNo, const std::string & errorStr);
    std::string message();
    void addToErrorMap();

private:
    int m_lineNo;
    std::string m_errorStr;
};

#endif // CODEGENERATEREXCEPTION_H
