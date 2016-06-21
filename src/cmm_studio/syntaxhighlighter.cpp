#include "syntaxhighlighter.h"
//#include <QTextDocument>
#include <QDebug>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
}


void SyntaxHighlighter::highlightBlock(const QString &text)
{
    QTextCharFormat format_1;
    format_1.setForeground(Qt::blue);
    //QString pattern = "(?<!\\w)(if|else|while|read|write|int|real)(?!\\w)";
    QString pattern_1 = "\\b(if|else|while|read|write|int|real)\\b";
    highlightBlockUtil(text, format_1, pattern_1);

    QTextCharFormat format_2;
    format_2.setForeground(Qt::green);
    QString pattern_2 = "//.*";
    highlightBlockUtil(text, format_2, pattern_2);
//    QString pattern_2_1 = "/\\*";
//    QRegExp expression_2_1(pattern_2_1);
//    QString pattern_2_2 = "\\*/";
//    QRegExp expression_2_2(pattern_2_2);
//    int lComIndex = text.indexOf(expression_2_1);
//    int rComIndex = 0;

//    while (lComIndex > 0)
//    {

//    }


//    QTextCharFormat format_3;
//    format_3.setForeground(Qt::red);
//    QString pattern_3 = "c\n";
//    highlightBlockUtil(text, format_3, pattern_3);
}

void SyntaxHighlighter::highlightBlockUtil(const QString &text, const QTextCharFormat &format, const QString &pattern)
{
    QRegExp expression(pattern);
    int index = text.indexOf(expression);
    while (index >= 0)
    {
        int length = expression.matchedLength();
        setFormat(index, length, format);
        index = text.indexOf(expression, index + length);
    }
}
