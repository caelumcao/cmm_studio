#include "codegenerater.h"
#include "codegeneraterexception.h"
#include "common.h"
#include "token.h"
#include "value.h"
#include <stdlib.h>
#include <QDebug>

CodeGenerater::CodeGenerater()
{
}

std::vector<QuaternaryExp> CodeGenerater::generateCode(const std::vector<TreeNode *> &treeNodeVec, std::string &errorStr)
{
    programEntry = -1;
    globalVarVec.clear();
    QuaternaryExp::setIndex(-1);
    m_level = 0;
    m_curStmtLineNo = 0;
    m_curFunLevel = 0;
    m_table.clearVec();
    m_qeVec.clear();
    m_returnJmpIndexVec.clear();
    while (!m_typeStack.empty())
        m_typeStack.pop();
    for (int i = 0; i < treeNodeVec.size(); ++i)
    {
        try {
            interpret(treeNodeVec.at(i));
        } catch (CodeGeneraterException & cge) {
            errorStr += ("\n    " + cge.message());
            cge.addToErrorMap();
        }
    }
    return m_qeVec;
}

void CodeGenerater::interpret(const TreeNode *treeNode)
{
    while (true)
    {
        m_curStmtLineNo = treeNode->lineNo();
        if (treeNode->type() != TreeNode::DECLARE_STMT && m_level == 0)
            throw CodeGeneraterException(m_curStmtLineNo, "非声明语句");
        switch (treeNode->type()) {
        case TreeNode::IF_STMT:
            interpretIfStmt(treeNode);
            break;
        case TreeNode::WHILE_STMT:
            interpretWhileStmt(treeNode);
            break;
        case TreeNode::FOR_STMT:
            interpretForStmt(treeNode);
            break;
        case TreeNode::READ_STMT:
            interpretReadStmt(treeNode);
            break;
        case TreeNode::WRITE_STMT:
            interpretWriteStmt(treeNode);
            break;
        case TreeNode::DECLARE_STMT:
            interpretDecalreStmt(treeNode);
            break;
        case TreeNode::ASSIGN_STMT:
            interpretAssignStmt(treeNode);
            break;
        case TreeNode::FUNCALL:
            interpretFunctionCall(treeNode);
            break;
        case TreeNode::RETURN_STMT:
            interpretReturnStmt(treeNode);
            break;
        default:
            break;
        }
        m_table.clearTempVec();
        if (treeNode->next() != NULL)
            treeNode = treeNode->next();
        else
            break;
    }
}

//解释if语句
void CodeGenerater::interpretIfStmt(const TreeNode *treeNode)
{
    //条件跳转 (jmp, 条件, null, 目标)  条件为假时跳转
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, interpretExp(treeNode->left()), "", "", m_curStmtLineNo));
    int ifFalseJmpIndex = QuaternaryExp::index();
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::IN, "", "", "", m_curStmtLineNo));
    m_level++;
    interpret(treeNode->middle());
    m_table.deregisterSymbol(m_level);
    m_level--;
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::OUT, "", "", "", m_curStmtLineNo));
    if (treeNode->right() != NULL)
    {
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, "", "", "", m_curStmtLineNo));
        int outJmpIndex = QuaternaryExp::index();
        m_qeVec.at(ifFalseJmpIndex).setForth(itos(QuaternaryExp::index() + 1));
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::IN, "", "", "", m_curStmtLineNo));
        m_level++;
        interpret(treeNode->right());
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::OUT, "", "", "", m_curStmtLineNo));
        m_table.deregisterSymbol(m_level);
        m_level--;
        m_qeVec.at(outJmpIndex).setForth(itos(QuaternaryExp::index() + 1));
    } else {
        m_qeVec.at(ifFalseJmpIndex).setForth(itos(QuaternaryExp::index() + 1));
    }
}

//解释while语句
void CodeGenerater::interpretWhileStmt(const TreeNode *treeNode)
{
    int jumpLine = QuaternaryExp::index() + 1;
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, interpretExp(treeNode->left()), "", "", m_curStmtLineNo));
    int falseJmpIndex = QuaternaryExp::index();
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::IN, "", "", "", m_curStmtLineNo));
    m_level++;
    interpret(treeNode->middle());
    m_table.deregisterSymbol(m_level);
    m_level--;
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::OUT, "", "", "", m_curStmtLineNo));
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, "", "", itos(jumpLine), m_curStmtLineNo));
    m_qeVec.at(falseJmpIndex).setForth(itos(QuaternaryExp::index() + 1));
}

//解释for语句
void CodeGenerater::interpretForStmt(const TreeNode *treeNode)
{
    interpret(treeNode->left());
    int jumpLine = QuaternaryExp::index() + 1;
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, interpretExp(treeNode->middle()), "", "", m_curStmtLineNo));
    int falseJmpIndex = QuaternaryExp::index();
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::IN, "", "", "", m_curStmtLineNo));
    m_level++;
    interpret(treeNode->middle()->next());
    interpret(treeNode->right());
    m_table.deregisterSymbol(m_level);
    m_level--;
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::OUT, "", "", "", m_curStmtLineNo));
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, "", "", itos(jumpLine), m_curStmtLineNo));
    m_qeVec.at(falseJmpIndex).setForth(itos(QuaternaryExp::index() + 1));
}

//解释read语句
void CodeGenerater::interpretReadStmt(const TreeNode *treeNode)
{
    int varType = m_table.checkSymbolIsDeclared(treeNode->left(), treeNode->left()->lineNo());
    std::string varStr = treeNode->left()->value();
    if (treeNode->left()->left() != NULL)
    {
        varStr += ("[" + interpretScript(treeNode->left()->left()) + "]");
    }
    if (treeNode->dataType() == Token::DERE)
    {
        switch (varType) {
        case 0:
            throw CodeGeneraterException(m_curStmtLineNo, "无法对单值解除引用");
            break;
        case 2:
            throw CodeGeneraterException(m_curStmtLineNo, "read参数类型不匹配");
            break;
        }
        varStr = "*" + varStr;
    } else {
        switch (varType) {
        case 1:
        case -1:
        case 2:
            throw CodeGeneraterException(m_curStmtLineNo, "read参数类型不匹配");
            break;
        }
    }
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::READ, "", "", varStr, m_curStmtLineNo));
}

//解释write语句
void CodeGenerater::interpretWriteStmt(const TreeNode *treeNode)
{
    std::string varStr = interpretExp(treeNode->left());
    m_typeStack.pop();
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::WRITE, "", "", varStr, m_curStmtLineNo));
}

//解释声明语句
void CodeGenerater::interpretDecalreStmt(const TreeNode *treeNode)
{
    if (treeNode->left()->type() == TreeNode::FUNDECLARE)
    {
        interpretFunction(treeNode);
        return;
    }
    int startStmtIndex = QuaternaryExp::index() + 1;
    TreeNode * var = treeNode->left();
    if (var->left() == NULL)     //单值
    {
        std::string value = "";
        if (treeNode->middle() != NULL)
        {
            if (treeNode->middle()->type() == TreeNode::REG)
            {
                value = treeNode->middle()->value();
            } else {
                value = interpretExp(treeNode->middle());
                checkDeclareIsRight(var->dataType());
            }
        }
        if (var->dataType() == Token::INT)
        {
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::INT, value, "", var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::SINGLE_INT, var->lineNo(), m_level);
            m_table.registerSymbol(symbol);
        } else if (var->dataType() == Token::REAL) {
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::REAL, value, "", var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::SINGLE_REAL, var->lineNo(), m_level);
            m_table.registerSymbol(symbol);
        } else if (var->dataType() == Token::CHAR) {
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::CHAR, value, "", var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::SINGLE_CHAR, var->lineNo(), m_level);
            m_table.registerSymbol(symbol);
        } else if (var->dataType() == Token::INT_POINT) {
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::INT_POINT, value, "", var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::POINT_INT, var->lineNo(), m_level, -1);
            m_table.registerSymbol(symbol);
        } else if (var->dataType() == Token::REAL_POINT) {
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::REAL_POINT, value, "", var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::POINT_REAL, var->lineNo(), m_level, -1);
            m_table.registerSymbol(symbol);
        } else if (var->dataType() == Token::CHAR_POINT) {
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::CHAR_POINT, value, "", var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::POINT_CHAR, var->lineNo(), m_level, -1);
            m_table.registerSymbol(symbol);
        }
    } else {
        std::string len = "0";
        int elementNum = 0;
        if (var->left()->type() != TreeNode::NUL)
        {
            len = interpretExp(var->left());
            elementNum = atoi(len.c_str());
            if (!isSubscriptRight(len) || len == "0")   //检查下标内容
                throw CodeGeneraterException(m_curStmtLineNo, "数组大小必须是整形常量并且大于0");
        }

        if (var->dataType() == Token::INT)
        {
            if (var->left()->type() == TreeNode::NUL)
            {
                m_qeVec.push_back(QuaternaryExp(QuaternaryExp::INT_POINT, "", "", var->value(), m_curStmtLineNo));
                Symbol symbol(var->value(), Symbol::POINT_INT, var->lineNo(), m_level, -1);
                m_table.registerSymbol(symbol);
            } else {
                m_qeVec.push_back(QuaternaryExp(QuaternaryExp::INT, "", len, var->value(), m_curStmtLineNo));
                Symbol symbol(var->value(), Symbol::ARRAY_INT, var->lineNo(), m_level, elementNum);
                m_table.registerSymbol(symbol);
            }
        } else if (var->dataType() == Token::REAL){
            if (var->left()->type() == TreeNode::NUL)
            {
                m_qeVec.push_back(QuaternaryExp(QuaternaryExp::REAL_POINT, "", "", var->value(), m_curStmtLineNo));
                Symbol symbol(var->value(), Symbol::POINT_REAL, var->lineNo(), m_level, -1);
                m_table.registerSymbol(symbol);
            } else {
                m_qeVec.push_back(QuaternaryExp(QuaternaryExp::REAL, "", len, var->value(), m_curStmtLineNo));
                Symbol symbol(var->value(), Symbol::ARRAY_REAL, var->lineNo(), m_level, elementNum);
                m_table.registerSymbol(symbol);
            }
        } else if (var->dataType() == Token::CHAR){
            if (var->left()->type() == TreeNode::NUL)
            {
                m_qeVec.push_back(QuaternaryExp(QuaternaryExp::CHAR_POINT, "", "", var->value(), m_curStmtLineNo));
                Symbol symbol(var->value(), Symbol::POINT_CHAR, var->lineNo(), m_level, -1);
                m_table.registerSymbol(symbol);
            } else {
                m_qeVec.push_back(QuaternaryExp(QuaternaryExp::CHAR, "", len, var->value(), m_curStmtLineNo));
                Symbol symbol(var->value(), Symbol::ARRAY_CHAR, var->lineNo(), m_level, elementNum);
                m_table.registerSymbol(symbol);
            }
        } else if (var->dataType() == Token::INT_POINT){
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::INT_POINT, "", len, var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::ARRAY_POINT_INT, var->lineNo(), m_level, elementNum);
            m_table.registerSymbol(symbol);
        } else if (var->dataType() == Token::REAL_POINT){
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::REAL_POINT, "", len, var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::ARRAY_POINT_REAL, var->lineNo(), m_level, elementNum);
            m_table.registerSymbol(symbol);
        }else if (var->dataType() == Token::CHAR_POINT){
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::CHAR_POINT, "", len, var->value(), m_curStmtLineNo));
            Symbol symbol(var->value(), Symbol::ARRAY_POINT_CHAR, var->lineNo(), m_level, elementNum);
            m_table.registerSymbol(symbol);
        }
        TreeNode * valueNode = treeNode->middle();
        if (valueNode != NULL)
        {
            int initNum = 0;
            if (valueNode->type() == TreeNode::EXP || valueNode->type() == TreeNode::FACTOR)
            {
                while (valueNode != NULL)
                {
                    initNum++;
                    if (initNum > atoi(len.c_str()))
                        throw CodeGeneraterException(m_curStmtLineNo, "数组初始值设定过多");
                    while (!m_typeStack.empty())
                        m_typeStack.pop();
                    std::string value = interpretExp(valueNode);
                    checkDeclareIsRight(var->dataType());
                    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ASSIGN, value, "", var->value() + "[" + itos(initNum - 1) + "]", m_curStmtLineNo));
                    valueNode = valueNode->next();
                }
            } else if (valueNode->type() == TreeNode::LITERAL) {
                for (int i = 1; i < valueNode->value().size() - 1; ++i)
                {
                    initNum++;
                    if (initNum > atoi(len.c_str()) - 1)
                        throw CodeGeneraterException(m_curStmtLineNo, "数组初始值设定过多");
                    std::string value = "'" + ctos(valueNode->value().at(i)) + "'";
                    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ASSIGN, value, "", var->value() + "[" + itos(initNum - 1) + "]", m_curStmtLineNo));
                }
                m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ASSIGN, "''", "", var->value() + "[" + itos(initNum) + "]", m_curStmtLineNo));
            } else if (valueNode->type() == TreeNode::REG) {
                m_qeVec.back().setSecond(valueNode->value());
            }
        }
    }
    if (m_level == 0)
    {
        int endStmtIndex = QuaternaryExp::index();
        for (int i = startStmtIndex; i <= endStmtIndex; ++i)
            globalVarVec.push_back(i);
    }

}

//解释赋值语句
void CodeGenerater::interpretAssignStmt(const TreeNode *treeNode)
{
    int varType = m_table.checkSymbolIsDeclared(treeNode->left(), treeNode->left()->lineNo());
    int targetType = -1;
    bool isArray = false;
    std::string varStr = treeNode->left()->value();
    if (treeNode->left()->left() != NULL)
    {
        isArray = true;
        varStr += ("[" + interpretScript(treeNode->left()->left()) + "]");
    }
    if (treeNode->dataType() == Token::DERE)    // *var = exp
    {
        switch (varType) {
        case 0:
            throw CodeGeneraterException(m_curStmtLineNo, "无法对单值解除引用");
            break;
        case 1:
        case -1:
        case 3:
            targetType = Value::SINGLE_VALUE;
            break;
        case 2:
            targetType = Value::ADDR;
            break;
        }
        varStr = "*" + varStr;
    } else {
        switch (varType) {
        case 0:
            targetType = Value::SINGLE_VALUE;
            break;
        case 1:
        case 3:
            throw CodeGeneraterException(m_curStmtLineNo, "无法对数组进行赋值");
            break;
        case -1:
            targetType = Value::ADDR;
            break;
        case 2:
            throw CodeGeneraterException(m_curStmtLineNo, "无法对数组指针进行赋值");
            break;
            break;
        }
    }
    std::string value = interpretExp(treeNode->middle());
    checkAssignIsRight(targetType);
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ASSIGN, value, "", varStr, m_curStmtLineNo));
}

//解释函数
void CodeGenerater::interpretFunction(const TreeNode *treeNode)
{
    if (m_level > 0)
        throw CodeGeneraterException(m_curStmtLineNo, "无法在函数内部定义函数");
    TreeNode * var = treeNode->left();
    int symbolType;
    std::string quaternaryExpType;
    switch (var->dataType()) {
    case Token::INT:
        symbolType = FunSymbol::INT;
        quaternaryExpType = QuaternaryExp::INT;
        break;
    case Token::REAL:
        symbolType = FunSymbol::REAL;
        quaternaryExpType = QuaternaryExp::REAL;
        break;
    case Token::CHAR:
        symbolType = FunSymbol::CHAR;
        quaternaryExpType = QuaternaryExp::CHAR;
        break;
    case Token::INT_POINT:
        symbolType = FunSymbol::INT_POINT;
        quaternaryExpType = QuaternaryExp::INT_POINT;
        break;
    case Token::REAL_POINT:
        symbolType = FunSymbol::REAL_POINT;
        quaternaryExpType = QuaternaryExp::REAL_POINT;
        break;
    case Token::CHAR_POINT:
        symbolType = FunSymbol::CHAR_POINT;
        quaternaryExpType = QuaternaryExp::CHAR_POINT;
        break;
    case Token::VOI:
        symbolType = FunSymbol::VOID;
        quaternaryExpType = QuaternaryExp::VOID;
        break;
    default:
        break;
    }
    if (var->value() == "main")
    {
        if (symbolType != FunSymbol::VOID)
            throw CodeGeneraterException(m_curStmtLineNo, "main函数类型应为void");
        programEntry = QuaternaryExp::index() + 1;
    }
    m_qeVec.push_back(QuaternaryExp(quaternaryExpType, "", "", var->value() + "()", m_curStmtLineNo));
    FunSymbol funSymbol(var->value(), var->lineNo(), symbolType, QuaternaryExp::index() + 1);
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::IN, "", "", "", m_curStmtLineNo));
    m_level++;
    m_curFunLevel = m_level;
    TreeNode * argNode = var->left();
    while (argNode != NULL)
    {
        if (var->value() == "main")
            throw CodeGeneraterException(m_curStmtLineNo, "无法对main函数赋参数");
        interpretDecalreStmt(argNode);
        funSymbol.m_argTypeVec.push_back(m_table.getLastSymbolType());
        argNode = argNode->next();
    }
    m_table.registerFunSymbol(funSymbol);
    interpret(treeNode->middle());

    m_table.deregisterSymbol(m_level);
    m_level--;
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::OUT, "", "", "", m_curStmtLineNo));
    for (int i = 0; i < m_returnJmpIndexVec.size(); ++i)
         m_qeVec.at(m_returnJmpIndexVec.at(i)).setForth(itos(QuaternaryExp::index()));
    m_returnJmpIndexVec.clear();
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, "", "", "@ebp", m_curStmtLineNo));
}

//解释return语句
void CodeGenerater::interpretReturnStmt(const TreeNode *treeNode)
{
    if (m_level == 0)
        throw CodeGeneraterException(m_curStmtLineNo, "应输入声明语句");
    if (m_table.getCurFunSymbolType() == FunSymbol::VOID)
        throw CodeGeneraterException(m_curStmtLineNo, "函数返回类型为void,无法使用return");
    std::string result = interpretExp(treeNode->left());
    int valueType = transformFunToValue(m_table.getCurFunSymbolType());
    checkAssignIsRight(valueType);
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ASSIGN, result, "", "@ebp-1", m_curStmtLineNo));
    for (int i = m_level; i > m_curFunLevel; --i)
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::OUT, "", "", "", m_curStmtLineNo));
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, "", "", "", m_curStmtLineNo));
    m_returnJmpIndexVec.push_back(QuaternaryExp::index());
}

//解释函数调用
std::string CodeGenerater::interpretFunctionCall(const TreeNode *treeNode)
{
    if (m_level == 0)
        throw CodeGeneraterException(m_curStmtLineNo, "应输入声明语句");
    FunSymbol funSymbol = m_table.getFunSymbol(treeNode->value(), m_curStmtLineNo);
    if (funSymbol.name() == "main")
        throw CodeGeneraterException(m_curStmtLineNo, "禁止调用main函数");
    std::stack<std::string> argStack;
    TreeNode * argNode = treeNode->left();
    int index = 0;
    while (argNode != NULL)
    {
        if (index >= funSymbol.m_argTypeVec.size())
            throw CodeGeneraterException(m_curStmtLineNo, "函数参数过多");
        int targetType = -1;
        switch (funSymbol.m_argTypeVec.at(index))
        {
        case Symbol::SINGLE_CHAR:
        case Symbol::SINGLE_INT:
        case Symbol::SINGLE_REAL:
            targetType = Value::SINGLE_VALUE;
            break;
        default:
            targetType = Value::ADDR;
            break;
        }
        std::string argValue = interpretExp(argNode);
        checkAssignIsRight(targetType);
        argStack.push(argValue);
        index++;
        argNode = argNode->next();
    }
    if (index < funSymbol.m_argTypeVec.size())
        throw CodeGeneraterException(m_curStmtLineNo, "函数参数过少");
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ASSIGN, "0", "", "@esp", m_curStmtLineNo));
    while (!argStack.empty())
    {
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ASSIGN, argStack.top(), "", "@esp", m_curStmtLineNo));
        argStack.pop();
    }
    int assignAddrIndex = QuaternaryExp::index() + 1;
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ASSIGN, "", "", "@esp", m_curStmtLineNo));
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::CALL, "", "", funSymbol.name(), m_curStmtLineNo));
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::JMP, "", "", itos(funSymbol.stmtIndex()), m_curStmtLineNo));
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::CALLFH, "", "", "", m_curStmtLineNo));
    m_qeVec.at(assignAddrIndex).setSecond("$" + itos(0x10000000 + 4 * QuaternaryExp::index()));
    //设置函数返回类型
    int valueType = transformFunToValue(funSymbol.returnType());
    m_typeStack.push(valueType);
    return "@esp-1";
}

//解释表达式
std::string CodeGenerater::interpretExp(const TreeNode *treeNode)
{
    if (treeNode->type() == TreeNode::EXP)
    {
        switch (treeNode->dataType())
        {
        case Token::LOGIC_EXP:
            return interpretLogicExp(treeNode);
        case Token::ADDTIVE_EXP:
            return interpretAddtiveExp(treeNode);
        case Token::TERM_EXP:
            return interpretTermExp(treeNode);
        default:
            throw CodeGeneraterException(treeNode->lineNo(), "复合表达式非法");
        }
    } else if (treeNode->type() == TreeNode::FACTOR) {
        if (treeNode->dataType() == Token::MINUS)
        {
            std::string nodeStr = interpretExp(treeNode->left());
            int type;
            if (isConstant(nodeStr, type))
                return ("-" + nodeStr);
            std::string temp = m_table.getNewTempSymbolName();
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::MINUS, "", nodeStr, temp, m_curStmtLineNo));
            return temp;
        } else if (treeNode->dataType() == Token::ADDR) {   //要考虑&arr
            std::string nodeStr = interpretExp(treeNode->left());
            std::string temp = m_table.getNewTempSymbolName();
            m_qeVec.push_back(QuaternaryExp(QuaternaryExp::ADDR, "", nodeStr, temp, m_curStmtLineNo));
            m_typeStack.push(Value::ADDR);
            return temp;
        } else if (treeNode->dataType() == Token::MUL) {    //要考虑*arr[3]
            return interpretReference(treeNode);
        } else {
            return interpretExp(treeNode->left());
        }
    } else if (treeNode->type() == TreeNode::VAR) {
        return interpretVar(treeNode);
    } else if (treeNode->type() == TreeNode::LITERAL) {
        if (treeNode->dataType() == Token::STR_VALUE)
            m_typeStack.push(Value::STR_VALUE);
        else
            m_typeStack.push(Value::SINGLE_VALUE);
        return treeNode->value();
    } else if (treeNode->type() == TreeNode::FUNCALL) {
        return interpretFunctionCall(treeNode);
    }
    throw CodeGeneraterException(treeNode->lineNo(), "表达式非法");
}

//解释逻辑表达式
std::string CodeGenerater::interpretLogicExp(const TreeNode *treeNode)
{
    int leftType, rightType;
    bool isAllConstant = false;
    std::string temp = "";
    std::string leftNodeStr = interpretExp(treeNode->left());
    std::string rightNodeStr = interpretExp(treeNode->right());
    int r_valueType = m_typeStack.top();
    m_typeStack.pop();
    int l_valueType = m_typeStack.top();
    m_typeStack.pop();
    int opType = treeNode->middle()->dataType();
    if (l_valueType != r_valueType)
        throw CodeGeneraterException(m_curStmtLineNo, "类型不同，无法进行逻辑运算");
    if (l_valueType == Value::STR_VALUE && r_valueType == Value::STR_VALUE && opType != Token::EQ && opType != Token::NEQ)
        throw CodeGeneraterException(m_curStmtLineNo, "无法对字符串进行" + Token::typeToString(opType) + "运算");
    m_typeStack.push(Value::SINGLE_VALUE);
    if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
        isAllConstant = true;
    if (!isAllConstant)
        temp = m_table.getNewTempSymbolName();
    switch (opType)
    {
    case Token::GT:
        if (isAllConstant)
            return Value(leftNodeStr, leftType) > Value(rightNodeStr, rightType);
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::GT, leftNodeStr, rightNodeStr, temp, m_curStmtLineNo));
        break;
    case Token::GET:
        if (isAllConstant)
            return Value(leftNodeStr, leftType) >= Value(rightNodeStr, rightType);
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::GET, leftNodeStr, rightNodeStr, temp, m_curStmtLineNo));
        break;
    case Token::LT:
        if (isAllConstant)
            return Value(leftNodeStr, leftType) < Value(rightNodeStr, rightType);
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::LT, leftNodeStr, rightNodeStr, temp, m_curStmtLineNo));
        break;
    case Token::LET:
        if (isAllConstant)
            return Value(leftNodeStr, leftType) <= Value(rightNodeStr, rightType);
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::LET, leftNodeStr, rightNodeStr, temp, m_curStmtLineNo));
        break;
    case Token::EQ:
        if (isAllConstant)
            return Value(leftNodeStr, leftType) == Value(rightNodeStr, rightType);
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::EQ, leftNodeStr, rightNodeStr, temp, m_curStmtLineNo));
        break;
    case Token::NEQ:
        if (isAllConstant)
            return nequal(Value(leftNodeStr, leftType), Value(rightNodeStr, rightType));
        m_qeVec.push_back(QuaternaryExp(QuaternaryExp::NEQ, leftNodeStr, rightNodeStr, temp, m_curStmtLineNo));
        break;
    default:
        throw CodeGeneraterException(treeNode->lineNo(), "逻辑表达非法");
    }
    return temp;
}

//解释多项式
std::string CodeGenerater::interpretAddtiveExp(const TreeNode *treeNode)
{
    int leftType, rightType;
    std::string leftNodeStr, rightNodeStr, temp_1;
    std::string addOp = transformAddOp(treeNode->middle()->dataType());
    if (treeNode->right()->type() == TreeNode::FACTOR || treeNode->right()->dataType() == Token::TERM_EXP)
    {
        leftNodeStr = interpretExp(treeNode->left());
        rightNodeStr = interpretExp(treeNode->right());
        checkAddtiveExpIsRight(treeNode->middle()->dataType());
        if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
        {
            temp_1 = interpretAddtiveExpUtil(addOp, leftNodeStr, rightNodeStr, leftType, rightType);
        } else {
            temp_1 = m_table.getNewTempSymbolName();
            m_qeVec.push_back(QuaternaryExp(addOp, leftNodeStr, rightNodeStr, temp_1, m_curStmtLineNo));
        }
    } else {
        leftNodeStr = interpretExp(treeNode->left());
        rightNodeStr = interpretExp(treeNode->right()->left());
        checkAddtiveExpIsRight(treeNode->middle()->dataType());
        if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
        {
            temp_1 = interpretAddtiveExpUtil(addOp, leftNodeStr, rightNodeStr, leftType, rightType);
        } else {
            temp_1 = m_table.getNewTempSymbolName();
            m_qeVec.push_back(QuaternaryExp(addOp, leftNodeStr, rightNodeStr, temp_1, m_curStmtLineNo));
        }
        treeNode = treeNode->right();
        std::string temp_2 = "";
        while (treeNode->right() != NULL && treeNode->right()->type() != TreeNode::FACTOR && treeNode->right()->dataType() != Token::TERM_EXP)
        {
            addOp = transformAddOp(treeNode->middle()->dataType());
            leftNodeStr = temp_1;
            rightNodeStr = interpretExp(treeNode->right()->left());
            checkAddtiveExpIsRight(treeNode->middle()->dataType());
            if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
            {
                temp_2 = interpretAddtiveExpUtil(addOp, leftNodeStr, rightNodeStr, leftType, rightType);
            } else {
                temp_2 = m_table.getNewTempSymbolName();
                m_qeVec.push_back(QuaternaryExp(addOp, leftNodeStr, rightNodeStr, temp_2, m_curStmtLineNo));
            }
            treeNode = treeNode->right();
            temp_1 = temp_2;
        }
        addOp = transformAddOp(treeNode->middle()->dataType());
        leftNodeStr = temp_1;
        rightNodeStr = interpretExp(treeNode->right());
        checkAddtiveExpIsRight(treeNode->middle()->dataType());
        if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
        {
            temp_2 = interpretAddtiveExpUtil(addOp, leftNodeStr, rightNodeStr, leftType, rightType);
        } else {
            temp_2 = m_table.getNewTempSymbolName();
            m_qeVec.push_back(QuaternaryExp(addOp, leftNodeStr, rightNodeStr, temp_2, m_curStmtLineNo));
        }
        temp_1 = temp_2;
    }
    return temp_1;
}

//解释项
std::string CodeGenerater::interpretTermExp(const TreeNode *treeNode)
{
    int leftType, rightType;
    std::string leftNodeStr, rightNodeStr, temp_1;
    std::string mulOp = transformMulOp(treeNode->middle()->dataType());
    if (treeNode->right()->type() == TreeNode::FACTOR)
    {
        leftNodeStr = interpretExp(treeNode->left());
        rightNodeStr = interpretExp(treeNode->right());
        checkTermIsRight();
        if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
        {
           temp_1 = interpretTermUtil(mulOp, leftNodeStr, rightNodeStr, leftType, rightType);
        } else {
            temp_1 = m_table.getNewTempSymbolName();
            m_qeVec.push_back(QuaternaryExp(mulOp, leftNodeStr, rightNodeStr, temp_1, m_curStmtLineNo));
        }
    } else {
        leftNodeStr = interpretExp(treeNode->left());
        rightNodeStr = interpretExp(treeNode->right()->left());
        checkTermIsRight();
        if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
        {
            temp_1 = interpretTermUtil(mulOp, leftNodeStr, rightNodeStr, leftType, rightType);
        } else {
            temp_1 = m_table.getNewTempSymbolName();
            m_qeVec.push_back(QuaternaryExp(mulOp, leftNodeStr, rightNodeStr, temp_1, m_curStmtLineNo));
        }
        treeNode = treeNode->right();
        std::string temp_2 = "";
        while (treeNode->right() != NULL && treeNode->right()->type() != TreeNode::FACTOR)
        {
            mulOp = transformMulOp(treeNode->middle()->dataType());
            leftNodeStr = temp_1;
            rightNodeStr = interpretExp(treeNode->right()->left());
            checkTermIsRight();
            if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
            {
                temp_2 = interpretTermUtil(mulOp, leftNodeStr, rightNodeStr, leftType, rightType);
            } else {
                temp_2 = m_table.getNewTempSymbolName();
                m_qeVec.push_back(QuaternaryExp(mulOp, leftNodeStr, rightNodeStr, temp_2, m_curStmtLineNo));
            }
            treeNode = treeNode->right();
            temp_1 = temp_2;
        }
        mulOp = transformMulOp(treeNode->middle()->dataType());
        leftNodeStr = temp_1;
        rightNodeStr = interpretExp(treeNode->right());
        checkTermIsRight();
        if (isConstant(leftNodeStr, leftType) && isConstant(rightNodeStr, rightType))
        {
            temp_2 = interpretTermUtil(mulOp, leftNodeStr, rightNodeStr, leftType, rightType);
        } else {
            temp_2 = m_table.getNewTempSymbolName();
            m_qeVec.push_back(QuaternaryExp(mulOp, leftNodeStr, rightNodeStr, temp_2, m_curStmtLineNo));
        }
        temp_1 = temp_2;
    }
    return temp_1;
}

//解释变量
std::string CodeGenerater::interpretVar(const TreeNode *treeNode)
{
    int result = m_table.checkSymbolIsDeclared(treeNode, treeNode->lineNo());
    std::string varStr = treeNode->value();
    if (treeNode->left() != NULL)
    {
        varStr += ("[" + interpretScript(treeNode->left()) + "]"); //这里要判断
    }
    if (result == 0)
        m_typeStack.push(Value::SINGLE_VALUE);
    else
        m_typeStack.push(Value::ADDR);
    return varStr;
}

//解释解除引用(*var)
std::string CodeGenerater::interpretReference(const TreeNode *treeNode) //参数为*var
{
    int varType = m_table.checkSymbolIsDeclared(treeNode->left(), treeNode->lineNo());
    std::string varStr = treeNode->left()->value();
    if (treeNode->left()->left() != NULL)
        varStr += ("[" + interpretScript(treeNode->left()->left()) + "]");
    switch (varType) {
    case 0:
        throw CodeGeneraterException(m_curStmtLineNo, "无法对单值解除引用");
        break;
    case 1:
    case -1:
        m_typeStack.push(Value::SINGLE_VALUE);
        break;
    case 2:
        m_typeStack.push(Value::ADDR);
        break;
    }
    std::string temp = m_table.getNewTempSymbolName();
    m_qeVec.push_back(QuaternaryExp(QuaternaryExp::MUL, "", varStr, temp, m_curStmtLineNo));
    return temp;
}

//解释数组下标
std::string CodeGenerater::interpretScript(const TreeNode *treeNode)
{
    std::string script = interpretExp(treeNode); //这里要判断
    int scriptType = m_typeStack.top();
    m_typeStack.pop();
    if (scriptType != Value::SINGLE_VALUE)
        throw CodeGeneraterException(m_curStmtLineNo, "数组下标类型错误");  //!!!!!!!!没有确定到int类型
    return script;
}

//解释声明语句辅助方法
void CodeGenerater::checkDeclareIsRight(int declarType)
{
    int valueType = m_typeStack.top();
    m_typeStack.pop();
    switch (declarType) {
    case Token::INT:
    case Token::REAL:
    case Token::CHAR:
        if (valueType != Value::SINGLE_VALUE)
            throw CodeGeneraterException(m_curStmtLineNo, "类型不匹配");
        break;
    case Token::INT_POINT:
    case Token::REAL_POINT:
        if (valueType != Value::ADDR)
            throw CodeGeneraterException(m_curStmtLineNo, "类型不匹配");
        break;
    case Token::CHAR_POINT:
        if (valueType != Value::ADDR && valueType != Value::STR_VALUE)
            throw CodeGeneraterException(m_curStmtLineNo, "类型不匹配");
        break;
    default:
        break;
    }
}

//检查赋值类型是否匹配
void CodeGenerater::checkAssignIsRight(int varType)
{
    int valueType = m_typeStack.top();
    m_typeStack.pop();
    if (varType == Value::SINGLE_VALUE)
    {
        switch (valueType) {
        case Value::ADDR:
            throw CodeGeneraterException(m_curStmtLineNo, "无法将数组赋值给单值");
        case Value::STR_VALUE:
            throw CodeGeneraterException(m_curStmtLineNo, "无法将字符串赋值给单值");
        case Value::VOID:
            throw CodeGeneraterException(m_curStmtLineNo, "该函数没有返回值，无法进行赋值");
        default:
            break;
        }
    } else if (varType == Value::ADDR) {
        switch (valueType) {
        case Value::SINGLE_VALUE:
            throw CodeGeneraterException(m_curStmtLineNo, "无法将单值赋值给地址");
        case Value::VOID:
            throw CodeGeneraterException(m_curStmtLineNo, "该函数没有返回值，无法进行赋值");
        default:
            break;
        }
    }
}

std::string CodeGenerater::transformAddOp(int op)
{
    if (op == Token::PLUS)
        return QuaternaryExp::PLUS;
    else
        return QuaternaryExp::MINUS;
}

std::string CodeGenerater::interpretAddtiveExpUtil(const std::string &addOp, const std::string &leftNodeStr, const std::string &rightNodeStr, int leftType, int rightType)
{
    std::string result;
    if (addOp == QuaternaryExp::PLUS)
        result = Value(leftNodeStr, leftType) + Value(rightNodeStr, rightType);
    else
        result = Value(leftNodeStr, leftType) - Value(rightNodeStr, rightType);
    return result;
}

void CodeGenerater::checkAddtiveExpIsRight(int opType)
{
    int r_valueType = m_typeStack.top();
    m_typeStack.pop();
    int l_valueType = m_typeStack.top();
    m_typeStack.pop();
    if (l_valueType == Value::SINGLE_VALUE && r_valueType == Value::SINGLE_VALUE)
    {
        m_typeStack.push(Value::SINGLE_VALUE);
    } else if (l_valueType == Value::ADDR && r_valueType == Value::SINGLE_VALUE) {
        m_typeStack.push(Value::ADDR);
    } else if (l_valueType == Value::SINGLE_VALUE && r_valueType == Value::ADDR) {
        if (opType == Token::PLUS)
            m_typeStack.push(Value::ADDR);
        else
            throw CodeGeneraterException(m_curStmtLineNo, "减数不能是地址类型");
    } else {
        throw CodeGeneraterException(m_curStmtLineNo, "无法对该类型的值进行加减法运算");
    }
}

std::string CodeGenerater::transformMulOp(int op)
{
    if (op == Token::MUL)
        return QuaternaryExp::MUL;
    else
        return QuaternaryExp::DIV;
}

std::string CodeGenerater::interpretTermUtil(const std::string &mulOp, const std::string &leftNodeStr, const std::string &rightNodeStr, int leftType, int rightType)
{
    std::string result;
    if (mulOp == QuaternaryExp::MUL) {
        result =  Value(leftNodeStr, leftType) * Value(rightNodeStr, rightType);
    } else {
        Value::callfrom = m_curStmtLineNo;
        result =  Value(leftNodeStr, leftType) / Value(rightNodeStr, rightType);
    }
    return result;
}

void CodeGenerater::checkTermIsRight()
{
    int r_valueType = m_typeStack.top();
    m_typeStack.pop();
    int l_valueType = m_typeStack.top();
    m_typeStack.pop();
    if (l_valueType == Value::SINGLE_VALUE && r_valueType == Value::SINGLE_VALUE)
        m_typeStack.push(Value::SINGLE_VALUE);
    else
        throw CodeGeneraterException(m_curStmtLineNo, "无法对该类型的值进行乘除法运算");
}

//检查是否是常量
bool CodeGenerater::isConstant(const std::string &str, int &type)
{
    type = Value::INT_VALUE;
    for (int i = 0; i < str.size(); ++i)
    {
        char ch = str.at(i);
        if (isalpha(ch) || ch == '_' || ch == '@')
            return false;
        if (ch == '.')
            type = Value::REAL_VALUE;
    }
    return true;
}

//检查数组下标
bool CodeGenerater::isSubscriptRight(const std::string &script)
{
    for (int i = 0; i < script.size(); ++i)
        if (!isdigit(script.at(i)))
            return false;
    return true;
}

//检查赋值
bool CodeGenerater::isAssignRight(int varType, const std::string &valueStr)
{
//    switch (varType) {
//    case Token::INT:
//    case Token::

//        break;
//    default:
//        break;
    //    }
}

//将function类型转为Value类型
int CodeGenerater::transformFunToValue(int funType)
{
    switch (funType) {
    case FunSymbol::VOID:
        return Value::VOID;
    case FunSymbol::INT:
    case FunSymbol::REAL:
    case FunSymbol::CHAR:
        return Value::SINGLE_VALUE;
    default:
        return Value::ADDR;
    }
}


CodeGenerater & myCodeGenerater()
{
    static CodeGenerater cg;
    return cg;
}
