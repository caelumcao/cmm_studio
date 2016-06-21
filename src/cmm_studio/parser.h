#ifndef PARSER_H
#define PARSER_H

#include "treenode.h"
#include "token.h"
#include <vector>
#include <string>
#include <map>
#include <queue>

struct TreeNodeStr {
    std::string value;
    int childNum;
};

typedef std::vector<std::queue<TreeNodeStr> > QueueVec;
typedef std::queue<TreeNodeStr> TQueue;


class Parser
{
public:
    Parser();
    std::vector<TreeNode *> syntacticAnalyse(std::vector<Token> tokenVec, std::vector<QueueVec> & qvvec, std::string & errorStr);    //语句分析接口

private:
    std::vector<TreeNode *> m_treeNodeVec;
    std::vector<Token> m_tokenVec;
    TreeNode * m_errorNode;               //出现异常的头结点，用来释放内存
    Token m_curToken;
    int index;                          //记录tokenVec的偏移量
    QueueVec m_queueVec;                //存储输出数据

    int m_argIndex;                     //函数参数索引

    TreeNode * parseStmt(int level);               //语句处理
    TreeNode * parseIfStmt(int level);
    TreeNode * parseWhileStmt(int level);
    TreeNode * parseForStmt(int level);
    TreeNode * parseReadStmt(int level);
    TreeNode * parseWriteStmt(int level);
    TreeNode * parseDeclareStmt(int level, bool isParseFun = false);
    TreeNode * parseAssignStmt(int level, bool isParseFor = false);
    TreeNode * parseStmtBlock(int level);
    TreeNode * parseExp(int level);                //表达式
    TreeNode * parseAddtiveExp(int level);         //多项式
    TreeNode * parseTerm(int level);               //项
    TreeNode * parseFactor(int level);             //因子
    TreeNode * parseLiteral(int level);            //字面值
    TreeNode * parseLogicOp(int level);            //逻辑运算符
    TreeNode * parseAddtiveOp(int level);          //加减运算符
    TreeNode * parseMutiplyOp(int level);          //乘除运算符
    TreeNode * parseVariableName(int level, bool isParseFun = false, bool isPoint = false);       //变量名
    TreeNode * parseFunctionDeclare(int level);    //函数名(type 0:声明 1:调用)
    TreeNode * parseFunctionCall(int level, bool isStmt = true);       //函数调用
    TreeNode * parseVariableType(int level);       //变量类型
    TreeNode * parseReturn(int level);


    /**
     * @brief consumeNextToken
     * 消耗掉下一个token,要求必须是type类型,消耗之后currentToken值将停在最后消耗的token上
     */
    void consumeNextToken(int type, int level);
    void consumeNextToken(int types[], int size, std::string typeStr, int level);
    
    bool checkNextTokenType(int types[], int size);
    
    int getNextTokenType();             //获取下一个token的type 没有返回Token::NUL

    int getNextNextTokenType();         //获取下下个token的type

    int getNextTokenLineNo();           //获取下一个token的lineNo，没有返回当前的lineNo;

    void insertIntoQueueVec(const std::string & value, int childNum, int level);    //将数据存入语法树输出容器

    int getPointType(int type);

    int getAddrType(int type);

    int getDereType(int type);



};

Parser & myParser();

#endif // PARSER_H
