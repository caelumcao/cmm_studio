#ifndef CODEGENERATER_H
#define CODEGENERATER_H

#include "quaternaryexp.h"
#include "treenode.h"
#include "symboltable.h"
#include <vector>
#include <string>
#include <stack>

class CodeGenerater
{
public:
    CodeGenerater();
    std::vector<QuaternaryExp> generateCode(const std::vector<TreeNode *> & treeNodeVec, std::string & errorStr);

private:
    enum {ALL, VALUE, ADDR, CHAR};
    int m_level;
    int m_curFunLevel;
    int m_curStmtLineNo;
    std::vector<QuaternaryExp> m_qeVec;
    SymbolTable m_table;

    bool isTypeMatch;   //当前类型是否匹配
    int m_targetType;   //目标类型

    std::stack<int> m_typeStack;     //存放结果的类型

    std::vector<int> m_returnJmpIndexVec;   //存放return语句的jump地址

    void interpret(const TreeNode * treeNode);

    void interpretIfStmt(const TreeNode * treeNode);
    void interpretWhileStmt(const TreeNode * treeNode);
    void interpretForStmt(const TreeNode * treeNode);
    void interpretReadStmt(const TreeNode * treeNode);
    void interpretWriteStmt(const TreeNode * treeNode);
    void interpretDecalreStmt(const TreeNode * treeNode);
    void interpretAssignStmt(const TreeNode * treeNode);
    void interpretFunction(const TreeNode * treeNode);
    void interpretReturnStmt(const TreeNode * treeNode);

    std::string interpretFunctionCall(const TreeNode * treeNode);
    std::string interpretExp(const TreeNode * treeNode);
    std::string interpretLogicExp(const TreeNode * treeNode);
    std::string interpretAddtiveExp(const TreeNode * treeNode);
    std::string interpretTermExp(const TreeNode * treeNode);
    std::string interpretVar(const TreeNode * treeNode);
    std::string interpretReference(const TreeNode * treeNode);
    std::string interpretScript(const TreeNode * treeNode);

    //解释declare语句辅助方法
    void checkDeclareIsRight(int declarType);

    //检查赋值类型是否匹配
    void checkAssignIsRight(int varType);

    //解释addtiveExp辅助方法
    std::string transformAddOp(int op);
    std::string interpretAddtiveExpUtil(const std::string & addOp, const std::string & leftNodeStr, const std::string & rightNodeStr, int leftType, int rightType);
    void checkAddtiveExpIsRight(int opType);

    //解释term辅助方法
    std::string transformMulOp(int op);
    std::string interpretTermUtil(const std::string & mulOp, const std::string & leftNodeStr, const std::string & rightNodeStr, int leftType, int rightType);
    void checkTermIsRight();

    //判断字符串是否是常量
    bool isConstant(const std::string & str, int & type);

    //判断数组下标是否合法
    bool isSubscriptRight(const std::string & script);

    //判断赋值是否合法
    bool isAssignRight(int varType, const std::string & valueStr);

    //将function类型转为Value类型
    int transformFunToValue(int funType);
};

CodeGenerater & myCodeGenerater();

#endif // CODEGENERATER_H
