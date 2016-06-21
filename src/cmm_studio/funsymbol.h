#ifndef FUNSYMBOL_H
#define FUNSYMBOL_H

#include <string>
#include <vector>

class FunSymbol
{
public:
    enum {
        INT,
        REAL,
        CHAR,
        INT_POINT,
        REAL_POINT,
        CHAR_POINT,
        VOID
    };

    FunSymbol(const std::string & name, int lineNo, int returnType, int stmtIndex = -1);

    std::string name() const;
    void setName(const std::string &name);

    int lineNo() const;
    void setLineNo(int lineNo);

    int returnType() const;
    void setReturnType(int returnType);

    std::vector<int> m_argTypeVec;

    int stmtIndex() const;
    void setStmtIndex(int stmtIndex);

private:
    std::string m_name;
    int m_lineNo;
    int m_returnType;
    int m_stmtIndex;
};

#endif // FUNSYMBOL_H
