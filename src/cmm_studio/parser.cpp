#include "parser.h"
#include "parserexception.h"
#include "common.h"
#include <QDebug>

Parser::Parser()
{

}

std::vector<TreeNode *> Parser::syntacticAnalyse(std::vector<Token> tokenVec, std::vector<QueueVec> & qvvec, std::string & errorStr)
{
    index = 0;
    m_tokenVec = tokenVec;
    m_treeNodeVec.clear();
    m_errorNode = NULL;
    try {
        while (index < m_tokenVec.size())
        {
            TreeNode * node = parseStmt(0);
            if (node == NULL)
                continue;
            m_treeNodeVec.push_back(node);
            QueueVec tempVec = m_queueVec;
            qvvec.push_back(tempVec);
            m_queueVec.clear();
        }
    } catch (ParserException & pe) {
        errorStr = pe.message();
        pe.addToErrorMap();
        if (m_errorNode != NULL)
            delete m_errorNode;
    }
    return m_treeNodeVec;
}

TreeNode * Parser::parseStmt(int level)
{
    int nextTokenType = getNextTokenType();
    switch (nextTokenType)
    {
    case Token::LCOM:
        index += 2;
        return parseStmt(level);
    case Token::SCOM:
        index++;
        return parseStmt(level);
    case Token::IF:
        return parseIfStmt(level);
    case Token::WHILE:
        return parseWhileStmt(level);
    case Token::FOR:
        return parseForStmt(level);
    case Token::READ:
        return parseReadStmt(level);
    case Token::WRITE:
        return parseWriteStmt(level);
    case Token::INT:
    case Token::REAL:
    case Token::CHAR:
    case Token::VOI:
        return parseDeclareStmt(level);
    case Token::LBRACE:
        return parseStmtBlock(level);
    case Token::ID:
        if (getNextNextTokenType() == Token::LPS)
            return parseFunctionCall(level);
        else
            return parseAssignStmt(level);
    case Token::MUL:
        return parseAssignStmt(level);
    case Token::RETURN:
        return parseReturn(level);
    default:
        throw ParserException("statement", Token(nextTokenType, getNextTokenLineNo()));
    }
}

//if语句
TreeNode * Parser::parseIfStmt(int level)
{
    TreeNode * node = new TreeNode(TreeNode::IF_STMT);
    if (level == 0)
        m_errorNode = node;
    insertIntoQueueVec("ifStmt", 5, level);
    consumeNextToken(Token::IF, level + 1);
    node->setLineNo(m_curToken.lineNoValue());
    consumeNextToken(Token::LPS, level + 1);
    node->setLeft(parseExp(level + 1));
    consumeNextToken(Token::RPS, level + 1);
    node->setMiddle(parseStmt(level + 1));
    if (getNextTokenType() == Token::ELSE)
    {
        consumeNextToken(Token::ELSE, level + 1);
        node->setRight(parseStmt(level + 1));
        m_queueVec.at(level).back().childNum = 7;
    }
    return node;
}

//while语句
TreeNode * Parser::parseWhileStmt(int level)
{
    TreeNode * node = new TreeNode(TreeNode::WHILE_STMT);
    if (level == 0)
        m_errorNode = node;
    insertIntoQueueVec("whileStmt", 5, level);
    consumeNextToken(Token::WHILE, level + 1);
    node->setLineNo(m_curToken.lineNoValue());
    consumeNextToken(Token::LPS, level + 1);
    node->setLeft(parseExp(level + 1));
    consumeNextToken(Token::RPS, level + 1);
    node->setMiddle(parseStmt(level + 1));
    return node;
}

//for语句
TreeNode *Parser::parseForStmt(int level)
{
    TreeNode * node = new TreeNode(TreeNode::FOR_STMT);
    if (level == 0)
        m_errorNode = node;
    insertIntoQueueVec("forStmt", 8, level);
    consumeNextToken(Token::FOR, level + 1);
    node->setLineNo(m_curToken.lineNoValue());
    consumeNextToken(Token::LPS, level + 1);
    node->setLeft(parseAssignStmt(level + 1));
    node->setMiddle(parseExp(level + 1));
    consumeNextToken(Token::SEMI, level + 1);
    node->setRight(parseAssignStmt(level + 1, true));
    consumeNextToken(Token::RPS, level + 1);
    node->middle()->setNext(parseStmt(level + 1));
    return node;
}

//read语句
TreeNode * Parser::parseReadStmt(int level)
{
    TreeNode * node = new TreeNode(TreeNode::READ_STMT);
    if (level == 0)
        m_errorNode = node;
    insertIntoQueueVec("readStmt", 5, level);
    consumeNextToken(Token::READ, level + 1);
    node->setLineNo(m_curToken.lineNoValue());
    consumeNextToken(Token::LPS, level + 1);
    if (getNextTokenType() == Token::MUL)
    {
        consumeNextToken(Token::MUL, level + 1);
        node->setDataType(Token::DERE);
        m_queueVec.at(level).back().childNum = 6;
    }
    node->setLeft(parseVariableName(level + 1));
    consumeNextToken(Token::RPS, level + 1);
    consumeNextToken(Token::SEMI, level + 1);
    return node;
}

//write语句
TreeNode * Parser::parseWriteStmt(int level)
{
    TreeNode * node = new TreeNode(TreeNode::WRITE_STMT);
    if (level == 0)
        m_errorNode = node;
    insertIntoQueueVec("writeStmt", 5, level);
    consumeNextToken(Token::WRITE, level + 1);
    node->setLineNo(m_curToken.lineNoValue());
    consumeNextToken(Token::LPS, level + 1);
    node->setLeft(parseExp(level + 1));
    consumeNextToken(Token::RPS, level + 1);
    consumeNextToken(Token::SEMI, level + 1);
    return node;
}

//声明语句
TreeNode * Parser::parseDeclareStmt(int level, bool isParseFun)
{
    TreeNode * node = new TreeNode(TreeNode::DECLARE_STMT);
    if (level == 0)
        m_errorNode = node;
    insertIntoQueueVec("declareStmt", 0, level);
    int childs = 3;
    TreeNode * temp = parseVariableType(level + 1);     //注意该指针的释放
    node->setLineNo(m_curToken.lineNoValue());
    int dataType = m_curToken.typeValue();
    bool isPoint = false;
    if (getNextTokenType() == Token::MUL)
    {
        dataType = getPointType(dataType);
        consumeNextToken(Token::MUL, level + 1);
        childs++;
        isPoint = true;
    }
    if (getNextNextTokenType() == Token::LPS)    //函数声明
    {
        if (isParseFun)
            throw ParserException("variable", Token(getNextTokenType(), getNextTokenLineNo()));
        TreeNode * funNode = parseFunctionDeclare(level + 1);
        funNode->setDataType(dataType);
        node->setLeft(funNode);
        node->setMiddle(parseStmtBlock(level + 1));
        symbolList.append(QString::fromStdString(funNode->value()));
    } else {        //声明变量
        if (temp->dataType() == Token::VOI)
            throw ParserException("function", Token(getNextTokenType(), getNextTokenLineNo()));
        TreeNode * varNode;
        if (isParseFun)
            varNode = parseVariableName(level + 1, true, isPoint);
        else
            varNode = parseVariableName(level + 1);
        varNode->setDataType(dataType);
        node->setLeft(varNode);
        symbolList.append(QString::fromStdString(varNode->value()));
        if (isParseFun)
        {
            TreeNode * regNode = new TreeNode(TreeNode::REG);
            regNode->setValue("@ebp-" + itos(m_argIndex));
            node->setMiddle(regNode);
        }
        else if (getNextTokenType() == Token::ASSIGN)
        {
            consumeNextToken(Token::ASSIGN, level + 1);
            childs += 2;
            if (varNode->left() == NULL)
            {
                node->setMiddle(parseExp(level + 1));
            } else {    //声明时初始化数组
                if (dataType == Token::CHAR && getNextTokenType() == Token::DOUQS)   //如果是char[] = ""
                {
                    node->setMiddle(parseLiteral(level + 1));
                } else {
                    consumeNextToken(Token::LBRACE, level + 1);
                    if (getNextTokenType() == Token::RBRACE)
                    {
                        consumeNextToken(Token::RBRACE, level + 1);
                        childs++;
                    } else {
                        TreeNode * temp_2 = parseExp(level + 1);
                        node->setMiddle(temp_2);
                        childs++;
                        while (getNextTokenType() == Token::COMMA)
                        {
                            consumeNextToken(Token::COMMA, level + 1);
                            childs++;
                            temp_2->setNext(parseExp(level + 1));
                            childs++;
                            temp_2 = temp_2->next();
                        }
                        consumeNextToken(Token::RBRACE, level + 1);
                        childs++;
                    }
                }
            }
        }
        if (isParseFun)
            childs--;
        else
            consumeNextToken(Token::SEMI, level + 1);
    }
    m_queueVec.at(level).back().childNum = childs;
    delete temp;
    return node;
}

//赋值语句
TreeNode * Parser::parseAssignStmt(int level, bool isParseFor)
{
    TreeNode * node = new TreeNode(TreeNode::ASSIGN_STMT);
    if (level == 0)
        m_errorNode = node;
    insertIntoQueueVec("assignStmt", 4, level);
    int childs = 4;
    if (getNextTokenType() == Token::MUL)
    {
        consumeNextToken(Token::MUL, level + 1);
        node->setDataType(Token::DERE);
        childs++;
    }
    node->setLeft(parseVariableName(level + 1));
    node->setLineNo(m_curToken.lineNoValue());
    consumeNextToken(Token::ASSIGN, level + 1);
    node->setMiddle(parseExp(level + 1));
    if (isParseFor)
        childs--;
    else
        consumeNextToken(Token::SEMI, level + 1);
    m_queueVec.at(level).back().childNum = childs;
    return node;
}

//语句块
TreeNode * Parser::parseStmtBlock(int level)
{
    TreeNode * node = new TreeNode(TreeNode::NUL);
    if (level == 0)
        m_errorNode = node;
    insertIntoQueueVec("stmtBlock", 0, level);
    int childs = 2;
    TreeNode * header = node;
    TreeNode * temp;
    consumeNextToken(Token::LBRACE, level + 1);
    while (getNextTokenType() != Token::RBRACE)
    {
        if (getNextTokenType() == Token::LCOM)
        {
            index += 2;
            continue;
        } else if (getNextTokenType() == Token::SCOM) {
            index++;
            continue;
        }
        ++childs;
        temp = parseStmt(level + 1);
        node->setNext(temp);
        node = temp;
    }    
    consumeNextToken(Token::RBRACE, level + 1);
    m_queueVec.at(level).back().childNum = childs;
    return header;
}

//表达式
TreeNode * Parser::parseExp(int level)
{
    TreeNode * node = new TreeNode(TreeNode::EXP);
    insertIntoQueueVec("expression", 1, level);
    node->setDataType(Token::LOGIC_EXP);
    TreeNode * leftNode = parseAddtiveExp(level + 1);
    int types[6] = {Token::EQ, Token::NEQ, Token::GT, Token::GET, Token::LT, Token::LET};
    if (checkNextTokenType(types, 6))
    {
        node->setLeft(leftNode);
        node->setMiddle(parseLogicOp(level + 1));
        node->setRight(parseAddtiveExp(level + 1));
        m_queueVec.at(level).back().childNum = 3;
        return node;
    } else {
        return leftNode;
    }
}

//多项式
TreeNode * Parser::parseAddtiveExp(int level)
{
    TreeNode * node = new TreeNode(TreeNode::EXP);
    insertIntoQueueVec("addtiveExp", 1, level);
    node->setDataType(Token::ADDTIVE_EXP);
    TreeNode * leftNode = parseTerm(level + 1);
    int types[2] = {Token::PLUS, Token::MINUS};
    if (checkNextTokenType(types, 2))
    {
        node->setLeft(leftNode);
        node->setMiddle(parseAddtiveOp(level + 1));
        node->setRight(parseAddtiveExp(level + 1));
        m_queueVec.at(level).back().childNum = 3;
        return node;
    } else {
        return leftNode;
    }
}

//项
TreeNode * Parser::parseTerm(int level)
{
    TreeNode * node = new TreeNode(TreeNode::EXP);
    insertIntoQueueVec("term", 1, level);
    node->setDataType(Token::TERM_EXP);
    TreeNode * leftNode = parseFactor(level + 1);
    int types[2] = {Token::MUL, Token::DIV};
    if (checkNextTokenType(types, 2))
    {
        node->setLeft(leftNode);
        node->setMiddle(parseMutiplyOp(level + 1));
        node->setRight(parseTerm(level + 1));
        m_queueVec.at(level).back().childNum = 3;
        return node;
    } else {
        return leftNode;
    }
}

//因子
TreeNode * Parser::parseFactor(int level)
{
    if (index < m_tokenVec.size())
    {
        TreeNode * node = new TreeNode(TreeNode::FACTOR);
        insertIntoQueueVec("factor", 1, level);
        switch (getNextTokenType()) {
        case Token::INT_VALUE:
        case Token::REAL_VALUE:
        case Token::SINQS:
        case Token::DOUQS:
            node->setLeft(parseLiteral(level + 1));
            break;
        case Token::ADDR:
            consumeNextToken(Token::ADDR, level + 1);
            node->setLeft(parseVariableName(level + 1));
            node->setDataType(Token::ADDR);
            m_queueVec.at(level).back().childNum = 2;
            break;
        case Token::MUL:
            consumeNextToken(Token::MUL, level + 1);
            node->setLeft(parseVariableName(level + 1));
            node->setDataType(Token::MUL);
            m_queueVec.at(level).back().childNum = 2;
            break;
        case Token::LPS:
            consumeNextToken(Token::LPS, level + 1);
            node = parseExp(level + 1);
            consumeNextToken(Token::RPS, level + 1);
            m_queueVec.at(level).back().childNum = 3;
            break;
        case Token::MINUS:
            node->setDataType(Token::MINUS);
            consumeNextToken(Token::MINUS, level + 1);
            node->setLeft(parseTerm(level + 1));
            m_queueVec.at(level).back().childNum = 2;
            break;
        case Token::PLUS:
            consumeNextToken(Token::PLUS, level + 1);
            node->setLeft(parseTerm(level + 1));
            m_queueVec.at(level).back().childNum = 2;
            break;
        default:
            if (getNextNextTokenType() == Token::LPS)
                node->setLeft(parseFunctionCall(level + 1, false));
            else
                node->setLeft(parseVariableName(level + 1));
            break;
        }
        return node;
    }
    throw ParserException("factor", Token(Token::NULLL, m_curToken.lineNoValue()));
}

//字面值
TreeNode * Parser::parseLiteral(int level)
{
    TreeNode * node = new TreeNode(TreeNode::LITERAL);
    insertIntoQueueVec("literal", 1, level);
    int types[2] = {Token::INT_VALUE, Token::REAL_VALUE};
    switch (getNextTokenType()) {
    case Token::INT_VALUE:
    case Token::REAL_VALUE:
        consumeNextToken(types, 2, "literal value", level + 1);
        node->setDataType(m_curToken.typeValue());
        node->setValue(m_curToken.value());
        break;
    case Token::SINQS:
        consumeNextToken(Token::SINQS, level + 1);
        consumeNextToken(Token::CHAR_VALUE, level + 1);
        node->setDataType(Token::CHAR_VALUE);
        node->setValue("'" + m_curToken.value() + "'");
        consumeNextToken(Token::SINQS, level + 1);
        break;
    case Token::DOUQS:
        consumeNextToken(Token::DOUQS, level + 1);
        consumeNextToken(Token::STR_VALUE, level + 1);
        node->setDataType(Token::STR_VALUE);
        node->setValue("\"" + m_curToken.value() + "\"");
        consumeNextToken(Token::DOUQS, level + 1);
        break;
    default:
        throw("literals", Token(getNextTokenType(), getNextTokenLineNo()));
    }
    return node;
}

//逻辑运算符
TreeNode * Parser::parseLogicOp(int level)
{
    TreeNode * node = new TreeNode(TreeNode::OP);
    insertIntoQueueVec("logicalOp", 1, level);
    int types[6] = {Token::EQ, Token::NEQ, Token::GT, Token::GET, Token::LT, Token::LET};
    consumeNextToken(types, 6, "logical operator", level + 1);
    node->setDataType(m_curToken.typeValue());
    return node;
}

//加减运算符
TreeNode * Parser::parseAddtiveOp(int level)
{
    TreeNode * node = new TreeNode(TreeNode::OP);
    insertIntoQueueVec("addtiveOp", 1, level);
    int types[2] = {Token::PLUS, Token::MINUS};
    consumeNextToken(types, 2, "addtive operator", level + 1);
    node->setDataType(m_curToken.typeValue());
    return node;
}

//乘除运算符
TreeNode * Parser::parseMutiplyOp(int level)
{
    TreeNode * node = new TreeNode(TreeNode::OP);
    insertIntoQueueVec("mutiplyOp", 1, level);
    int types[2] = {Token::MUL, Token::DIV};
    consumeNextToken(types, 2, "multiple operator", level + 1);
    node->setDataType(m_curToken.typeValue());
    return node;
}

//变量名
TreeNode * Parser::parseVariableName(int level, bool isParseFun, bool isPoint)
{
    TreeNode * node = new TreeNode(TreeNode::VAR);
    insertIntoQueueVec("variableName", 1, level);
    consumeNextToken(Token::ID, level + 1);
    node->setValue(m_curToken.value());
    node->setLineNo(m_curToken.lineNoValue());
    if (getNextTokenType() == Token::LBRACKET && !isPoint)
    {
        consumeNextToken(Token::LBRACKET, level + 1);
        if (isParseFun)
        {
            //TreeNode tempNode
            node->setLeft(new TreeNode(TreeNode::NUL));
        } else
            node->setLeft(parseExp(level + 1));
        consumeNextToken(Token::RBRACKET, level + 1);
        m_queueVec.at(level).back().childNum = 4;
    }
    return node;
}

//函数声明
TreeNode *Parser::parseFunctionDeclare(int level)
{
    TreeNode * node = new TreeNode(TreeNode::FUNDECLARE);
    insertIntoQueueVec("functionName", 0, level);
    int childs = 3;
    consumeNextToken(Token::ID, level + 1);
    node->setValue(m_curToken.value());
    node->setLineNo(m_curToken.lineNoValue());
    consumeNextToken(Token::LPS, level + 1);
    int types[3] = {Token::INT, Token::REAL, Token::CHAR};
    TreeNode * header, * temp;
    if (checkNextTokenType(types, 3))
    {
        m_argIndex = 1;
        header = parseDeclareStmt(level + 1, true);
        childs++;
        node->setLeft(header);
        while (getNextTokenType() != Token::RPS)
        {
            consumeNextToken(Token::COMMA, level + 1);
            m_argIndex++;
            if (!checkNextTokenType(types, 3))
                throw ParserException("variable", Token(getNextTokenType(), getNextTokenLineNo()));
            temp = parseDeclareStmt(level + 1, true);
            childs += 2;
            header->setNext(temp);
            header = temp;
        }
    }
    consumeNextToken(Token::RPS, level + 1);
    m_queueVec.at(level).back().childNum = childs;
    return node;
}

//函数调用
TreeNode *Parser::parseFunctionCall(int level, bool isStmt)
{
    TreeNode * node = new TreeNode(TreeNode::FUNCALL);
    insertIntoQueueVec("functionCall", 0, level);
    int childs = 3;
    consumeNextToken(Token::ID, level + 1);
    node->setLineNo(m_curToken.lineNoValue());
    node->setValue(m_curToken.value());
    consumeNextToken(Token::LPS, level + 1);
    TreeNode * header, * temp;
    if (getNextTokenType() != Token::RPS)
    {
        header = parseExp(level + 1);
        childs++;
        node->setLeft(header);
        while (getNextTokenType() != Token::RPS)
        {
            consumeNextToken(Token::COMMA, level + 1);
            temp = parseExp(level + 1);
            childs += 2;
            header->setNext(temp);
            header = temp;
        }
    }
    consumeNextToken(Token::RPS, level + 1);
    if (isStmt)
    {
        childs++;
        consumeNextToken(Token::SEMI, level + 1);
    }
    m_queueVec.at(level).back().childNum = childs;
    return node;
}

//变量类型
TreeNode *Parser::parseVariableType(int level)
{
    TreeNode * node = new TreeNode(TreeNode::VARTYPE);
    insertIntoQueueVec("variableType", 1, level);
    int types[4] = {Token::INT, Token::REAL, Token::CHAR, Token::VOI};
    consumeNextToken(types, 4, "variable type", level + 1);
    node->setDataType(m_curToken.typeValue());
    node->setValue(m_curToken.value());
    return node;
}

//return语句
TreeNode *Parser::parseReturn(int level)
{
    TreeNode * node = new TreeNode(TreeNode::RETURN_STMT);
    insertIntoQueueVec("returnStmt", 3, level);
    consumeNextToken(Token::RETURN, level + 1);
    node->setLeft(parseExp(level + 1));
    node->setLineNo(m_curToken.lineNoValue());
    consumeNextToken(Token::SEMI, level + 1);
    return node;
}

//消耗掉下一个Token（单个类型）
void Parser::consumeNextToken(int type, int level)
{
    if (index >= m_tokenVec.size())
        throw ParserException(type, Token(Token::NULLL, m_curToken.lineNoValue()));
    int tempType = m_tokenVec.at(index).typeValue();
    while (index < m_tokenVec.size() && (tempType == Token::SCOM || tempType == Token::LCOM))
    {        
        if (tempType == Token::SCOM)
            index++;
        else if (tempType == Token::LCOM)
            index += 2;
        if (index < m_tokenVec.size())
            tempType = m_tokenVec.at(index).typeValue();
    }
    if (index < m_tokenVec.size())
    {
        m_curToken = m_tokenVec.at(index);
        index++;
        if (m_curToken.typeValue() == type)
        {
            if (type != Token::SINQS && type != Token::DOUQS)
                insertIntoQueueVec(m_curToken.value(), 0, level);
            return;
        }
    } else {
        throw ParserException(type, Token(Token::NULLL, m_curToken.lineNoValue()));
    }
    throw ParserException(type, m_curToken);
}

//消耗掉下一个Token（多个类型中选择）
void Parser::consumeNextToken(int types[], int size, std::string typeStr, int level)
{
    if (index >= m_tokenVec.size())
        throw ParserException(typeStr, Token(Token::NULLL, m_curToken.lineNoValue()));
    int tempType = m_tokenVec.at(index).typeValue();
    while (index < m_tokenVec.size() && (tempType == Token::SCOM || tempType == Token::LCOM))
    {
        if (tempType == Token::SCOM)
            index++;
        else if (tempType == Token::LCOM)
            index += 2;
        if (index < m_tokenVec.size())
            tempType = m_tokenVec.at(index).typeValue();
    }
    if (index < m_tokenVec.size())
    {
        m_curToken = m_tokenVec.at(index);
        index++;
        for (int i = 0; i < size; ++i)
            if (m_curToken.typeValue() == types[i])
            {
                insertIntoQueueVec(m_curToken.value(), 0, level);
                return;
            }
    } else {
        throw ParserException(typeStr, Token(Token::NULLL, m_curToken.lineNoValue()));
    }
    throw ParserException(typeStr, m_curToken);
}

//检查下一个Token类型
bool Parser::checkNextTokenType(int types[], int size)
{
    if (index < m_tokenVec.size())
    {
        int nextType = getNextTokenType();
        for (int i = 0; i < size; ++i)
            if (nextType == types[i])
                return true;
    }
    return false;
}

//获取下一个Token类型
int Parser::getNextTokenType()
{
    if (index < m_tokenVec.size())
        return m_tokenVec.at(index).typeValue();
    else
        return Token::NULLL;
}

//获取下下个Token类型
int Parser::getNextNextTokenType()
{
    if (index + 1 < m_tokenVec.size())
        return m_tokenVec.at(index + 1).typeValue();
    else
        return Token::NULLL;
}

//获取下一个Token行号
int Parser::getNextTokenLineNo()
{
    if (index < m_tokenVec.size())
        return static_cast<Token>(m_tokenVec.at(index)).lineNoValue();
    else
        return m_curToken.lineNoValue();
}

//将数据存入语法树输出容器
void Parser::insertIntoQueueVec(const std::string &value, int childNum, int level)
{
    TreeNodeStr tns = {value, childNum};
    if (m_queueVec.size() < level + 1)
    {
        TQueue tQueue;
        tQueue.push(tns);
        m_queueVec.push_back(tQueue);
    } else {
        m_queueVec.at(level).push(tns);
    }
}

//根据变量类型返回指针类型
int Parser::getPointType(int type)
{
    switch (type) {
    case Token::INT:
        return Token::INT_POINT;
    case Token::REAL:
        return Token::REAL_POINT;
    case Token::CHAR:
        return Token::CHAR_POINT;
    default:
        return -1;
    }
}

//根据变量类型返回地址类型
int Parser::getAddrType(int type)
{
    switch (type) {
    case Token::INT:
        return Token::INT_ADDR;
    case Token::REAL:
        return Token::REAL_ADDR;
    case Token::CHAR:
        return Token::CHAR_ADDR;
    default:
        return -1;
    }
}

//根据变量类型返回解除引用的值类型
int Parser::getDereType(int type)
{
    switch (type) {
    case Token::INT:
        return Token::INT_DERE;
    case Token::REAL:
        return Token::REAL_DERE;
    case Token::CHAR:
        return Token::CHAR_DERE;
    default:
        return -1;
    }
}


Parser & myParser()
{
    static Parser ps;
    return ps;
}


