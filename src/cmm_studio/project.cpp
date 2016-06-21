#include "project.h"
#include <QFileInfo>

Project::Project()
{

}

Project::Project(const QString &path, int type)
{
    m_type = type;
    m_path = path;
    if (m_type == 0)
        m_name = QFileInfo(path).fileName();
    else
        m_name = path;
    m_saved = true;
    m_editor = NULL;
    m_tabWidget = NULL;
    m_wordList << "if" << "else" << "for" << "while" << "read"
               << "write" << "void" << "int" << "real" << "char"
               << "return" << "main";
}

QString Project::name() const
{
    return m_name;
}

void Project::setName(const QString &name)
{
    m_name = name;
}
QString Project::path() const
{
    return m_path;
}

void Project::setPath(const QString &path)
{
    m_path = path;
}

bool Project::isSaved() const
{
    return m_saved;
}

void Project::setSaved(bool saved)
{
    m_saved = saved;
}
int Project::type() const
{
    return m_type;
}

void Project::setType(int type)
{
    m_type = type;
}
QModelIndex Project::modelIndex() const
{
    return m_modelIndex;
}

void Project::setModelIndex(const QModelIndex &modelIndex)
{
    m_modelIndex = modelIndex;
}

CodeEditor *Project::editor() const
{
    return m_editor;
}

void Project::setEditor(CodeEditor *editor)
{
    m_editor = editor;
}








