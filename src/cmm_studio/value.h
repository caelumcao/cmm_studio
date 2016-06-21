#ifndef VALUE_H
#define VALUE_H

#include <string>

class Value
{
public:
    enum { INT_VALUE,
           REAL_VALUE,
           CHAR_VALUE,
           STR_VALUE,
           ADDR,
           SINGLE_VALUE,
           VOID
         };

    Value();

    Value(const std::string & valueStr, int type);

    static int addrUnitSize;
    static int callfrom;

    std::string valueStr() const;
    void setValueStr(const std::string &valueStr);

    int type() const;
    void setType(int type);

    int size() const;
    void setSize(int size);

    std::string typeToStr();

    friend int toInt(Value value);
    friend double toDouble(Value value);

    friend std::string nequal(const Value & l_value, const Value & r_value);

    /*
     * 重载运算符
     */
    friend std::string operator+(const Value & l_value, const Value & r_value);
    friend std::string operator-(const Value & l_value, const Value & r_value);
    friend std::string operator*(const Value & l_value, const Value & r_value);
    friend std::string operator/(const Value & l_value, const Value & r_value);
    friend std::string operator>(const Value & l_value, const Value & r_value);
    friend std::string operator>=(const Value & l_value, const Value & r_value);
    friend std::string operator<(const Value & l_value, const Value & r_value);
    friend std::string operator<=(const Value & l_value, const Value & r_value);
    friend std::string operator==(const Value & l_value, const Value & r_value);


private:
    std::string m_valueStr;
    int m_type;
    int m_size;

};

#endif // VALUE_H
