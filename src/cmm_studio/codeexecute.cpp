#include "codeexecute.h"
#include "common.h"
#include "codeexecuteexception.h"
#include <qDebug>

CodeExecute::CodeExecute()
{

}

bool CodeExecute::init(const std::vector<QuaternaryExp> &qeVec)
{
    m_nextQeVecIndex = programEntry;
    //m_curValueVecIndex = 0;
    m_ebp = 0;
    m_esp = 1;
    while(!m_previousEbp.empty())
        m_previousEbp.pop();
    while (!m_previousEsp.empty())
        m_previousEsp.pop();
    m_level = 0;
    m_waitInput = false;
    m_valueVec.clear();
    m_valueVec.push_back(Value("$" + itos(0x00000000), Value::ADDR));
    m_addrMap.clear();
    m_addrMap[0] = 0x50000000;
    m_curDataAddr = 0x50000000;
    m_symbolTable.clearVec();
    m_qeVec = qeVec;
    m_tempMap.clear();
    m_constVec.clear();
    m_constMap.clear();
    m_curConstAddr = CONST_ADDR;
    m_globalVec.clear();
    m_globalMap.clear();
    m_inputStream.clear();
    m_curGlobalAddr = GLOBAL_ADDR;
    m_isOver = false;
    m_runMode = 0;
    m_breakpointLineNoVec.clear();
    m_jmpFun = 0;
    m_isDebugFinished = 0;
    m_isRunFinished = 0;
    m_isExceptionHappened = 0;
    try {
        if (m_nextQeVecIndex < 0)
            throw CodeExecuteException(-1, "无法找到main函数入口");
        for (int i = 0; i < globalVarVec.size(); ++i)
        {
            m_nextQeVecIndex = globalVarVec.at(i);
            runSingleStmt();
        }
        m_nextQeVecIndex = programEntry;
    } catch (CodeExecuteException & cee) {
        emit sig_exception(QString::fromStdString(cee.message()));
        return false;
    }
    return true;
}

void CodeExecute::runSingleStmt()
{
    QuaternaryExp qExp = m_qeVec.at(m_nextQeVecIndex);
    curExecuteStmtLineNo = qExp.lineNo();
    m_nextQeVecIndex++;
    std::string type = qExp.first();
    if (type == QuaternaryExp::JMP)
    {
        if (qExp.second() == "" || readValue(qExp.second()).valueStr() == "0")
        {
            Value jupValue = readValue(qExp.forth());
            if (jupValue.type() == Value::ADDR)
            {
                if (jupValue.valueStr() == "$" + itos(0x00000000))
                    m_isOver = true;
                int addrRange, offset;
                addrRange = offset = 0;
                m_nextQeVecIndex = getValueIndex(jupValue.valueStr(), addrRange, offset);
            } else {
                m_nextQeVecIndex = atoi(jupValue.valueStr().c_str());
            }
        }
    } else if (type == QuaternaryExp::READ) {
        int index = getScript(qExp.forth());
        int redeference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, redeference);
        emit sig_input();
        m_waitInput = true;
        while (m_waitInput)
        {
            if (m_isOver)
                return;
            msleep(100);
        }
        switch (tempSymbol.type()) {
        case Symbol::SINGLE_INT:
        case Symbol::ARRAY_INT:
        case Symbol::POINT_INT:
        case Symbol::ARRAY_POINT_INT:
            int intTemp;
            m_inputStream >> intTemp;
            assignProcess(tempSymbol, index, redeference, itos(intTemp));
            break;
        case Symbol::SINGLE_REAL:
        case Symbol::ARRAY_REAL:
        case Symbol::POINT_REAL:
        case Symbol::ARRAY_POINT_REAL:
            double doubleTemp;
            m_inputStream >> doubleTemp;
            assignProcess(tempSymbol, index, redeference, dtos(doubleTemp));
            break;
        case Symbol::SINGLE_CHAR:
        case Symbol::POINT_CHAR:
        case Symbol::ARRAY_POINT_CHAR:
            char charTemp;
            m_inputStream >> charTemp;
            assignProcess(tempSymbol, index, redeference, "'" + ctos(charTemp) + "'");
            break;
        case Symbol::ARRAY_CHAR:
            if (index < 0 && redeference == 0)  //cin >> ch
            {
                std::string strTemp;
                m_inputStream >> strTemp;
                int i = 0;
                for (i; i < strTemp.size(); ++i)
                    assignProcess(tempSymbol, i, redeference, "'" + ctos(strTemp.at(i)) + "'");
                assignProcess(tempSymbol, i, redeference, "'\0'");
            } else {
                char charTemp;
                m_inputStream >> charTemp;
                assignProcess(tempSymbol, index, redeference, "'" + ctos(charTemp) + "'");
            }
        default:
            break;
        }
    } else if (type == QuaternaryExp::WRITE) {
        Value tempValue = readValue(qExp.forth());
        std::string returnValue = "";
        if (tempValue.type() == Value::CHAR_VALUE)
        {
            std::string charStr = tempValue.valueStr();
            if (charStr.size() <= 2)
                returnValue = "";
            else
                returnValue = ctos(tempValue.valueStr().at(1));
        } else if (tempValue.type() == Value::ADDR) {
            int addrRange = 0;
            int offset = 0;
            int addrIndex = getValueIndex(tempValue.valueStr(), addrRange, offset);
            if (addrRange == 1)
            {
                if (m_valueVec.at(addrIndex).type() != Value::CHAR_VALUE)
                {
                    int intValue = atoi(tempValue.valueStr().substr(1, tempValue.valueStr().size() - 1).c_str());
                    char ch[9];
                    sprintf(ch, "%08X", intValue);
                    returnValue = ch;
                    returnValue = "0X" + returnValue;
                }
                while (addrIndex > 0 && m_valueVec.at(addrIndex).type() == Value::CHAR_VALUE && m_valueVec.at(addrIndex).valueStr() != "''")
                {
                    returnValue += ctos(m_valueVec.at(addrIndex).valueStr().at(1));
                    addrIndex--;
                }
            } else if (addrRange == 2) {
                if (m_globalVec.at(addrIndex).type() != Value::CHAR_VALUE)
                {
                    int intValue = atoi(tempValue.valueStr().substr(1, tempValue.valueStr().size() - 1).c_str());
                    char ch[9];
                    sprintf(ch, "%08X", intValue);
                    returnValue = ch;
                    returnValue = "0X" + returnValue;
                }
                while (addrIndex < m_globalVec.size() && m_globalVec.at(addrIndex).type() == Value::CHAR_VALUE && m_globalVec.at(addrIndex).valueStr() != "''")
                {
                    returnValue += ctos(m_globalVec.at(addrIndex).valueStr().at(1));
                    addrIndex++;
                }
            } else if (addrRange == 3) {
                if (m_constVec.at(addrIndex).type() != Value::CHAR_VALUE)
                {
                    int intValue = atoi(tempValue.valueStr().substr(1, tempValue.valueStr().size() - 1).c_str());
                    char ch[9];
                    sprintf(ch, "%08X", intValue);
                    returnValue = ch;
                    returnValue = "0X" + returnValue;
                }
                while (addrIndex < m_constVec.size() && m_constVec.at(addrIndex).type() == Value::CHAR_VALUE && m_constVec.at(addrIndex).valueStr() != "''")
                {
                    returnValue += ctos(m_constVec.at(addrIndex).valueStr().at(1));
                    addrIndex++;
                }
            }
        } else {
            returnValue = tempValue.valueStr();
        }
        emit sig_output(QString::fromStdString(returnValue));
    } else if (type == QuaternaryExp::IN) {
        m_previousEsp.push(m_esp);
        m_level++;
    } else if (type == QuaternaryExp::OUT) {
        m_symbolTable.deregisterSymbol(m_level);
        m_level--;
        int tempEsp = m_previousEsp.top();
        m_previousEsp.pop();
        m_valueVec.erase(m_valueVec.begin() + tempEsp, m_valueVec.end());
        m_esp = tempEsp;
    } else if (type == QuaternaryExp::INT) {
        declareProcess(qExp, QuaternaryExp::INT);
    } else if (type == QuaternaryExp::REAL) {
        declareProcess(qExp, QuaternaryExp::REAL);
    } else if (type == QuaternaryExp::CHAR) {
        declareProcess(qExp, QuaternaryExp::CHAR);
    } else if (type == QuaternaryExp::INT_POINT) {
        declareProcess(qExp, QuaternaryExp::INT_POINT);
    } else if (type == QuaternaryExp::REAL_POINT) {
        declareProcess(qExp, QuaternaryExp::REAL_POINT);
    } else if (type == QuaternaryExp::CHAR_POINT) {
        declareProcess(qExp, QuaternaryExp::CHAR_POINT);
    } else if (type == QuaternaryExp::VOID) {
        declareProcess(qExp, QuaternaryExp::VOID);
    } else if (type == QuaternaryExp::ASSIGN) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol;
        if (qExp.forth().at(0) == '@')      //说明是向寄存器赋值
        {
            tempSymbol = Symbol(getRegNum(qExp.forth()));
//            if (qExp.forth() == "@esp")
//            {
//                m_valueVec.push_back(Value());
//                m_esp++;
//            }
        } else {
            tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        }
        assignProcess(tempSymbol, index, dereference, qExp.second());
    } else if (type == QuaternaryExp::PLUS) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        if (l_value.type() == Value::ADDR)
        {
            setAddrUnitSize(l_value.valueStr());
        }else if (r_value.type() == Value::ADDR)
            setAddrUnitSize(r_value.valueStr());
        std::string result = l_value + r_value;
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::MINUS) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        if (qExp.second() == "")
            qExp.setSecond("0");
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        if (l_value.type() == Value::ADDR)
            setAddrUnitSize(l_value.valueStr());
        std::string result = l_value - r_value;
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::MUL) {
        std::string result = "";
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        if (qExp.second() != "")
        {
            Value l_value = readValue(qExp.second());
            Value r_value = readValue(qExp.third());
            result = l_value * r_value;
        } else {    //说明是解除引用
            Value value = readValue(qExp.third());
            int addrRange = 0;
            int offset = 0;
            int addrIndex = getValueIndex(value.valueStr(), addrRange, offset);
            if (addrRange == 1 || addrRange == 2)
            {
                if (offset != 0)
                    throw CodeExecuteException(curExecuteStmtLineNo, "无法对该地址取值");
                if (addrRange == 1)
                    result = m_valueVec.at(addrIndex).valueStr();
                else
                    result = m_globalVec.at(addrIndex).valueStr();
            } else if (addrRange == 3) {
                if (offset != 0)
                    throw CodeExecuteException(curExecuteStmtLineNo, "无法对该地址取值");
                result = m_constVec.at(addrIndex).valueStr();
            }
        }
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::DIV) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        std::string result = l_value / r_value;
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::GT) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        std::string result = l_value > r_value;
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::GET) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        std::string result = l_value >= r_value;
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::LT) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        std::string result = l_value < r_value;
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::LET) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        std::string result = l_value <= r_value;
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::EQ) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        std::string result = l_value == r_value;
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::NEQ) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        Value l_value = readValue(qExp.second());
        Value r_value = readValue(qExp.third());
        std::string result = nequal(l_value, r_value);
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::ADDR) {
        int index = getScript(qExp.forth());
        int dereference = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(qExp.forth(), index, dereference);
        int index_2 = getScript(qExp.third());
        int dereference_2 = 0;
        Symbol symbol = m_symbolTable.getSymbol(qExp.third(), index_2, dereference_2);
        std::string result;
        if (symbol.level() == 0)
            result = "$" + itos(m_globalMap[symbol.valueIndex()]);
        else
            result = "$" + itos(m_addrMap[symbol.valueIndex()]);
        assignProcess(tempSymbol, index, dereference, result);
    } else if (type == QuaternaryExp::CALL) {
        m_previousEbp.push(m_ebp);
        m_ebp = m_esp - 1;
    } else if (type == QuaternaryExp::CALLFH) {
        //m_valueVec.erase(m_valueVec.begin() + m_ebp, m_valueVec.end());
        m_valueVec.erase(m_valueVec.end() - 1);
        m_esp = m_ebp;
        m_ebp = m_previousEbp.top();
        m_previousEbp.pop();
    }

    checkRunMode();
}

//向输入流中添加内容
void CodeExecute::inputToStream(const std::string &value)
{
    m_inputStream.clear();
    m_inputStream.str("");
    m_inputStream << value;
    m_waitInput = false;
}

//运行
void CodeExecute::run()
{
    try {
        m_isOver = false;
        while (!m_isOver)
            runSingleStmt();
    } catch (CodeExecuteException & cee) {
        m_isExceptionHappened = 1;
        emit sig_exception(QString::fromStdString(cee.message()));
        return;
    }

    if (m_nextQeVecIndex == -1 && m_runMode > 0)
        m_isDebugFinished = 1;
    else if (m_nextQeVecIndex == -1 && m_runMode == 0)
        m_isRunFinished = 1;
}
int CodeExecute::isExceptionHappened() const
{
    return m_isExceptionHappened;
}

void CodeExecute::setIsExceptionHappened(int isExceptionHappened)
{
    m_isExceptionHappened = isExceptionHappened;
}

int CodeExecute::isRunFinished() const
{
    return m_isRunFinished;
}

void CodeExecute::setIsRunFinished(int isRunFinished)
{
    m_isRunFinished = isRunFinished;
}

int CodeExecute::isDebugFinished() const
{
    return m_isDebugFinished;
}

void CodeExecute::setIsDebugFinished(int isDebugFinished)
{
    m_isDebugFinished = isDebugFinished;
}

bool CodeExecute::isOver() const
{
    return m_isOver;
}

void CodeExecute::setIsOver(bool isOver)
{
    m_isOver = isOver;
}


void CodeExecute::checkRunMode()
{
    if (m_runMode == 0 || m_nextQeVecIndex >= m_qeVec.size())
        return;
    QuaternaryExp curExp = m_qeVec.at(m_nextQeVecIndex - 1);
    QuaternaryExp nextExp = m_qeVec.at(m_nextQeVecIndex);
    if (m_runMode == 1)     //调试模式
    {
        for (int i = 0; i < m_breakpointLineNoVec.size(); ++i)
            if (nextExp.lineNo() == m_breakpointLineNoVec.at(i) && nextExp.lineNo() != curExecuteStmtLineNo)
                m_isOver = true;
    } else if (m_runMode == 2) {     //逐过程
        if (curExp.first() == QuaternaryExp::CALL)
            m_jmpFun++;
        else if (curExp.first() == QuaternaryExp::CALLFH)
            m_jmpFun--;
        if (m_jmpFun == 0)
        {
            if (nextExp.lineNo() != curExecuteStmtLineNo)
                m_isOver = true;
        }
    } else if (m_runMode == 3) {
        if (nextExp.lineNo() != curExecuteStmtLineNo)
            m_isOver = true;
    }
    nextExecuteStmtLineNo = nextExp.lineNo();
}

std::vector<int> CodeExecute::breakpointLineNoVec() const
{
    return m_breakpointLineNoVec;
}

void CodeExecute::setBreakpointLineNoVec(const std::vector<int> &breakpointLineNoVec)
{
    m_breakpointLineNoVec = breakpointLineNoVec;
}

//获取栈数据
QList<QStringList> CodeExecute::getStackData()
{
    QList<QStringList> dataList;
    for (int i = 0; i < m_valueVec.size(); ++i)
    {
        QStringList tempList;
        std::string addrStr = addrFromInt(m_addrMap[i]);
        tempList.append(QString::fromStdString(addrStr));
        Value value = m_valueVec.at(i);
        std::string valueStr = value.valueStr();
        if (valueStr.at(0) == '$')
            valueStr = addrFromStr(valueStr);
        tempList.append(QString::fromStdString(valueStr));
        tempList.append(QString::fromStdString(value.typeToStr()));
        dataList.append(tempList);
    }
    for (int i = 0; i < m_symbolTable.symbolVec().size(); ++i)
    {
        Symbol tempSymbol = m_symbolTable.symbolVec().at(i);
        if (tempSymbol.level() > 0)
            dataList[tempSymbol.valueIndex()].append(QString::fromStdString(tempSymbol.name()));
    }
    return dataList;
}

//获取全局区数据
QList<QStringList> CodeExecute::getGlobalData()
{
    QList<QStringList> dataList;
    for (int i = 0; i < m_globalVec.size(); ++i)
    {
        QStringList tempList;
        std::string addrStr = addrFromInt(m_globalMap[i]);
        tempList.append(QString::fromStdString(addrStr));
        Value value = m_globalVec.at(i);
        std::string valueStr = value.valueStr();
        if (valueStr.at(0) == '$')
            valueStr = addrFromStr(valueStr);
        tempList.append(QString::fromStdString(valueStr));
        tempList.append(QString::fromStdString(value.typeToStr()));
        dataList.append(tempList);
    }
    for (int i = 0; i < m_symbolTable.symbolVec().size(); ++i)
    {
        Symbol tempSymbol = m_symbolTable.symbolVec().at(i);
        if (tempSymbol.level() == 0)
            dataList[tempSymbol.valueIndex()].append(QString::fromStdString(tempSymbol.name()));
    }
    return dataList;
}

//获取常量区数据
QList<QStringList> CodeExecute::getConstData()
{
    QList<QStringList> dataList;
    for (int i = 0; i < m_constVec.size(); ++i)
    {
        QStringList tempList;
        std::string addrStr = addrFromInt(m_constMap[i]);
        tempList.append(QString::fromStdString(addrStr));
        Value value = m_constVec.at(i);
        std::string valueStr = value.valueStr();
        if (valueStr.at(0) == '$')
            valueStr = addrFromStr(valueStr);
        tempList.append(QString::fromStdString(valueStr));
        tempList.append(QString::fromStdString(value.typeToStr()));
        dataList.append(tempList);
    }
    return dataList;
}

//获取代码段数据
QList<QStringList> CodeExecute::getCodeData()
{
    QList<QStringList> dataList;
    for (int i = 0; i < m_qeVec.size(); ++i)
    {
        QStringList tempList;
        std::string addrStr = addrFromInt(CODE_ADDR + i * 4);
        tempList.append(QString::fromStdString(addrStr));
        QuaternaryExp qExp = m_qeVec.at(i);
        tempList.append(QString::fromStdString(qExp.first()));
        tempList.append(QString::fromStdString(qExp.second()));
        tempList.append(QString::fromStdString(qExp.third()));
        tempList.append(QString::fromStdString(qExp.forth()));
        dataList.append(tempList);
    }
    return dataList;
}

int CodeExecute::runMode() const
{
    return m_runMode;
}

void CodeExecute::setRunMode(int runMode)
{
    m_runMode = runMode;
}



//根据string返回类型(0:int, 1:float, 2:variable, 3.temp)
int CodeExecute::getStringType(const std::string &str)
{
    if ('0' < str.at(0) < '9')
    {
        for (int i = 1; i < str.size(); ++i)
            if (str.at(i) == '.')
                return 1;
        return 0;
    } else if (str.at(0) == '#') {
        return 3;
    } else {
        return 2;
    }
}

//根据字符串读取值
Value CodeExecute::readValue(const std::string &str)
{
    if (isdigit(str.at(0)) || str.at(0) == '-')
    {
        for (int i = 1; i < str.size(); ++i)
            if (str.at(i) == '.')
                return Value(str, Value::REAL_VALUE);
        return Value(str, Value::INT_VALUE);
    } else if (str.at(0) == '\'') {
        return Value(str, Value::CHAR_VALUE);
    } else if (str.at(0) == '$') {
        return Value(str, Value::ADDR);
    } else if (str.at(0) == '#') {
        return m_tempMap[str];
    } else if (str.at(0) == '@') {
        int index = getRegNum(str);
        return m_valueVec.at(index);
    } else if (str.at(0) == '"') {
        int returnAddr = m_curConstAddr;
        for (int i = 1; i < str.size() - 1; ++i)
        {
            m_constVec.push_back(Value("'" + ctos(str.at(i)) + "'", Value::CHAR_VALUE));
            m_constMap[m_constVec.size() - 1] = m_curConstAddr;
            m_curConstAddr = m_curConstAddr + 1;
        }
        m_constVec.push_back(Value("''", Value::CHAR_VALUE));
        m_constMap[m_constVec.size() - 1] = m_curConstAddr;
        m_curConstAddr = m_curConstAddr + 1;
        return Value("$" + itos(returnAddr), Value::ADDR);
    } else {
        int index = getScript(str);
        int dereference = 0;
        std::string resAddr = "";
        int addrRange = 0;
        int offset = 0;
        int addrIndex = 0;
        Symbol tempSymbol = m_symbolTable.getSymbol(str, index, dereference);
        switch (tempSymbol.type()) {
        case Symbol::SINGLE_INT:
        case Symbol::SINGLE_REAL:
        case Symbol::SINGLE_CHAR:
            if (tempSymbol.level() == 0)
                return m_globalVec.at(tempSymbol.valueIndex());
            else
                return m_valueVec.at(tempSymbol.valueIndex());
        case Symbol::POINT_INT:
        case Symbol::POINT_REAL:
        case Symbol::POINT_CHAR:
            if (tempSymbol.level() == 0)
            {
                if (index == -1)
                {
                    return m_globalVec.at(tempSymbol.valueIndex());
                } else {
                    setAddrUnitSize(tempSymbol.type());
                    resAddr = m_globalVec.at(tempSymbol.valueIndex()) + Value(itos(index), Value::INT_VALUE);
                }
            } else {
                if (index == -1)
                {
                    return m_valueVec.at(tempSymbol.valueIndex());
                } else {
                    setAddrUnitSize(tempSymbol.type());
                    resAddr = m_valueVec.at(tempSymbol.valueIndex()) + Value(itos(index), Value::INT_VALUE);
                }
            }
            addrIndex = getValueIndex(resAddr, addrRange, offset);
            if (addrRange == 1 || addrRange == 2)
            {
                if (offset != 0)
                    throw CodeExecuteException(curExecuteStmtLineNo, "无法对该地址取值");
                if (addrRange == 1)
                    return m_valueVec.at(addrIndex);
                else
                    return m_globalVec.at(addrIndex);
            } else if (addrRange == 3) {
                if (offset != 0)
                    throw CodeExecuteException(curExecuteStmtLineNo, "无法对该地址取值");
                return m_constVec.at(addrIndex);
            }
            break;
        case Symbol::ARRAY_INT:
        case Symbol::ARRAY_REAL:
        case Symbol::ARRAY_CHAR:
        case Symbol::ARRAY_POINT_INT:
        case Symbol::ARRAY_POINT_REAL:
        case Symbol::ARRAY_POINT_CHAR:
            if (tempSymbol.level() == 0)
            {
                if (index == -1)
                    return Value("$" + itos(m_globalMap[tempSymbol.valueIndex()]), Value::ADDR);
                return m_globalVec.at(tempSymbol.valueIndex() + index);
            } else {
                if (index == -1)
                    return Value("$" + itos(m_addrMap[tempSymbol.valueIndex()]), Value::ADDR);
                return m_valueVec.at(tempSymbol.valueIndex() - index);
            }
        default:
            break;
        }
    }
}

//赋值处理
void CodeExecute::assignProcess(const Symbol &symbol, int index, int dereference, const std::string &str)
{
    Value tempValue = readValue(str);
    if (dereference == 1)
    {
        int valueIndex = 0;
        int addrRange = 1;
        int offset = 0;
        switch (symbol.type()) {
        case Symbol::ARRAY_INT:
            assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::INT_VALUE, symbol.level() == 0);
            break;
        case Symbol::ARRAY_REAL:
            assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::REAL_VALUE, symbol.level() == 0);
            break;
        case Symbol::ARRAY_CHAR:
            assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::CHAR_VALUE, symbol.level() == 0);
            break;
        case Symbol::POINT_INT:
            if (symbol.level() == 0)
                valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
            else
                valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
            assignUtil(valueIndex, 0, tempValue.valueStr(), Value::INT_VALUE, addrRange == 2);
            break;
        case Symbol::POINT_REAL:
            if (symbol.level() == 0)
                valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
            else
                valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
            assignUtil(valueIndex, 0, tempValue.valueStr(), Value::REAL_VALUE, addrRange == 2);
            break;
        case Symbol::POINT_CHAR:
            if (symbol.level() == 0)
                valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
            else
                valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
            assignUtil(valueIndex, 0, tempValue.valueStr(), Value::CHAR_VALUE, addrRange == 2);
            break;
        case Symbol::ARRAY_POINT_INT:
            if (index >= 0)
            {
                if (symbol.level() == 0)
                    valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex() + index).valueStr(), addrRange, offset);
                else
                    valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex() - index).valueStr(), addrRange, offset);
                assignUtil(valueIndex, 0, tempValue.valueStr(), Value::INT_VALUE, addrRange == 2);
            } else {
                if (symbol.level() == 0)
                    valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
                else
                    valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
                assignUtil(valueIndex, 0, tempValue.valueStr(), Value::ADDR, addrRange == 2);
            }
            break;
        case Symbol::ARRAY_POINT_REAL:
            if (index >= 0)
            {
                if (symbol.level() == 0)
                    valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex() + index).valueStr(), addrRange, offset);
                else
                    valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex() - index).valueStr(), addrRange, offset);
                assignUtil(valueIndex, 0, tempValue.valueStr(), Value::REAL_VALUE, addrRange == 2);
            } else {
                if (symbol.level() == 0)
                    valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
                else
                    valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
                assignUtil(valueIndex, 0, tempValue.valueStr(), Value::ADDR, addrRange == 2);
            }
            break;
        case Symbol::ARRAY_POINT_CHAR:
            if (index >= 0)
            {
                if (symbol.level() == 0)
                    valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex() + index).valueStr(), addrRange, offset);
                else
                    valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex() - index).valueStr(), addrRange, offset);
                assignUtil(valueIndex, 0, tempValue.valueStr(), Value::CHAR_VALUE, addrRange == 2);
            } else {
                if (symbol.level() == 0)
                    valueIndex = getValueIndex(m_globalVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
                else
                    valueIndex = getValueIndex(m_valueVec.at(symbol.valueIndex()).valueStr(), addrRange, offset);
                assignUtil(valueIndex, 0, tempValue.valueStr(), Value::ADDR, addrRange == 2);
            }
            break;
        }
    } else {
        int addrRange = 0;
        int offset = 0;
        int valueIndex = 0;
        std::string resAddr = "";
        if (offset != 0)
            throw CodeExecuteException(curExecuteStmtLineNo, "无法对该地址进行赋值");

        switch (symbol.type()) {
        case Symbol::TEMP:
            m_tempMap[symbol.name()] = tempValue;
            break;
        case Symbol::REG:
            if (symbol.valueIndex() == m_esp)
            {
                m_valueVec.push_back(tempValue);
                m_addrMap[m_esp] = m_curDataAddr - tempValue.size();
                m_curDataAddr = m_addrMap[m_esp];
                m_esp++;
            }
            m_valueVec.at(symbol.valueIndex()) = tempValue;
            break;
        case Symbol::SINGLE_INT:
            assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::INT_VALUE, symbol.level() == 0);
            break;
        case Symbol::SINGLE_REAL:
            assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::REAL_VALUE, symbol.level() == 0);
            break;
        case Symbol::SINGLE_CHAR:
            assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::CHAR_VALUE, symbol.level() == 0);
            break;
        case Symbol::ARRAY_INT:
            assignUtil(symbol.valueIndex(), index, tempValue.valueStr(), Value::INT_VALUE, symbol.level() == 0);
            break;
        case Symbol::ARRAY_REAL:
            assignUtil(symbol.valueIndex(), index, tempValue.valueStr(), Value::REAL_VALUE, symbol.level() == 0);
            break;
        case Symbol::ARRAY_CHAR:
            assignUtil(symbol.valueIndex(), index, tempValue.valueStr(), Value::CHAR_VALUE, symbol.level() == 0);
            break;
        case Symbol::POINT_CHAR:
        case Symbol::POINT_INT:
        case Symbol::POINT_REAL:
            if (symbol.level() == 0 && index != -1)
            {
                setAddrUnitSize(symbol.type());
                resAddr = m_globalVec.at(symbol.valueIndex()) + Value(itos(index), Value::INT_VALUE);
            } else if (symbol.level() > 0 && index != -1) {
                setAddrUnitSize(symbol.type());
                resAddr = m_valueVec.at(symbol.valueIndex()) + Value(itos(index), Value::INT_VALUE);
            }
            if (symbol.type() == Symbol::POINT_INT)
            {
                if (index == -1)
                {
                    checkAddressIsRight(Symbol::POINT_INT, tempValue.valueStr());
                    assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::ADDR, symbol.level() == 0);
                } else {
                    checkAddressIsRight(Symbol::POINT_INT, resAddr);
                    valueIndex = getValueIndex(resAddr, addrRange, offset);
                    if (offset != 0)
                        throw CodeExecuteException(curExecuteStmtLineNo, "无法对该地址进行赋值操作");
                    if (addrRange == 3)
                        throw CodeExecuteException(curExecuteStmtLineNo, "无法对常量区进行赋值操作");
                    assignUtil(valueIndex, 0, tempValue.valueStr(), Value::INT_VALUE, addrRange == 2);
                }
            } else if (symbol.type() == Symbol::POINT_REAL) {
                if (index == -1)
                {
                    checkAddressIsRight(Symbol::POINT_REAL, tempValue.valueStr());
                    assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::ADDR, symbol.level() == 0);
                } else {
                    checkAddressIsRight(Symbol::POINT_REAL, resAddr);
                    valueIndex = getValueIndex(resAddr, addrRange, offset);
                    if (offset != 0)
                        throw CodeExecuteException(curExecuteStmtLineNo, "无法对该地址进行赋值操作");
                    if (addrRange == 3)
                        throw CodeExecuteException(curExecuteStmtLineNo, "无法对常量区进行赋值操作");
                    assignUtil(valueIndex, 0, tempValue.valueStr(), Value::REAL_VALUE, addrRange == 2);
                }
            } else if (symbol.type() == Symbol::POINT_CHAR) {
                if (index == -1)
                {
                    checkAddressIsRight(Symbol::POINT_CHAR, tempValue.valueStr());
                    assignUtil(symbol.valueIndex(), 0, tempValue.valueStr(), Value::ADDR, symbol.level() == 0);
                } else {
                    checkAddressIsRight(Symbol::POINT_CHAR, resAddr);
                    valueIndex = getValueIndex(resAddr, addrRange, offset);
                    if (offset != 0)
                        throw CodeExecuteException(curExecuteStmtLineNo, "无法对该地址进行赋值操作");
                    if (addrRange == 3)
                        throw CodeExecuteException(curExecuteStmtLineNo, "无法对常量区进行赋值操作");
                    assignUtil(valueIndex, 0, tempValue.valueStr(), Value::CHAR_VALUE, addrRange == 2);
                }
            }
            break;
        case Symbol::ARRAY_POINT_INT:
            checkAddressIsRight(Symbol::POINT_INT, tempValue.valueStr());
            assignUtil(symbol.valueIndex(), index, tempValue.valueStr(), Value::ADDR, symbol.level() == 0);
            break;
        case Symbol::ARRAY_POINT_REAL:
            checkAddressIsRight(Symbol::POINT_REAL, tempValue.valueStr());
            assignUtil(symbol.valueIndex(), index, tempValue.valueStr(), Value::ADDR, symbol.level() == 0);
            break;
        case Symbol::ARRAY_POINT_CHAR:
            checkAddressIsRight(Symbol::POINT_CHAR, tempValue.valueStr());
            assignUtil(symbol.valueIndex(), index, tempValue.valueStr(), Value::ADDR, symbol.level() == 0);
            break;
        }
    }
}

//赋值辅助函数
void CodeExecute::assignUtil(int valueIndex, int index, const std::string valueStr, int valueType, bool isGlobal)
{
    if (isGlobal)
        m_globalVec.at(valueIndex + index) = Value(valueStr, valueType);
    else
        m_valueVec.at(valueIndex - index) = Value(valueStr, valueType);
}

//声明处理
void CodeExecute::declareProcess(const QuaternaryExp &qExp, const std::string & type)
{
     int symbolType = -1;
     int funSymbolType = -1;
     int valueType = -1;
     std::string initValue = "0";
     int elementNum = 0;
     if (qExp.third() == "")    //非数组
     {
         if (type == QuaternaryExp::INT) {
             symbolType = Symbol::SINGLE_INT;
             funSymbolType = FunSymbol::INT;
             valueType = Value::INT_VALUE;
         } else if (type == QuaternaryExp::REAL) {
             symbolType = Symbol::SINGLE_REAL;
             funSymbolType = FunSymbol::REAL;
             valueType = Value::REAL_VALUE;
         } else if (type == QuaternaryExp::CHAR) {
             symbolType = Symbol::SINGLE_CHAR;
             funSymbolType = FunSymbol::CHAR;
             valueType = Value::CHAR_VALUE;
             initValue = "''";
         } else if (type == QuaternaryExp::INT_POINT) {
             symbolType = Symbol::POINT_INT;
             funSymbolType = FunSymbol::INT_POINT;
             valueType = Value::ADDR;
             initValue = "$-1";
             elementNum = -1;
         } else if (type == QuaternaryExp::REAL_POINT) {
             symbolType = Symbol::POINT_REAL;
             funSymbolType = FunSymbol::REAL_POINT;
             valueType = Value::ADDR;
             initValue = "$-1";
             elementNum = -1;
         } else if (type == QuaternaryExp::CHAR_POINT) {
             symbolType = Symbol::POINT_CHAR;
             funSymbolType = FunSymbol::CHAR_POINT;
             valueType = Value::ADDR;
             initValue = "$-1";
             elementNum = -1;
         } else if (type == QuaternaryExp::VOID) {
             symbolType = FunSymbol::VOID;
         }

         if (qExp.forth().at(qExp.forth().size() - 1) == ')')  //说明是函数
         {
             FunSymbol funSymbol(qExp.forth().substr(0, qExp.forth().size() - 2), qExp.lineNo(), funSymbolType);
             m_symbolTable.registerFunSymbol(funSymbol);
         } else {
             if (qExp.second() != "" && qExp.second().at(0) == '@') //说明是函数参数
             {
                 Symbol symbol(qExp.forth(), symbolType, qExp.lineNo(), m_level, elementNum);
                 symbol.setValueIndex(getRegNum(qExp.second()));
                 m_symbolTable.registerSymbol(symbol);
                 assignProcess(symbol, -1, 0, qExp.second());
             } else {
                 Symbol symbol(qExp.forth(), symbolType, qExp.lineNo(), m_level, elementNum);
                 if (m_level == 0)
                 {
                     int globalIndex = m_globalVec.size();
                     symbol.setValueIndex(globalIndex);
                     m_symbolTable.registerSymbol(symbol);
                     m_globalVec.push_back(Value(initValue, valueType));
                     m_globalMap[globalIndex] = m_curGlobalAddr;
                     m_curGlobalAddr = m_globalMap[globalIndex] + symbol.dataSize();
                 } else {
                     symbol.setValueIndex(m_esp);
                     m_symbolTable.registerSymbol(symbol);
                     m_valueVec.push_back(Value(initValue, valueType));
                     m_addrMap[m_esp] = m_curDataAddr - symbol.dataSize();
                     m_curDataAddr = m_addrMap[m_esp];
                     m_esp++;
                 }
                 if (qExp.second() != "")
                     assignProcess(symbol, -1, 0, qExp.second());
             }
         }
     } else {       //数组
         if (type == QuaternaryExp::INT) {
             if (qExp.third() == "0")   //说明是函数参数，数组相当于指针
                 symbolType = Symbol::POINT_INT;
             else
                symbolType = Symbol::ARRAY_INT;
             valueType = Value::INT_VALUE;
         } else if (type == QuaternaryExp::REAL) {
             if (qExp.third() == "0")   //说明是函数参数，数组相当于指针
                 symbolType = Symbol::POINT_REAL;
             else
                symbolType = Symbol::ARRAY_REAL;
             valueType = Value::REAL_VALUE;
         } else if (type == QuaternaryExp::CHAR) {
             if (qExp.third() == "0")   //说明是函数参数，数组相当于指针
                 symbolType = Symbol::POINT_CHAR;
             else
                symbolType = Symbol::ARRAY_CHAR;
             valueType = Value::CHAR_VALUE;
             initValue = "''";
         } else if (type == QuaternaryExp::INT_POINT) {
             symbolType = Symbol::ARRAY_POINT_INT;
             valueType = Value::ADDR;
             initValue = "$-1";
         } else if (type == QuaternaryExp::REAL_POINT) {
             symbolType = Symbol::ARRAY_POINT_REAL;
             valueType = Value::ADDR;
             initValue = "$-1";
         } else if (type == QuaternaryExp::CHAR_POINT) {
             symbolType = Symbol::ARRAY_POINT_CHAR;
             valueType = Value::ADDR;
             initValue = "$-1";
         }

         if (qExp.third() == "0")
         {
             Symbol symbol(qExp.forth(), symbolType, qExp.lineNo(), m_level);
             symbol.setValueIndex(getRegNum(qExp.second()));
             m_symbolTable.registerSymbol(symbol);
             assignProcess(symbol, -1, 0, qExp.second());
         } else {
             Symbol symbol(qExp.forth(), symbolType, qExp.lineNo(), m_level, atoi(qExp.third().c_str()));
             if (m_level == 0)
             {
                 int globalIndex = m_globalVec.size();
                 symbol.setValueIndex(globalIndex);
                 m_symbolTable.registerSymbol(symbol);
                 for (int i = 0; i< symbol.elementNum(); ++i)
                 {
                     m_globalVec.push_back(Value(initValue, valueType));
                     m_globalMap[globalIndex] = m_curGlobalAddr + symbol.dataSize() * (i + 1);
                     globalIndex++;
                 }
                 m_curGlobalAddr = m_globalMap[globalIndex - 1];
             } else {
                 int eleNum = symbol.elementNum();
                 int preEsp = m_esp;
                 m_esp += eleNum;
                 symbol.setValueIndex(m_esp - 1);
                 m_symbolTable.registerSymbol(symbol);
                 for (int i = 0; i < eleNum; ++i)
                 {
                     m_valueVec.push_back(Value(initValue, valueType));
                     m_addrMap[preEsp] = m_curDataAddr - symbol.dataSize() * (i + 1);
                     preEsp++;
                 }
                 m_curDataAddr = m_addrMap[symbol.valueIndex()];
             }
         }
     }
}

//判断地址类型是否正确
void CodeExecute::checkAddressIsRight(int pointType, const std::string &addrValue)
{
    int addrRange = 0;
    int offset = 0;
    int addrIndex = getValueIndex(addrValue, addrRange, offset);
    if (offset != 0)
        throw CodeExecuteException(curExecuteStmtLineNo, "无法将该类型的地址赋给指针");
    if (addrRange == 3)
    {
        if (pointType == Symbol::POINT_CHAR)
            return;
        else
            throw CodeExecuteException(curExecuteStmtLineNo, "无法将该类型的地址赋给指针");
    }
    bool isFalse;
    switch (pointType) {
    case Symbol::POINT_INT:
        if (addrRange == 1)
            isFalse = m_valueVec.at(addrIndex).type() != Value::INT_VALUE;
        else if (addrRange == 2)
            isFalse = m_globalVec.at(addrIndex).type() != Value::INT_VALUE;
        if (isFalse)
            throw CodeExecuteException(curExecuteStmtLineNo, "无法将该类型的地址赋给指针");
        break;
    case Symbol::POINT_REAL:
        if (addrRange == 1)
            isFalse = m_valueVec.at(addrIndex).type() != Value::REAL_VALUE;
        else if (addrRange == 2)
            isFalse = m_globalVec.at(addrIndex).type() != Value::REAL_VALUE;
        if (isFalse)
            throw CodeExecuteException(curExecuteStmtLineNo, "无法将该类型的地址赋给指针");
        break;
    case Symbol::POINT_CHAR:
        if (addrRange == 1)
            isFalse = m_valueVec.at(addrIndex).type() != Value::CHAR_VALUE;
        else if (addrRange == 2)
            isFalse = m_globalVec.at(addrIndex).type() != Value::CHAR_VALUE;
        if (isFalse)
            throw CodeExecuteException(curExecuteStmtLineNo, "无法将该类型的地址赋给指针");
        break;
    default:
        break;
    }
}

//根据地址锁定其位置
int CodeExecute::getValueIndex(const std::string &addrValue, int &addrRange, int &offset)
{
    int addr = atoi(addrValue.substr(1, addrValue.size() - 1).c_str());
    if (addr <= DATA_ADDR && addr > 0x40000000)     //数据段
    {
        addrRange = 1;
        for (int i = 0; i < m_valueVec.size(); ++i)
        {
            if (addr >= m_addrMap[i])
            {
                if (i + 1 < m_valueVec.size() && m_addrMap[i + 1] > m_addrMap[i])
                {
                    i = i + 1;
                    while (i < m_valueVec.size())
                    {

                    }
                } else {
                    offset = addr - m_addrMap[i];
                    return i;
                }
            }
        }
        throw CodeExecuteException(curExecuteStmtLineNo, "堆溢出");
    } else if (addr >= GLOBAL_ADDR && addr < 0x40000000) {      //全局区
        addrRange = 2;
        for (int i = 0; i < m_globalVec.size(); ++i)
        {
            if (addr <= m_globalMap[i])
            {
                if (addr < m_globalMap[i])
                {
                    offset = addr - m_globalMap[i - 1];
                    return i - 1;
                } else {
                    return i;
                }
            }
        }
        throw CodeExecuteException(curExecuteStmtLineNo, "堆溢出");
    } else if (addr >= CONST_ADDR && addr < GLOBAL_ADDR) {      //常量区
        addrRange = 3;
        for (int i = 0; i < m_constVec.size(); ++i)
        {
            if (addr <= m_constMap[i])
            {
                if (addr < m_constMap[i])
                {
                    offset = addr - m_constMap[i - 1];
                    return i - 1;
                } else {
                    return i;
                }
            }
        }
        throw CodeExecuteException(curExecuteStmtLineNo, "栈溢出");
    } else if (addr >= CODE_ADDR && addr < CONST_ADDR) {        //代码区
        addrRange = 4;
        return (addr - CODE_ADDR) / 4;
    } else if (addr > DATA_ADDR) {
        throw CodeExecuteException(curExecuteStmtLineNo, "栈溢出");
    } else {
        return -1;
    }
}

//如果是数组，则获取数组下标，否则返回-1
int CodeExecute::getScript(const std::string varStr)
{
    int index = -1;
    int start = varStr.find('[');
    int end = varStr.find(']');
    if (start >= 0)
    {
        std::string indexStr = varStr.substr(start + 1, end - start - 1);
        index = atoi(readValue(indexStr).valueStr().c_str());
    }
    return index;
}


//获取"@ebp..."的值
int CodeExecute::getRegNum(const std::string str)
{
    int result = 0;
    if (str.substr(1, 3) == "ebp")
    {
        if (str.size() > 4 && str.at(4) == '-')
            result = m_ebp - atoi(str.substr(5, str.size() - 5).c_str());
        else
            result = m_ebp;
    } else if (str.substr(1, 3) == "esp") {
        if (str.size() > 4 && str.at(4) == '-')
            result = m_esp - atoi(str.substr(5, str.size() - 5).c_str());
        else
            result = m_esp;
    }
    return result;
}

//根据字符串获取地址
std::string CodeExecute::addrFromStr(const std::string &addr)
{
    std::string addrStr;
    int intValue = atoi(addr.substr(1, addr.size() - 1).c_str());
    char ch[9];
    sprintf(ch, "%08X", intValue);
    addrStr = ch;
    return "0X" + addrStr;
}

//根据int获取地址
std::string CodeExecute::addrFromInt(int addr)
{
    std::string addrStr;
    char ch[9];
    sprintf(ch, "%08X", addr);
    addrStr = ch;
    return "0X" + addrStr;
}

//设置addrUntiSize的值
void CodeExecute::setAddrUnitSize(int symbolType)
{
    switch (symbolType) {
    case Symbol::POINT_INT:
        Value::addrUnitSize = 4;
        break;
    case Symbol::POINT_REAL:
        Value::addrUnitSize = 8;
        break;
    case Symbol::POINT_CHAR:
        Value::addrUnitSize = 1;
        break;
    default:
        break;
    }
}

//设置addrUntiSize的值
void CodeExecute::setAddrUnitSize(const std::string &addrStr)
{
    //Value value = readValue(qExp.third());
    Value value;
    int addrRange = 0;
    int offset = 0;
    int addrIndex = getValueIndex(addrStr, addrRange, offset);
    if (addrRange == 1 || addrRange == 2)
    {
        if (offset != 0)
            throw CodeExecuteException(curExecuteStmtLineNo, "地址偏移异常");
        if (addrRange == 1)
            value = m_valueVec.at(addrIndex);
        else
            value = m_globalVec.at(addrIndex);
    } else if (addrRange == 3) {
        if (offset != 0)
            throw CodeExecuteException(curExecuteStmtLineNo, "地址偏移异常");
        value = m_constVec.at(addrIndex);
    }
    Value::addrUnitSize = value.size();
}
