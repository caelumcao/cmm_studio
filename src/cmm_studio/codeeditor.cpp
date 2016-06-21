#include <QCompleter>
#include <QKeyEvent>
#include <QApplication>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QStringListModel>
#include <iterator>
#include "codeeditor.h"
#include "common.h"
#include "util.h"
#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QTextStream>
#include <QMessageBox>
#include <QTextBlock>
#include <QPainter>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QDebug>
#include <windows.h>



CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), c(0)
{
    lineNumberArea = new LineNumberArea(this);
    m_curBlockCount = 1;
    m_isShowDebugArrow = false;
    m_debugArrowLineNo = 0;
    m_isAutoRun = false;
    m_waitAutoCheckNum = 0;
    m_curCompleterList.clear();
    num = 0;

    //设置completer
    m_wordList << "if" << "else" << "while" << "read"
               << "write" << "void" << "int" << "real" << "char"
               << "return" << "main" << "for";
    m_wordList.sort(Qt::CaseInsensitive);
    m_completer = new QCompleter(this);
    m_model = new QStringListModel(m_wordList, m_completer);
    m_completer->setModel(m_model);
    m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setWrapAround(false);
    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate(this);
    m_completer->popup()->setItemDelegate(itemDelegate);

    setCompleter(m_completer);

    QFont font;
    font.setFamily(QStringLiteral("Courier"));
    font.setPointSize(12);
    //font.setBold(false);
    //font.setItalic(false);
    //font.setWeight(50);
    setFont(font);
    //setStyleSheet(QLatin1String("background-color: rgb(76, 76, 76);"
//                                          "font: 12pt \"Courier\";"
//                                          "color: rgb(236, 236, 236);"));
    setTabStopWidth(40);
    setWordWrapMode(QTextOption::NoWrap);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(textChanged()), this, SLOT(autoCheck()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

}


void CodeEditor::updateErrorStatus()
{
    QTextBlock block = this->firstVisibleBlock();
    std::multimap<int, std::string>::iterator pos;
    pos = errorMap.begin();
    while (block.isValid())
    {
        QTextCursor cursor = textCursor();
        cursor.setPosition(block.position());
        if (pos != errorMap.end() && block.blockNumber() + 1 == pos->first)
        {
            for (QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it)
            {
                QTextCharFormat format = block.charFormat();
                format.setUnderlineColor(Qt::red);
                format.setFontUnderline(true);
                format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
                format.setToolTip(QString::fromStdString(pos->second));
                cursor.setPosition(it.fragment().position() + it.fragment().length(), QTextCursor::KeepAnchor);
                cursor.setCharFormat(format);
            }
            ++pos;
        } else {
            for (QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it)
            {
                QTextCharFormat format = block.charFormat();
                format.setFontUnderline(false);
                format.setToolTip("");
                cursor.setPosition(it.fragment().position() + it.fragment().length(), QTextCursor::KeepAnchor);
                cursor.setCharFormat(format);
            }
        }
        block = block.next();
    }
}

/**
  * 后台自动检查函数
  */
void CodeEditor::autoCheck()
{
    if (m_isAutoRun)
        return;
    num++;
    m_isAutoRun = true;
    Util util;
    QStringList list;
    if (util.lexicalAnalysisProcess(this->toPlainText(), list))
    {
        std::vector<QueueVec> outputVec;
        std::string errorStr = "";
        if (util.syntacticAnalysisProcess(outputVec, errorStr))
        {
            std::vector<QuaternaryExp> qeVec;
            errorStr.clear();
            util.generaterCodeProcess(qeVec, errorStr);
        }
    }

    QStringList tempList = symbolList;
    tempList.sort(Qt::CaseInsensitive);
    if (m_curCompleterList != tempList)
    {
        m_curCompleterList = tempList;
        QStringList list = m_wordList;
        for (int i = 0; i < m_curCompleterList.size(); ++i)
            list.append(m_curCompleterList.at(i));
        list.sort(Qt::CaseInsensitive);
        m_model->setStringList(list);
    }
    QTextBlock block = this->firstVisibleBlock();
    std::multimap<int, std::string>::iterator pos;
    pos = errorMap.begin();
    while (block.isValid())
    {
        QTextCursor curCursor = textCursor();
        QTextCursor cursor = curCursor;
        cursor.setPosition(block.position());
        if (curCursor.blockNumber() != block.blockNumber() && pos != errorMap.end() && block.blockNumber() + 1 == pos->first)
        {
            for (QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it)
            {
                QTextCharFormat format = block.charFormat();
                format.setUnderlineColor(Qt::red);
                format.setFontUnderline(true);
                format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
                format.setToolTip(QString::fromStdString(pos->second));
                cursor.setPosition(it.fragment().position() + it.fragment().length(), QTextCursor::KeepAnchor);
                cursor.setCharFormat(format);
            }
            ++pos;
        } else {
            for (QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it)
            {
                QTextCharFormat format = block.charFormat();
                format.setFontUnderline(false);
                format.setToolTip("");
                cursor.setPosition(it.fragment().position() + it.fragment().length(), QTextCursor::KeepAnchor);
                cursor.setCharFormat(format);
            }
        }
        block = block.next();
    }
    //Sleep(500);
    m_isAutoRun = false;
}


/**
 * @brief 文件操作
 */

void CodeEditor::loadFile(QFile &file)
{
    QTextStream in(&file); // 新建文本流对象
    in.setCodec("utf-8");       //设置编码
    QApplication::setOverrideCursor(Qt::WaitCursor); // 设置鼠标状态为等待状态
    setPlainText(in.readAll());  // 读取文件的全部文本内容，并添加到编辑器中
    QApplication::restoreOverrideCursor(); // 恢复鼠标状态
}

//保存文件
bool CodeEditor::saveFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("多文档编辑器"),
                             tr("无法写入文件 %1:\n%2.")
                             .arg(path).arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText(); // 以纯文本文件写入
    QApplication::restoreOverrideCursor();

    return true;
}


/**
  * QCompleter相关
  */
void CodeEditor::setCompleter(QCompleter *completer)
{
    if (c)
        QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

QCompleter *CodeEditor::completer() const
{
    return c;
}

void CodeEditor::insertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}
bool CodeEditor::isAutoRun() const
{
    return m_isAutoRun;
}

void CodeEditor::setIsAutoRun(bool isAutoRun)
{
    m_isAutoRun = isAutoRun;
}


void CodeEditor::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }


    bool isShortcut = ((e->modifiers() & Qt::AltModifier) && e->key() == Qt::Key_Slash); // CTRL+E
    if (!c || !isShortcut) // do not process the shortcut when we have a completer
    {
        if ((e->modifiers() & Qt::AltModifier) && e->key() == Qt::Key_I)
        {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Up);
            setTextCursor(cursor);
        } else if ((e->modifiers() & Qt::AltModifier) && e->key() == Qt::Key_K) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Down);
            setTextCursor(cursor);
        } else if ((e->modifiers() & Qt::AltModifier) && e->key() == Qt::Key_J) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Left);
            setTextCursor(cursor);
        } else if ((e->modifiers() & Qt::AltModifier) && e->key() == Qt::Key_L) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Right);
            setTextCursor(cursor);
        } else if (e->key() == Qt::Key_Return) {
            QPlainTextEdit::keyPressEvent(e);
            QString data = this->toPlainText();
            QTextCursor cursor = textCursor();
            int cursorPosition = cursor.position();
            QString insertStr = "";
            bool isBlockStart = false;
            if (data.mid(cursorPosition - 2, 1) == "{")
                isBlockStart = true;
            int i;
            for (i = cursorPosition - 2; i >= 0; i--)
            {
                if (data.mid(i, 1) == "\n")
                    break;
            }
            while (data.mid(i + 1, 1) == "\t")
            {
                insertStr += "\t";
                i++;
            }
            if (isBlockStart)
            {
                QString tempStr = insertStr;
                insertStr = tempStr + "\t\n" + tempStr + "}";
                cursor.insertText(insertStr);
                cursor.movePosition(QTextCursor::PreviousBlock);
                cursor.movePosition(QTextCursor::EndOfBlock);
            } else {
                cursor.insertText(insertStr);
            }
            setTextCursor(cursor);
        } else {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
//! [7]

//! [8]
    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 1
                      || eow.contains(e->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());
    cr.moveTo(cr.x() + lineNumberAreaWidth(), cr.y());
    c->complete(cr); // popup it up!
}

/**
 * @brief LineNumberArea相关
 */

void CodeEditor::setDebugArrowIcon(int lineNo, bool isShow)
{
    m_isShowDebugArrow = isShow;
    m_debugArrowLineNo = lineNo;
    lineNumberArea->update();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 15 + fontMetrics().width(QLatin1Char('9')) * (digits + 2);

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);

    int blockLineNum = textCursor().blockNumber() + 1;
    if (newBlockCount > m_curBlockCount)
    {
        for (int i = 0; i < breakpointLineNoVec.size(); ++i)
            if (breakpointLineNoVec.at(i) >= blockLineNum)
                breakpointLineNoVec.at(i) += 1;
    } else if (newBlockCount < m_curBlockCount){
        for (int i = 0; i < breakpointLineNoVec.size(); ++i)
        {
            if (breakpointLineNoVec.at(i) == blockLineNum + 1) {
                breakpointLineNoVec.erase(breakpointLineNoVec.begin() + i);
                --i;
            } else if (breakpointLineNoVec.at(i) > blockLineNum + 1) {
                breakpointLineNoVec.at(i) -= 1;
            }
        }
    }
    m_curBlockCount = newBlockCount;
}



void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(m_curBlockCount);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(60, 60, 60);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

int CodeEditor::getBreakpointLineNoFromY(int y)
{
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int lineNo = (int)((y - top) / blockBoundingRect(firstVisibleBlock()).height()) + 1 + blockNumber;
    return lineNo;
}


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(5, 70, 120));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    int lineTop = top;
    QFont font;
    font.setFamily(QStringLiteral("Courier"));
    font.setPointSize(13);
    painter.setFont(font);
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::lightGray);
            painter.drawText(-15, top + 1, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }

    int size = (int) blockBoundingRect(firstVisibleBlock()).height();
    for (int i = 0; i < breakpointLineNoVec.size(); ++i)
    {
        QTextBlock block_2 = firstVisibleBlock();
        int blockNumber_2 = block_2.blockNumber();
        int top_2 = (int) blockBoundingGeometry(block_2).translated(contentOffset()).top();
        int breakpointY = (breakpointLineNoVec.at(i) - blockNumber_2 - 1) * size + 3 + top_2;
        painter.setBrush(Qt::red);
        painter.drawEllipse(3, breakpointY, size - 5, size - 5);
    }

    if (m_isShowDebugArrow)
    {
        QTextBlock block_3 = firstVisibleBlock();
        int blockNumber_3 = block_3.blockNumber();
        int top_3 = (int) blockBoundingGeometry(block_3).translated(contentOffset()).top();
        int debugArrowY = (m_debugArrowLineNo - blockNumber_3 - 1) * size + 3 + top_3;
        QPixmap pix(":/resource/images/debug_arrow.png");
        painter.drawPixmap(1, debugArrowY, pix.width(), pix.height(), pix);
    }

    painter.setPen(QPen(Qt::green, 1));
    painter.drawLine(QPointF(lineNumberArea->width() - 10, lineTop), QPointF(lineNumberArea->width() - 10, bottom));
}


void LineNumberArea::mouseMoveEvent(QMouseEvent *event)
{
    if (event->pos().x() < 20 && codeEditor->getBreakpointLineNoFromY(event->pos().y()) <= codeEditor->blockCount())
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(Qt::ArrowCursor);
}

void LineNumberArea::mousePressEvent(QMouseEvent *event)
{
    if (event->pos().x() < 20)
    {
        int lineNo = codeEditor->getBreakpointLineNoFromY(event->pos().y());
        if (lineNo > codeEditor->blockCount())
            return;
        for (int i = 0; i < codeEditor->breakpointLineNoVec.size(); ++i)
        {
            if (lineNo == codeEditor->breakpointLineNoVec.at(i))
            {
                codeEditor->breakpointLineNoVec.erase(codeEditor->breakpointLineNoVec.begin() + i);
                update();
                return;
            }
        }
        codeEditor->breakpointLineNoVec.push_back(codeEditor->getBreakpointLineNoFromY(event->pos().y()));
        update();
    }
}
