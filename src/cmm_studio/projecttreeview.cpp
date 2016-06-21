#include "projecttreeview.h"
#include <QMouseEvent>
#include <QDebug>

ProjectTreeView::ProjectTreeView(QWidget *parent) : QTreeView(parent)
{

}

void ProjectTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QModelIndex index = currentIndex();
        if (index.parent().data().toString() == "examples" || index.parent().data().toString() == "projects")
            emit mouseDoubleClicked(index);
        else
            QTreeView::mouseDoubleClickEvent(event);
    }
}
