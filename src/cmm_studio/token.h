#ifndef TOKEN_H
#define TOKEN_H

#include <string>


class Token
{
public:
    enum {
        /*空*/
        NULLL,
        /*错误标记*/
        ERR,
        /*保留字*/
        IF,
        ELSE,
        WHILE,      
        FOR,
        READ,
        WRITE,
        INT,
        REAL,
        CHAR,           //10
        VOI,
        /*特殊符号*/
        PLUS,   // '+'
        MINUS,  // '-'
        MUL,    // '*'
        DIV,    // '/'
        ASSIGN, // '='
        LT,     // '<'
        EQ,     // '=='
        NEQ,    // '<>'
        LPS,    // '('  //20
        RPS,    // ')'
        COMMA,  // ','
        SEMI,   // ';'
        LBRACE, // '{'
        RBRACE, // '}'
        LCOM,   // '/*'
        RCOM,   // '*/'
        SCOM,   // '//'
        LBRACKET, // '['
        RBRACKET, // ']'
        SINQS,  // '\'' //30
        DOUQS,  // '\"'
        LET,    // '<='
        GET,    // '>='
        GT,     // '>'
        ADDR,   // '&'
        /*标识符（由数字、字母和下划线组成的串，但必须以字母开头、且不能以下划线结尾的串）*/
        ID,
        RETURN,
        /*十进制的整数与实数*/
        INT_VALUE,  //int值
        REAL_VALUE, //real值         //40
        CHAR_VALUE, //char值
        STR_VALUE,  //char字符串

        //地址 &a
        INT_ADDR,
        REAL_ADDR,
        CHAR_ADDR,

        //解除引用 *a
        INT_DERE,
        REAL_DERE,
        CHAR_DERE,

        //变量指针 int*
        INT_POINT,
        REAL_POINT,         //50
        CHAR_POINT,

        DERE,

        LOGIC_EXP,
        ADDTIVE_EXP,
        TERM_EXP

    };

    Token();
    Token(int type, int lineNo, std::string value = "");
    std::string toString();
    int lineNoValue();
    int typeValue();

    std::string value() const;

    static std::string typeToString(int type);

private:
    int m_type;
    int m_lineNo;
    std::string m_value;
};

#endif // TOKEN_H
