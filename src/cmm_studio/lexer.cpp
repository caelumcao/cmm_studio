#include "lexer.h"
#include "token.h"
//#include "error.h"
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <QDebug>
#include "common.h"

Lexer::Lexer()
{
    m_isEOF = false;
    m_lineNo = 0;
    m_curChar = ' ';
}

//词法分析
std::vector<Token> Lexer::analyze(const std::string &text)
{
    m_tokenVec.clear();
    m_isEOF = false;
    m_lineNo = 1;
    m_stream.clear();
    m_stream.str(text);

    readChar();
    while (!m_isEOF)
    {
        if (isBlankChar())
        {
            readChar();
            continue;
        }
        if (isSingleChar())
        {
            readChar();
            continue;
        }
        if (isCombinedChar())
        {
            continue;
        }
        if (isDigit())
        {
            continue;
        }
        if (isRWorID())
        {
            continue;
        }
        errorVec.push_back(2);
        m_tokenVec.push_back(Token(Token::ERR, m_lineNo, errorStr(2)));
        errorMap.insert(std::make_pair(m_lineNo, errorStr(2)));
        readChar();
    }
    return m_tokenVec;
}

//读取一个字符
void Lexer::readChar()
{
    m_curChar = m_stream.get();
    if (int(m_curChar) == -1)
        m_isEOF = true;
}

//判断是否是空白字符
bool Lexer::isBlankChar()
{
    //if (m_curChar == '\n' || m_curChar == '\r' || m_curChar == '\t' || m_curChar == '\f' || m_curChar == ' ')
    if (isspace(m_curChar))
    {
        if (m_curChar == '\n')
            m_lineNo++;
        return true;
    } else {
        return false;
    }
}

//决断是否是简单的特殊符号
bool Lexer::isSingleChar()
{
    switch (m_curChar) {
    case '+':
        m_tokenVec.push_back(Token(Token::PLUS, m_lineNo, "+"));
        return true;
    case '-':
        m_tokenVec.push_back(Token(Token::MINUS, m_lineNo, "-"));
        return true;
    case '*':
        m_tokenVec.push_back(Token(Token::MUL, m_lineNo, "*"));
        return true;
    case ',':
        m_tokenVec.push_back(Token(Token::COMMA, m_lineNo, ","));
        return true;
    case ';':
        m_tokenVec.push_back(Token(Token::SEMI, m_lineNo, ";"));
        return true;
    case '(':
        m_tokenVec.push_back(Token(Token::LPS, m_lineNo, "("));
        return true;
    case ')':
        m_tokenVec.push_back(Token(Token::RPS, m_lineNo, ")"));
        return true;
    case '[':
        m_tokenVec.push_back(Token(Token::LBRACKET, m_lineNo, "["));
        return true;
    case ']':
        m_tokenVec.push_back(Token(Token::RBRACKET, m_lineNo, "]"));
        return true;
    case '{':
        m_tokenVec.push_back(Token(Token::LBRACE, m_lineNo, "{"));
        return true;
    case '}':
        m_tokenVec.push_back(Token(Token::RBRACE, m_lineNo, "}"));
        return true;
    default:
        return false;
    }
}

//判断是否是复合特殊符号
bool Lexer::isCombinedChar()
{
    if (m_curChar == '/')
    {
        readChar();
        if (m_curChar == '*')
        { //多行注释
            m_tokenVec.push_back(Token(Token::LCOM, m_lineNo, "/*"));
            readChar();
            while (!m_isEOF)    //读取完注释字符
            {
                if (m_curChar == '\n')
                    m_lineNo++;
                if (m_curChar == '*') {
                    readChar();
                    if (m_curChar == '/')
                    {
                        m_tokenVec.push_back(Token(Token::RCOM, m_lineNo, "*/"));
                        break;
                    }
                } else {
                    readChar();
                }
            }
            if (!m_isEOF)
                readChar();
        } else if (m_curChar == '/') {
          //单行注释
            m_tokenVec.push_back(Token(Token::SCOM, m_lineNo, "//"));
            readChar();
            while (!m_isEOF && m_curChar != '\n')   //读取完注释字符
                readChar();
            if (m_curChar == '\n')
            {
                m_lineNo++;
                readChar();
            }
        } else {
          //除号
            m_tokenVec.push_back(Token(Token::DIV, m_lineNo, "/"));
        }
        return true;
    } else if (m_curChar == '=') {
        readChar();
        if (m_curChar == '=')
        {
            m_tokenVec.push_back(Token(Token::EQ, m_lineNo, "=="));
            readChar();
        } else {
            m_tokenVec.push_back(Token(Token::ASSIGN, m_lineNo, "="));
        }
        return true;
    } else if (m_curChar == '>') {
        readChar();
        if (m_curChar == '=')
        {
            m_tokenVec.push_back(Token(Token::GET, m_lineNo, ">="));
            readChar();
        } else {
            m_tokenVec.push_back(Token(Token::GT, m_lineNo, ">"));
        }
        return true;
    } else if (m_curChar == '<') {
        readChar();
        if (m_curChar == '=')
        {
            m_tokenVec.push_back(Token(Token::LET, m_lineNo, "<="));
            readChar();
        } else if (m_curChar == '>') {
            m_tokenVec.push_back(Token(Token::NEQ, m_lineNo, "<>"));
            readChar();
        } else {
            m_tokenVec.push_back(Token(Token::LT, m_lineNo, "<"));
        }
        return true;
    } else if (m_curChar == '\'') {
        m_tokenVec.push_back(Token(Token::SINQS, m_lineNo, "'"));
        std::stringstream charStream;
        while (true)
        {
            readChar();
            if (m_curChar == '\'')
            {
                m_tokenVec.push_back(Token(Token::CHAR_VALUE, m_lineNo, charStream.str()));
                m_tokenVec.push_back(Token(Token::SINQS, m_lineNo, "'"));
                readChar();
                break;
            } else if (m_curChar == '\n' || m_isEOF) {
                m_tokenVec.push_back(Token(Token::CHAR_VALUE, m_lineNo, charStream.str()));
                break;
            } else {
                charStream << m_curChar;
            }
        }
        return true;
    } else if (m_curChar == '"') {
        m_tokenVec.push_back(Token(Token::DOUQS, m_lineNo, "\""));
        std::stringstream strStream;
        while (true)
        {
            readChar();
            if (m_curChar == '"')
            {
                m_tokenVec.push_back(Token(Token::STR_VALUE, m_lineNo, strStream.str()));
                m_tokenVec.push_back(Token(Token::DOUQS, m_lineNo, "\""));
                readChar();
                break;
            } else if (m_curChar == '\n' || m_isEOF) {
                m_tokenVec.push_back(Token(Token::STR_VALUE, m_lineNo, strStream.str()));
                break;
            } else if (m_curChar == '\\') {
                readChar();
                if (m_curChar == 't')
                    strStream << '\t';
                else if (m_curChar == 'n')
                    strStream << '\n';
                else
                    strStream << m_curChar;
            } else {
                strStream << m_curChar;
            }
        }
        return true;
    } else if (m_curChar == '&') {
        readChar();
        if (m_curChar == '&')
        {

        } else {
            m_tokenVec.push_back(Token(Token::ADDR, m_lineNo, "&"));
        }
    } else {
        return false;
    }
}

//判断是否是数字
bool Lexer::isDigit()
{
    if (isdigit(m_curChar))
    {
        bool isReal = false;
        std::stringstream numStream;    //用来存放数字字符
        numStream << m_curChar;
        readChar();
        while (isdigit(m_curChar) || m_curChar == '.')
        {
            if (m_curChar == '.')
            {
                if (isReal)
                    break;
                else
                    isReal = true;
            }
            numStream << m_curChar;
            readChar();
        }
        if (isReal)
            m_tokenVec.push_back(Token(Token::REAL_VALUE, m_lineNo, numStream.str()));
        else
            m_tokenVec.push_back(Token(Token::INT_VALUE, m_lineNo, numStream.str()));
        return true;
    } else {
        return false;
    }
}

//判断是否保留字或ID
bool Lexer::isRWorID()
{
    if (isalpha(m_curChar) || m_curChar == '_')
    {
        std::stringstream wordStream;
        wordStream << m_curChar;
        readChar();
        while (isalnum(m_curChar) || m_curChar == '_')
        {
            wordStream << m_curChar;
            readChar();
        }
        std::string word = wordStream.str();
        if (word.at(word.size() - 1) == '_')
        {
            errorVec.push_back(1);
            m_tokenVec.push_back(Token(Token::ERR, m_lineNo, errorStr(1)));
            errorMap.insert(std::make_pair(m_lineNo, errorStr(1)));
        } else if (word == "if") {
            m_tokenVec.push_back(Token(Token::IF, m_lineNo, word));
        } else if (word == "else") {
            m_tokenVec.push_back(Token(Token::ELSE, m_lineNo, word));
        } else if (word == "while") {
            m_tokenVec.push_back(Token(Token::WHILE, m_lineNo, word));
        } else if (word == "for") {
            m_tokenVec.push_back(Token(Token::FOR, m_lineNo, word));
        } else if (word == "read") {
            m_tokenVec.push_back(Token(Token::READ, m_lineNo, word));
        } else if (word == "write") {
            m_tokenVec.push_back(Token(Token::WRITE, m_lineNo, word));
        } else if (word == "int") {
            m_tokenVec.push_back(Token(Token::INT, m_lineNo, word));
        } else if (word == "real") {
            m_tokenVec.push_back(Token(Token::REAL, m_lineNo, word));
        } else if (word == "char") {
            m_tokenVec.push_back(Token(Token::CHAR, m_lineNo, word));
        } else if (word == "void") {
            m_tokenVec.push_back(Token(Token::VOI, m_lineNo, word));
        } else if (word == "return") {
            m_tokenVec.push_back(Token(Token::RETURN, m_lineNo, word));
        } else {
            m_tokenVec.push_back(Token(Token::ID, m_lineNo, word));
        }
        return true;
    } else {
        return false;
    }
}
