#ifndef REPLWIDGET_H
#define REPLWIDGET_H

#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QStack>

class ReplWidget : public QPlainTextEdit
{
  Q_OBJECT
public:
    ReplWidget(QWidget *parent = 0);

public slots:
    void append(const QString & text);
    void startInput();
    void finishOutput();

protected:
    void keyPressEvent(QKeyEvent *event);

    // Do not handle other events
    void mousePressEvent(QMouseEvent *)       { /* Ignore */ }
    void mouseDoubleClickEvent(QMouseEvent *) { /* Ignore */ }
    void mouseMoveEvent(QMouseEvent *)        { /* Ignore */ }
    void mouseReleaseEvent(QMouseEvent *)     { /* Ignore */ }
    void closeEvent(QCloseEvent * event);

private:
    void handleLeft(QKeyEvent *event);
    void handleEnter();
    void handleHome();

    void moveToEndOfLine();
    void clearLine();
    QString getUserInput() const;

    int getIndex (const QTextCursor &crQTextCursor );

    int m_locked;
    int m_inputLineOccupyLen;

signals:
    void sendUserInput(const QString & text);
    void replWidgetClosed();

};

#endif // REPLWIDGET_H
