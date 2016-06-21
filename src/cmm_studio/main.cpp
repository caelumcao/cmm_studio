#include "mainwindow.h"
#include "util.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

        QString qss;
        QFile qssFile(":/resource/qss/style.qss");
        qssFile.open(QFile::ReadOnly);
        if(qssFile.isOpen())
        {
            qss = QLatin1String(qssFile.readAll());
            qApp->setStyleSheet(qss);
            qssFile.close();
        } else {
            qDebug() << "can't open file";
        }



    MainWindow w;
    w.show();

    return a.exec();
}
