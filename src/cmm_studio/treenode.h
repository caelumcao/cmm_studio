#ifndef TREENODE_H
#define TREENODE_H

#include <vector>
#include <string>

class TreeNode
{
public:
    enum {
        NUL,            //空
        IF_STMT,        //if语句
        WHILE_STMT,     //while语句
        FOR_STMT,       //for语句
        READ_STMT,      //read语句
        WRITE_STMT,     //write语句
        DECLARE_STMT,   //声明语句
        ASSIGN_STMT,    //赋值语句
        RETURN_STMT,    //return语句
        EXP,            //表达式
        VAR,            //变量
        FUNDECLARE,     //函数声明
        FUNCALL,        //函数调用
        VARTYPE,        //变量类型
        OP,             //运算符
        FACTOR,         //因子
        LITERAL,        //字面值
        REG             //寄存器
    };

    TreeNode(int type);
    ~TreeNode();

    TreeNode * left() const;
    void setLeft(TreeNode *left);

    TreeNode * middle() const;
    void setMiddle(TreeNode *middle);

    TreeNode * right() const;
    void setRight(TreeNode *right);

    TreeNode * next() const;
    void setNext(TreeNode *next);

    int type() const;
    void setType(int type);

    int dataType() const;
    void setDataType(int dataType);

    

    std::string value() const;
    void setValue(const std::string &value);

    int lineNo() const;
    void setLineNo(int lineNo);

private:
    int m_type;
    int m_lineNo;
    std::vector<TreeNode *> m_childVec;
    TreeNode * m_left;
    TreeNode * m_middle;
    TreeNode * m_right;
    TreeNode * m_next;    //如果是代码块中的代码,则m_next指向其后面的一条语句

    /**
     * @brief m_dataType
     * VAR: INT/REAL...
     * OP:  LT/GT...
     * FACTOR:
     * LITERAL: 字面值的存储类型
     */
    int m_dataType;

    /**
     * @brief value
     * FACTOR:  存储表达式的字符串形式的值
     * VAR:     存储变量名
     */
    std::string m_value;

};

#endif // TREENODE_H
