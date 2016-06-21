#include "value.h"
#include "common.h"
#include "codeexecuteexception.h"
#include "codegeneraterexception.h"
#include <stdlib.h>
#include <cstring>

int Value::addrUnitSize = 1;
int Value::callfrom = -10;

Value::Value()
{

}

Value::Value(const std::string &valueStr, int type)
            : m_type(type)
{
    bool isChar = false;
    if (valueStr.at(0) == '\'')
        isChar = true;
    switch (type) {
    case INT_VALUE:
        m_size = 4;
        if (isChar)
            m_valueStr = valueStr.at(1);
        else
            m_valueStr = itos(atoi(valueStr.c_str()));
        break;
    case REAL_VALUE:
        m_size = 8;
        if (isChar)
            m_valueStr = valueStr.at(1);
        else
            m_valueStr = dtos(atof(valueStr.c_str()));
        break;
    case CHAR_VALUE:
        m_size = 1;
        if (isChar)
            m_valueStr = valueStr;
        else
            m_valueStr = "'" + ctos(char(atoi(valueStr.c_str()))) + "'";
        break;
    case ADDR:
        m_size = 4;
        m_valueStr = valueStr;
        break;
    default:
        break;
    }
}

std::string Value::valueStr() const
{
    return m_valueStr;
}

void Value::setValueStr(const std::string &valueStr)
{
    m_valueStr = valueStr;
}
int Value::type() const
{
    return m_type;
}

void Value::setType(int type)
{
    m_type = type;
}
int Value::size() const
{
    return m_size;
}

void Value::setSize(int size)
{
    m_size = size;
}

std::string Value::typeToStr()
{
    switch (m_type) {
    case INT_VALUE:
        return "int";
    case REAL_VALUE:
        return "real";
    case CHAR_VALUE:
        return "char";
    case ADDR:
        return "address";
    default:
        return "";
    }
}

int toInt(Value value)
{
    switch (value.m_type) {
    case Value::CHAR_VALUE:
        return value.m_valueStr.at(1);
    case Value::INT_VALUE:
        return atoi(value.m_valueStr.c_str());
    case Value::ADDR:
        return atoi(value.m_valueStr.substr(1, value.m_valueStr.size() - 1).c_str());
    case Value::REAL_VALUE:
        throw CodeExecuteException(curExecuteStmtLineNo, "无法将浮点数和地址进行运算");
    }
    return 0;
}

double toDouble(Value value)
{
    switch (value.m_type) {
    case Value::CHAR_VALUE:
        return value.m_valueStr.at(1);
    case Value::INT_VALUE:
    case Value::REAL_VALUE:
        return atof(value.m_valueStr.c_str());
    }
    return 0;
}


std::string nequal(const Value &l_value, const Value &r_value)
{
    int result = 0;
    if ((l_value == r_value) == "0")
        result = 1;
    return itos(result);
}

std::string operator+(const Value &l_value, const Value &r_value)
{
    if ((l_value.type() == Value::INT_VALUE || l_value.type() == Value::CHAR_VALUE)
             && (r_value.type() == Value::INT_VALUE || r_value.type() == Value::CHAR_VALUE))
    {
        int result = toInt(l_value) + toInt(r_value);
        return itos(result);
    } else if (l_value.type() == Value::ADDR) {
        int result = toInt(l_value) + toInt(r_value) * Value::addrUnitSize;
        return "$" + itos(result);
    } else if (r_value.type() == Value::ADDR) {
        int result = toInt(l_value) * Value::addrUnitSize + toInt(r_value);
        return "$" + itos(result);
    } else if (l_value.type() == Value::REAL_VALUE || r_value.type() == Value::REAL_VALUE){
        double result = toDouble(l_value) + toDouble(r_value);
        return dtos(result);
    }
    return "0";
    //throw CodeExecuteException(curExecuteStmtLineNo, "运算表达式错误");
}

std::string operator-(const Value &l_value, const Value &r_value)
{
    if ((l_value.type() == Value::INT_VALUE || l_value.type() == Value::CHAR_VALUE)
             && (r_value.type() == Value::INT_VALUE || r_value.type() == Value::CHAR_VALUE))
    {
        int result = toInt(l_value) - toInt(r_value);
        return itos(result);
    } else if (l_value.type() == Value::ADDR) {
        int result = toInt(l_value) - toInt(r_value) * Value::addrUnitSize;
        return "$" + itos(result);
    } else if (l_value.type() == Value::REAL_VALUE || r_value.type() == Value::REAL_VALUE){
        double result = toDouble(l_value) - toDouble(r_value);
        return dtos(result);
    }
    return "0";
    //throw CodeExecuteException(curExecuteStmtLineNo, "运算表达式错误");
}

std::string operator*(const Value &l_value, const Value &r_value)
{
    if ((l_value.type() == Value::INT_VALUE || l_value.type() == Value::CHAR_VALUE)
             && (r_value.type() == Value::INT_VALUE || r_value.type() == Value::CHAR_VALUE))
    {
        int result = toInt(l_value) * toInt(r_value);
        return itos(result);
    } else if (l_value.type() == Value::REAL_VALUE || r_value.type() == Value::REAL_VALUE){
        double result = toDouble(l_value) * toDouble(r_value);
        return dtos(result);
    }
    return "0";
    //throw CodeExecuteException(curExecuteStmtLineNo, "运算表达式错误");
}

std::string operator/(const Value &l_value, const Value &r_value)
{
    if ((l_value.type() == Value::INT_VALUE || l_value.type() == Value::CHAR_VALUE)
             && (r_value.type() == Value::INT_VALUE || r_value.type() == Value::CHAR_VALUE))
    {
        if (toInt(r_value) == 0)
        {
            if (Value::callfrom == -10)
            {
                throw CodeExecuteException(curExecuteStmtLineNo, "除数不能为0");
            } else {
                int tempLineNo = Value::callfrom;
                Value::callfrom = -10;
                throw CodeGeneraterException(tempLineNo, "除数不能为0");
            }
        }
        int result = toInt(l_value) / toInt(r_value);
        return itos(result);
    } else if (l_value.type() == Value::REAL_VALUE || r_value.type() == Value::REAL_VALUE){
        if (toDouble(r_value) == 0)
        {
            if (Value::callfrom == -10)
            {
                throw CodeExecuteException(curExecuteStmtLineNo, "除数不能为0");
            } else {
                int tempLineNo = Value::callfrom;
                Value::callfrom = -10;
                throw CodeGeneraterException(tempLineNo, "除数不能为0");
            }
        }
        double result = toDouble(l_value) / toDouble(r_value);
        return dtos(result);
    }
    return "0";
    //throw CodeExecuteException(curExecuteStmtLineNo, "运算表达式错误");
}

std::string operator>(const Value &l_value, const Value &r_value)
{
    int result = 0;
    if (l_value.type() == Value::ADDR && r_value.type() == Value::ADDR)
        result = (toInt(l_value) > toInt(r_value));
//    else if (l_value.type() == Value::STR_VALUE && r_value.type() == Value::STR_VALUE)
//        result = (strcmp(l_value.valueStr(), r_value.valueStr()) > 0);
    else
        result = (atof((l_value - r_value).c_str()) > 0);
    return itos(result);
}

std::string operator>=(const Value &l_value, const Value &r_value)
{
    int result = 0;
    if (l_value.type() == Value::ADDR && r_value.type() == Value::ADDR)
        result = (toInt(l_value) >= toInt(r_value));
    else
        result = (atof((l_value - r_value).c_str()) >= 0);
    return itos(result);
}

std::string operator<(const Value &l_value, const Value &r_value)
{
    int result = 0;
    if(r_value > l_value != "0")
        result = 1;
    return itos(result);
}

std::string operator<=(const Value &l_value, const Value &r_value)
{
    int result = 0;
    if (r_value >= l_value != "0")
        result = 1;
    return itos(result);
}

std::string operator==(const Value &l_value, const Value &r_value)
{
    int result = 0;
    if (l_value.type() == Value::ADDR && r_value.type() == Value::ADDR)
        result = (toInt(l_value) == toInt(r_value));
    else if (l_value.type() == Value::STR_VALUE && r_value.type() == Value::STR_VALUE)
        result = (strcmp(l_value.valueStr().c_str(), r_value.valueStr().c_str()) == 0);
    else
        result = (atof((l_value - r_value).c_str()) == 0);
    return itos(result);
}





