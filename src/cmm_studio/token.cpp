#include "token.h"
#include <sstream>

Token::Token()
{
    m_type = NULLL;
    m_lineNo = -1;
    m_value = "";
}

Token::Token(int type, int lineNo, std::string value)
{
    m_type = type;
    m_lineNo = lineNo;
    m_value = value;
}

std::string Token::toString()
{
    std::stringstream tokenStream;
    tokenStream << m_lineNo << ": ";
    switch (m_type) {
    case ERR:
        tokenStream << "ERROR: " << m_value;
        return tokenStream.str();
    case IF:
    case ELSE:
    case WHILE:
    case FOR:
    case READ:
    case WRITE:
    case INT:
    case REAL:
    case CHAR:
    case VOI:
    case RETURN:
        tokenStream << "reserved word: " << m_value;
        return tokenStream.str();
    case PLUS:
    case MINUS:
    case MUL:
    case DIV:
    case ASSIGN:
    case LT:
    case EQ:
    case NEQ:
    case LPS:
    case RPS:
    case COMMA:
    case SEMI:
    case LBRACE:
    case RBRACE:
    case LCOM:
    case RCOM:
    case SCOM:
    case LBRACKET:
    case RBRACKET:
    case SINQS:
    case DOUQS:
    case LET:
    case GET:
    case GT:
    case ADDR:
        tokenStream << m_value;
        return tokenStream.str();
    case ID:
        tokenStream << "ID, name = " << m_value;
        return tokenStream.str();
    case INT_VALUE:
        tokenStream << "int, value = " << m_value;
        return tokenStream.str();
    case REAL_VALUE:
        tokenStream << "real, value = " << m_value;
        return tokenStream.str();
    case CHAR_VALUE:
        tokenStream << "char, value = " << m_value;
        return tokenStream.str();
    case STR_VALUE:
        tokenStream << "char string, value = " << m_value;
        return tokenStream.str();
    }
}

int Token::lineNoValue()
{
    return m_lineNo;
}

int Token::typeValue()
{
    return m_type;
}
std::string Token::value() const
{
    return m_value;
}

std::string Token::typeToString(int type)
{
    switch (type) {
    case NULLL: return "";
    case IF: return "if";
    case ELSE: return "else";
    case WHILE: return "while";
    case READ: return "read";
    case WRITE: return "write";
    case FOR: return "for";
    case INT: return "int";
    case REAL: return "real";
    case CHAR: return "char";
    case VOI: return "void";
    case RETURN: return "return";
    case PLUS: return "+";
    case MINUS: return "-";
    case MUL: return "*";
    case DIV: return "/";
    case ASSIGN: return "=";
    case LT: return "<";
    case EQ: return "==";
    case NEQ: return "<>";
    case LPS: return "(";
    case RPS: return ")";
    case COMMA: return ",";
    case SEMI: return ";";
    case LBRACE: return "{";
    case RBRACE: return "}";
    case LCOM: return "/*";
    case RCOM: return "*/";
    case SCOM: return "//";
    case LBRACKET: return "[";
    case RBRACKET: return "]";
    case SINQS: return "'";
    case DOUQS: return "\"";
    case LET: return "<=";
    case GT: return ">";
    case GET: return ">=";
    case ADDR: return "&";
    case ID:  return "id";
    case INT_VALUE: return "int value";
    case REAL_VALUE: return "real value";
    case CHAR_VALUE: return "char value";
    case STR_VALUE: return "char string value";
    default: return "UNKNOWN";
    }
}
