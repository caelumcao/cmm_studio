#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>

class Symbol
{
public:
    enum {
        TEMP,
        REG,
        SINGLE_INT,
        SINGLE_REAL,
        SINGLE_CHAR,
        ARRAY_INT,
        ARRAY_REAL,
        ARRAY_CHAR,
        POINT_INT,
        POINT_REAL,
        POINT_CHAR,
        ARRAY_POINT_INT,
        ARRAY_POINT_REAL,
        ARRAY_POINT_CHAR
    };

    Symbol();
    Symbol(const std::string & name, int type = TEMP);
    Symbol(int valueIndex, int type = REG);
    Symbol(const std::string & name, int type, int lineNo, int level, int elementNum = 0);

    int type() const;
    void setType(int type);

    int dataSize() const;
    void setDataSize(int dataSize);

    int level() const;
    void setLevel(int level);

    std::string name() const;
    void setName(const std::string &name);

    int elementNum() const;
    void setElementNum(int elementNum);

    int lineNo() const;
    void setLineNo(int lineNo);

    int valueIndex() const;
    void setValueIndex(int valueIndex);

private:
    std::string m_name;     //变量名
    int m_lineNo;           //行号
    int m_type;             //变量类型
    int m_dataSize;         //数据的大小
    int m_level;            //级别
    int m_elementNum;       //元素的个数，非数组则为0
    int m_valueIndex;       //该变量值的索引

};

#endif // SYMBOL_H
