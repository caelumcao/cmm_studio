#include "util.h"
#include "common.h"


Util::Util()
{

}

bool Util::generaterCodeProcess(std::vector<QuaternaryExp> & outputVec, std::string & cgErrorStr)
{
    qeVec = codeGenerater.generateCode(treeNodeVec, cgErrorStr);
    outputVec = qeVec;
    if (cgErrorStr == "")
        return true;
    else
        return false;
}

bool Util::syntacticAnalysisProcess(std::vector<QueueVec> & outputVec, std::string & parserErrorStr)
{
   treeNodeVec = parser.syntacticAnalyse(tokenVec, outputVec, parserErrorStr);

   if (parserErrorStr == "")
       return true;
   else
       return false;

}

bool Util::lexicalAnalysisProcess(const QString & text, QStringList & outputList)
{
    errorVec.clear();
    errorMap.clear();
    symbolList.clear();
    std::stringstream textStream(text.toStdString());
    tokenVec = lexer.analyze(text.toStdString());

    std::string line;
    int lineNo = 1;
    int index = 0;
    while (std::getline(textStream, line))
    {
        outputList.append(QString("%1: ").arg(lineNo) + QString::fromStdString(line));
        while ((index < tokenVec.size()) && (static_cast<Token>(tokenVec.at(index)).lineNoValue() == lineNo))
        {
            outputList.append(QString::fromStdString("  " + static_cast<Token>(tokenVec.at(index)).toString()));
            ++index;
        }
        ++lineNo;
    }

    if (errorVec.size() > 0)
        return false;
    else
        return true;
}
