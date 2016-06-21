#ifndef ERROR_H
#define ERROR_H

#include <vector>
#include <string>

extern std::vector<int> errorVec;

static std::string errorStr(int errorNo)
{
    switch (errorNo)
    {
    case 1:
        return "词法错误：标识符以下划线结尾";
    case 2:
        return "词法错误：非合法的字符";
    }
}

#endif // ERROR_H
