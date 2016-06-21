#ifndef CODEEXECUTE_H
#define CODEEXECUTE_H
#include "quaternaryexp.h"
#include <vector>
#include <QObject>
#include "symboltable.h"
#include "value.h"
#include <map>
#include <QThread>
#include <stack>
#include <sstream>
#include <QList>
#include <QStringList>

class CodeExecute : public QThread
{
    Q_OBJECT
public:
    enum {RUN, DEBUG, SINGLE_STMT, SINGLE_PRO};
    CodeExecute();
    bool init(const std::vector<QuaternaryExp> & qeVec);        //初始化
    void runSingleStmt();

    void inputToStream(const std::string & value);      //向流中输入内容

    int runMode() const;
    void setRunMode(int runMode);

    std::vector<int> breakpointLineNoVec() const;
    void setBreakpointLineNoVec(const std::vector<int> &breakpointLineNoVec);

    QList<QStringList> getStackData();      //获取栈数据
    QList<QStringList> getGlobalData();     //获取全局区数据
    QList<QStringList> getConstData();      //获取常量区数据
    QList<QStringList> getCodeData();       //获取代码段数据

    bool isOver() const;
    void setIsOver(bool isOver);

    int isDebugFinished() const;
    void setIsDebugFinished(int isDebugFinished);

    int isRunFinished() const;
    void setIsRunFinished(int isRunFinished);

    int isExceptionHappened() const;
    void setIsExceptionHappened(int isExceptionHappened);

protected:
    void run();

signals:
    void sig_input();
    void sig_output(const QString & message);
    void sig_runFinish();
    void sig_exception(const QString & errorStr);
    void sig_debugFinish();

private:
    static const int DATA_ADDR = 0x50000000;    //数据段起始地址(5->4)
    static const int GLOBAL_ADDR = 0x30000000;  //全局区起始地址(3->4)
    static const int CONST_ADDR = 0x20000000;   //常量区起始地址(2->3)
    static const int CODE_ADDR = 0x10000000;    //代码段起始地址(1->2)
    int m_nextQeVecIndex;        //当前代码段索引
    //int m_curValueVecIndex;     //当前数据段索引
    int m_ebp;                  //栈底
    int m_esp;                  //栈顶
    std::stack<int> m_previousEbp;          //上一层函数的栈底
    std::stack<int> m_previousEsp;          //上一层的栈顶
    int m_level;                //代码层
    bool m_waitInput;             //等待输入标记
    std::vector<Value> m_valueVec;      //数据段
    std::map<int, int> m_addrMap;       //数据段索引,地址
    int m_curDataAddr;          //当前数据段地址
    SymbolTable m_symbolTable;          //符号表
    std::vector<QuaternaryExp> m_qeVec;     //四元式集(代码段)
    std::map<std::string, Value> m_tempMap;     //存放临时变量
    std::vector<Value> m_constVec;      //常量区
    std::map<int, int> m_constMap;      //常量区索引，地址
    int m_curConstAddr;         //当前常量区地址
    bool m_isOver;                //程序是否执行完毕
    std::vector<Value> m_globalVec;     //全局区
    std::map<int, int> m_globalMap;     //全局区索引，地址
    int m_curGlobalAddr;                //当前全局区地址
    int m_runMode;                      //运行模式  (0:运行，1:调试，2:逐过程，3:逐语句)
    int m_jmpFun;                    //设置逐过程执行时是否正在跳过函数
    std::vector<int> m_breakpointLineNoVec;     //断点行号

    std::stringstream m_inputStream; //输入缓冲区

    int m_isDebugFinished;
    int m_isRunFinished;
    int m_isExceptionHappened;




private:
    void checkRunMode();                            //检查运行模式
    int getStringType(const std::string & str);     //根据四元式的内容获取其类型
    Value readValue(const std::string & str);
    void assignProcess(const Symbol & symbol, int index, int dereference, const std::string & str);   //赋值操作
    void assignUtil(int valueIndex, int index, const std::string valueStr, int valueType, bool isGlobal);      //赋值辅助操作
    void declareProcess(const QuaternaryExp & qExp, const std::string & type);      //声明操作
    void checkAddressIsRight(int pointType, const std::string & addrValue);    //判断地址类型是否正确
    int getValueIndex(const std::string & addrValue, int & addrRange, int & offset);      //根据地址锁定其位置
    int getScript(const std::string varStr);        //如果是数组，则获取数组下标，否则返回-1
    int getRegNum(const std::string str);           //获取"@ebp..."的值
    std::string addrFromStr(const std::string & addr);  //根据字符串获取地址
    std::string addrFromInt(int addr);      //根据int获取地址
    void setAddrUnitSize(int symbolType);       //设置addrUntiSize的值
    void setAddrUnitSize(const std::string & addrStr);      //设置addrUntiSize的值

};

#endif // CODEEXECUTE_H
