#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include <sstream>
#include "token.h"
#include "error.h"

class Lexer
{
public:
    Lexer();
    std::vector<Token> analyze(const std::string & text);     //词法分析

private:
    void readChar();        //读取一个字符
    bool isBlankChar();     //判断是否是空白字符
    bool isSingleChar();    //决断是否是简单的特殊符号
    bool isCombinedChar();  //判断是否是复合特殊符号
    bool isDigit();         //判断是否是数字
    bool isRWorID();        //判断是否保留字或ID

private:
    bool m_isEOF;                       //是否读取结束
    int m_lineNo;                       //行号
    std::vector<Token> m_tokenVec;    //存放Token信息
    char m_curChar;                     //当前读取的字符
    std::stringstream m_stream;         //传入的流
};

#endif // LEXER_H
