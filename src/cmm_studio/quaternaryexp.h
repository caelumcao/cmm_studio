#ifndef QUATERNARYEXP_H
#define QUATERNARYEXP_H

#include <string>

class QuaternaryExp
{
public:
    static const std::string JMP;
    static const std::string READ;
    static const std::string WRITE;
    static const std::string IN;
    static const std::string OUT;
    static const std::string INT;
    static const std::string REAL;
    static const std::string CHAR;
    static const std::string INT_POINT;
    static const std::string REAL_POINT;
    static const std::string CHAR_POINT;
    static const std::string ASSIGN;
    static const std::string PLUS;
    static const std::string MINUS;
    static const std::string MUL;
    static const std::string DIV;
    static const std::string GT;
    static const std::string LT;
    static const std::string GET;
    static const std::string LET;
    static const std::string EQ;
    static const std::string NEQ;
    static const std::string ADDR;
    static const std::string CALL;
    static const std::string CALLFH;
    static const std::string VOID;

    QuaternaryExp(const std::string & first, const std::string & second, const std::string & third, const std::string & forth, int lineNo = -1);

    std::string first() const;
    void setFirst(const std::string &first);

    std::string second() const;
    void setSecond(const std::string &second);

    std::string third() const;
    void setThird(const std::string &third);

    std::string forth() const;
    void setForth(const std::string &forth);

    static int index();
    static void setIndex(int index);

    int lineNo() const;
    void setLineNo(int lineNo);

    std::string toString();

private:
    std::string m_first;
    std::string m_second;
    std::string m_third;
    std::string m_forth;
    int m_lineNo;
    static int m_index;
};

#endif // QUATERNARYEXP_H
