#ifndef PROJECT_H
#define PROJECT_H
#include <QString>
#include <QModelIndex>

#include "codeeditor.h"

class QCompleter;
class QWidget;

class Project
{
public:
    Project();
    Project(const QString & path, int type = 0);    //0表示打开，1表示新建

    QString name() const;
    void setName(const QString &name);

    QString path() const;
    void setPath(const QString &path);

    bool isSaved() const;
    void setSaved(bool saved);

    int type() const;
    void setType(int type);

    QModelIndex modelIndex() const;
    void setModelIndex(const QModelIndex &modelIndex);

    CodeEditor *editor() const;
    void setEditor(CodeEditor *editor);


public:

    QWidget * m_tabWidget;


private:
    QString m_name;
    QString m_path;
    bool m_saved;
    int m_type;
    CodeEditor * m_editor;
    QModelIndex m_modelIndex;
    QStringList m_wordList;

};

#endif // PROJECT_H
