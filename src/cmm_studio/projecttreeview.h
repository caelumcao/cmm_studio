#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H
#include <QTreeView>
#include <QModelIndex>

class ProjectTreeView : public QTreeView
{
Q_OBJECT
public:
    explicit ProjectTreeView(QWidget *parent = 0);

    void mouseDoubleClickEvent(QMouseEvent * event);

signals:
    void mouseDoubleClicked(const QModelIndex & modelIndex);

};

#endif // PROJECTTREEVIEW_H
