#include "replwidget.h"

ReplWidget::ReplWidget(QWidget *parent) : QPlainTextEdit(parent)
{
    m_locked = 0;
    m_inputLineOccupyLen = 0;

    setLineWrapMode(NoWrap);
    resize(800, 600);
}

//输出字符
void ReplWidget::append(const QString &text)
{
    insertPlainText(text);
    ensureCursorVisible();
}

//开始输入
void ReplWidget::startInput()
{
    m_inputLineOccupyLen = getIndex(textCursor());
    m_locked = 1;
    setFocus();
}

//输出结束
void ReplWidget::finishOutput()
{
    insertPlainText("请按任意键结束!");
    m_locked = 2;
    setFocus();
}

//键盘事件
void ReplWidget::keyPressEvent(QKeyEvent *event) {
    if(m_locked == 0)
        return;
    else if (m_locked == 2)
        deleteLater();

    switch(event->key()) {
    case Qt::Key_Return:
        handleEnter();
        break;
    case Qt::Key_Backspace:
        handleLeft(event);
        break;
    case Qt::Key_Left:
        handleLeft(event);
        break;
    case Qt::Key_Home:
        handleHome();
        break;
    default:
        QPlainTextEdit::keyPressEvent(event);
        break;
    }
}

//关闭事件
void ReplWidget::closeEvent(QCloseEvent *event)
{
    emit replWidgetClosed();
}

//Enter键按下
void ReplWidget::handleEnter() {
    QString inputStr = getUserInput();

    moveToEndOfLine();

    if(inputStr.length() > 0) {
        m_locked = 0;
        //setFocus();
        insertPlainText("\n");
        emit sendUserInput(inputStr);
    } else {
//        insertPlainText("\n");
//        ensureCursorVisible();
    }
}

//获取用户输入的字符串
QString ReplWidget::getUserInput() const
{
    QTextCursor c = this->textCursor();
    c.select(QTextCursor::LineUnderCursor);

    QString text = c.selectedText();
    text.remove(0, m_inputLineOccupyLen);

    return text;
}

//将光标移动到行末
void ReplWidget::moveToEndOfLine() {
    QPlainTextEdit::moveCursor(QTextCursor::EndOfLine);
}

//处理删除和左移光标
void ReplWidget::handleLeft(QKeyEvent *event) {
    if(getIndex(textCursor()) > m_inputLineOccupyLen) //如果光标在系统输出字符串之后
        QPlainTextEdit::keyPressEvent(event);
}

//按下home键
void ReplWidget::handleHome() {
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::StartOfLine);
    c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, m_inputLineOccupyLen);
    setTextCursor(c);
}

//返回光标所在列数
int ReplWidget::getIndex (const QTextCursor &crQTextCursor ) {
    QTextBlock b;
    int column = 1;
    b = crQTextCursor.block();
    column = crQTextCursor.position() - b.position();
    return column;
}




