#ifndef UTIL_H
#define UTIL_H

#include "lexer.h"
#include "parser.h"
#include "codegenerater.h"
#include <QString>
#include <QStringList>

class Lexer;
class Parser;
class CodeGenerater;

class Util
{
public:
    Util();
    bool generaterCodeProcess(std::vector<QuaternaryExp> & outputVec, std::string & cgErrorStr);
    bool syntacticAnalysisProcess(std::vector<QueueVec> & outputVec, std::string & parserErrorStr);
    bool lexicalAnalysisProcess(const QString & text, QStringList & outputList);

private:
    Lexer lexer;
    Parser parser;
    CodeGenerater codeGenerater;
    std::vector<Token> tokenVec;
    std::vector<TreeNode *> treeNodeVec;
    std::vector<QuaternaryExp> qeVec;
};

#endif // UTIL_H
