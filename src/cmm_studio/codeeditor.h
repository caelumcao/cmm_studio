#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QFile>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QCompleter;
class LineNumberArea;
class QStringListModel;


class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void loadFile(QFile & file);       //加载文件内容

    bool saveFile(const QString & path);    //保存文件

    void setCompleter(QCompleter *completer);
    QCompleter *completer() const;

    void setDebugArrowIcon(int lineNo, bool isShow);

    int getBreakpointLineNoFromY(int y);

    void updateErrorStatus();

public:
    std::vector<int> breakpointLineNoVec;

    bool isAutoRun() const;
    void setIsAutoRun(bool isAutoRun);

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void insertCompletion(const QString &completion);

    void autoCheck();

private:
    QString textUnderCursor() const;


private:
    QWidget *lineNumberArea;
    QCompleter *c;
    int m_curBlockCount;
    bool m_isShowDebugArrow;
    int m_debugArrowLineNo;
    QStringList m_wordList;
    QCompleter * m_completer;
    QStringListModel * m_model;
    bool m_isAutoRun;
    int m_waitAutoCheckNum;
    QStringList m_curCompleterList;
    int num;

};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
        setMouseTracking(true);
    }

    QSize sizeHint() const Q_DECL_OVERRIDE {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

    void mouseMoveEvent(QMouseEvent * event) Q_DECL_OVERRIDE ;

    void mousePressEvent(QMouseEvent * event) Q_DECL_OVERRIDE ;


private:
    CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H
