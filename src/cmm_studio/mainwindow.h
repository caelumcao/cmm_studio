#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include "project.h"
#include "codeexecute.h"
#include "replwidget.h"
#include "util.h"


namespace Ui {
class MainWindow;
}

class QTreeView;
class QStandardItem;
class ProjectTreeView;
class QStandardItemModel;
class QAbstractItemModel;
class QSlider;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const QString PREFIX;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_action_lexicalAnalysis_triggered();

    void on_action_syntacticAnalysis_triggered();

    void on_action_open_triggered();

    void on_action_codeGene_triggered();

    void openProject(const QModelIndex & modelIndex);     //打开文件内容

    void codeRunFinished();        //代码运行结束

    void output(const QString & str);              //输出运行结果

    void input();                           //输入处理

    void reveiveUserInput(const QString & str);     //接收用户输入字符串

    void on_action_run_triggered();

    void exceptionHappen(const QString & errorStr);

    void on_action_new_triggered();

    void on_action_save_triggered();

    void on_action_saveAs_triggered();

    void on_projectView_customContextMenuRequested(const QPoint &pos);

    void on_openAction_triggered();

    void on_runAction_triggered();

    void on_deleteAction_triggered();

    void on_renameAction_triggered();

    void on_docTabWidget_tabCloseRequested(int index);

    void on_m_projectModel_dataChanged(const QModelIndex & topleft, const QModelIndex & bottomright);

    void on_docTabWidget_currentChanged(int index);

    void setOpacity(int value);

    void on_action_debug_triggered();


    void on_action_singleProcess_triggered();

    void on_action_singleStmt_triggered();

    void on_action_stopDebug_triggered();

    void on_action_exit_triggered();

    void on_action_cut_triggered();

    void on_action_copy_triggered();

    void on_action_paste_triggered();

    void on_replWidget_closed();

private:
    bool lexicalAnalysis();     //词法分析
    bool syntacticAnalysis();   //语法分析
    bool generaterCode();       //中间代码生成

    /**
     * 初始化语法树
     */
    void initTreeView(QStandardItem * pItem, QueueVec & qVec, int level);
    QPixmap initPixmap(int level);

    void addProject(QStandardItem * pItem, const QString & fileName, int type = 0);      //添加项目
    int findProjectIndex(const QModelIndex & modelIndex);     //获取项目索引
    QString getNewFileName();       //获取新建文件名
    void setActionEnabled(bool isEnable);       //设置动作是否可执行
    void setDebug(bool isDebug);        //设置是否在调试
    void setRun(bool isRun);            //设置是否在运行
    void updateDebugArrow(bool isShow);            //更新调试窗口状态

    void updateStackView(const QList<QStringList> & list);      //更新栈窗口
    void updateGlobalView(const QList<QStringList> & list);     //更新全局区窗口
    void updateConstView(const QList<QStringList> & list);      //更新常量区
    void updateCodeView(const QList<QStringList> & list);       //更新代码段

    QAbstractItemModel * modelFromList();



private:
    Ui::MainWindow *ui;
    Util util;
    QStandardItem * m_parentItem;
    QStandardItem * m_projectParentItem;    //项目树父项
    QStandardItemModel * m_projectModel;    //项目树模型
    std::vector<Project> m_projectVec;      //项目vector
    int m_OpenedFileNum;         //打开文件数
    int m_curOpenedFileIndex;    //当前打开项目索引号
    bool m_isRunCurSelectedIndex;     //判断是否是通过右键菜单运行
    bool m_isRename;                //判断是否正在进行重命名操作
    int m_curRunProjectIndex;       //当前正在运行的项目索引
    bool m_isDebug;                 //是否正在调试

    ReplWidget * m_replWidget;    //控制台

    std::vector<QuaternaryExp> m_qeVec;     //中间代码
    CodeExecute m_codeExecute;              //代码运行线程

    int m_fileNameIndex;

    QSlider * transparencySlider;

    QStandardItemModel * m_stackModel;
    QStandardItemModel * m_globalModel;
    QStandardItemModel * m_constModel;
    QStandardItemModel * m_codeModel;
};

#endif // MAINWINDOW_H
