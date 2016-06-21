#include "common.h"
#include <sstream>

int curExecuteStmtLineNo = 0;

int nextExecuteStmtLineNo = 1;

int programEntry = -1;

std::vector<int> globalVarVec;

std::multimap<int, std::string> errorMap;

QStringList symbolList;

std::string itos(int n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
}


std::string dtos(double d)
{
    std::stringstream ss;
    ss << d;
    return ss.str();
}


std::string ctos(char ch)
{
    std::stringstream ss;
    ss << ch;
    return ss.str();
}

