#ifndef COMMON_H
#define COMMON_H
#include <string>
#include <vector>
#include <map>
#include <QStringList>

extern int curExecuteStmtLineNo;

extern int nextExecuteStmtLineNo;

extern int programEntry;

extern std::multimap<int, std::string> errorMap;

extern std::vector<int> globalVarVec;

extern QStringList symbolList;

std::string itos(int n);

std::string dtos(double d);

std::string ctos(char ch);



#endif // COMMON_H
