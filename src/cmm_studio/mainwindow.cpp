#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "highlighter.h"
#include <QTreeView>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include "projecttreeview.h"
#include <QSplitter>
#include <QStringListModel>
#include <QGridLayout>
#include <QMessageBox>
#include <QCompleter>
#include <QStyledItemDelegate>
#include <QSlider>
#include <QLabel>
#include <QSpacerItem>
#include <QAbstractItemView>
#include "common.h"
#include <QDebug>

const QString MainWindow::PREFIX = "cmmdemo";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_OpenedFileNum = 0;
    m_curOpenedFileIndex = -1;
    m_fileNameIndex = 1;
    m_isRunCurSelectedIndex = false;
    m_isRename = true;
    m_curRunProjectIndex = -1;
    m_isDebug = false;

    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    setActionEnabled(false);

//    setWindowFlags(Qt::FramelessWindowHint);
//    setAttribute(Qt::WA_TranslucentBackground);

    //初始化透明控制条
    QWidget * spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainToolBar->addWidget(spacer);
    QLabel * transparencyLabel=new QLabel("透明度：");
    transparencyLabel->setStyleSheet("*{font-size: 14px; color: lightGray;}");
    transparencySlider = new QSlider(Qt::Horizontal);      // 设置透明度的滚动条
    transparencySlider->setFixedWidth(150);
    transparencySlider->setRange(70,99);
    transparencySlider->setValue(99);
    ui->mainToolBar->addWidget(transparencyLabel);
    ui->mainToolBar->addWidget(transparencySlider);
    QWidget * spacer_2 = new QWidget(this);
    spacer_2->setMinimumWidth(40);
    ui->mainToolBar->addWidget(spacer_2);


    //初始化语法树treeView
    QStandardItemModel * model = new QStandardItemModel(this);
    ui->syntacticTreeView->setModel(model);
    m_parentItem = model->invisibleRootItem();

    //初始化项目treeView
    m_projectModel = new QStandardItemModel(this);
    ui->projectView->setModel(m_projectModel);
    ui->projectView->setIconSize(QSize(20, 20));

    m_projectParentItem = m_projectModel->invisibleRootItem();
    QStandardItem * item_1 = new QStandardItem;
    QStandardItem * item_2 = new QStandardItem;
    QFont font;
    font.setPointSize(12);
    item_1->setFont(font);
    item_2->setFont(font);
    item_1->setText("examples");
    item_2->setText("projects");
    item_1->setIcon(QIcon(":/resource/images/dir.png"));
    item_2->setIcon(QIcon(":/resource/images/dir.png"));
    m_projectParentItem->appendRow(item_1);
    m_projectParentItem->appendRow(item_2);
    //遍历目录
    QDir dir(":/resource/examples");
    foreach (QFileInfo fi, dir.entryInfoList())
    {
        if (fi.isFile())
            addProject(item_1, fi.filePath());
    }
    ui->projectView->expandAll();

    //初始化栈tableView
    m_stackModel = new QStandardItemModel(this);
    m_stackModel->setColumnCount(4);
    m_stackModel->setHeaderData(0, Qt::Horizontal, tr("地址"));
    m_stackModel->setHeaderData(1, Qt::Horizontal, tr("值"));
    m_stackModel->setHeaderData(2, Qt::Horizontal, tr("类型"));
    m_stackModel->setHeaderData(3, Qt::Horizontal, tr("注释"));
    ui->stackView->setModel(m_stackModel);
    ui->stackView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //初始化全局区tableView
    m_globalModel = new QStandardItemModel(this);
    m_globalModel->setColumnCount(4);
    m_globalModel->setHeaderData(0, Qt::Horizontal, tr("地址"));
    m_globalModel->setHeaderData(1, Qt::Horizontal, tr("值"));
    m_globalModel->setHeaderData(2, Qt::Horizontal, tr("类型"));
    m_globalModel->setHeaderData(3, Qt::Horizontal, tr("注释"));
    ui->globalView->setModel(m_globalModel);
    ui->globalView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //初始化常量区tableView
    m_constModel = new QStandardItemModel(this);
    m_constModel->setColumnCount(3);
    m_constModel->setHeaderData(0, Qt::Horizontal, tr("地址"));
    m_constModel->setHeaderData(1, Qt::Horizontal, tr("值"));
    m_constModel->setHeaderData(2, Qt::Horizontal, tr("类型"));
    ui->constView->setModel(m_constModel);
    ui->constView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //初始化代码段tableView
    m_codeModel = new QStandardItemModel(this);
    m_codeModel->setColumnCount(5);
    m_codeModel->setHeaderData(0, Qt::Horizontal, tr("地址"));
    m_codeModel->setHeaderData(1, Qt::Horizontal, tr("项1"));
    m_codeModel->setHeaderData(2, Qt::Horizontal, tr("项2"));
    m_codeModel->setHeaderData(3, Qt::Horizontal, tr("项3"));
    m_codeModel->setHeaderData(4, Qt::Horizontal, tr("项4"));
    ui->codeView->setModel(m_codeModel);
    ui->codeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->action_cut->setEnabled(false);
    ui->action_copy->setEnabled(false);
    ui->action_paste->setEnabled(false);


    ui->splitter->setStretchFactor(0, 200);
    ui->splitter->setStretchFactor(1, 10);
    ui->splitter_2->setStretchFactor(0, 70);
    ui->splitter_2->setStretchFactor(1, 0);
    ui->splitter_3->setStretchFactor(0, 0);
    ui->splitter_3->setStretchFactor(1, 70);

    connect(ui->projectView, SIGNAL(mouseDoubleClicked(QModelIndex)), this, SLOT(openProject(QModelIndex)));
    connect(&m_codeExecute, SIGNAL(finished()), this, SLOT(codeRunFinished()));
    connect(&m_codeExecute, SIGNAL(sig_exception(QString)), this, SLOT(exceptionHappen(QString)));
    connect(&m_codeExecute, SIGNAL(sig_output(QString)), this, SLOT(output(QString)));
    connect(&m_codeExecute, SIGNAL(sig_input()), this, SLOT(input()));
    //connect(&m_codeExecute, SIGNAL(sig_debugFinish()), this, SLOT(on_action_stopDebug_triggered()));
    connect(m_projectModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(on_m_projectModel_dataChanged(QModelIndex,QModelIndex)));
    connect(transparencySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


//词法分析
void MainWindow::on_action_lexicalAnalysis_triggered()
{
    lexicalAnalysis();
}

//语法分析
void MainWindow::on_action_syntacticAnalysis_triggered()
{
    syntacticAnalysis();
}

//打开文件
void MainWindow::on_action_open_triggered()
{
    QStringList fileNameList = QFileDialog::getOpenFileNames(this, tr("Open File"), "", "CMM Files (*.cmm);;所有文件 (*.*)");
    foreach (QString fileName, fileNameList)
    {
        bool isAlreadyOpen = false;
        for (int i = 0; i < m_projectVec.size(); ++i)
        {
            Project project = m_projectVec.at(i);
            if (project.path() == fileName)
            {
                isAlreadyOpen = true;
                QMessageBox::warning(this, tr("cmm_studio"),
                                     tr("can't open file %1: the file is already opened in projects")
                                     .arg(fileName));
                break;
            }
        }
        if (!isAlreadyOpen)
            addProject(m_projectParentItem->child(1), fileName);
    }
}

//中间代码生成
void MainWindow::on_action_codeGene_triggered()
{
    generaterCode();
}

//新建、打开文件窗口
void MainWindow::openProject(const QModelIndex &modelIndex)
{
    int previousIndex = m_curOpenedFileIndex;
    m_curOpenedFileIndex = findProjectIndex(modelIndex);
    if (m_projectVec.at(m_curOpenedFileIndex).m_tabWidget != NULL)  //已经打开
    {
        ui->docTabWidget->setCurrentWidget(m_projectVec.at(m_curOpenedFileIndex).m_tabWidget);
        return;
    }
    QFile file;
    if (m_projectVec.at(m_curOpenedFileIndex).type() == 0)
    {
        file.setFileName(m_projectVec.at(m_curOpenedFileIndex).path());
        if (!file.open(QFile::ReadOnly | QFile::Text))  //打开失败
        {
            QMessageBox::warning(this, tr("cmm_studio"),
                                 tr("can't open file %1:\n%2.")
                                 .arg(m_projectVec.at(m_curOpenedFileIndex).name()).arg(file.errorString()));
            m_curOpenedFileIndex = previousIndex;
            return;
        }
    }
    int index = 0;
    QWidget * tab;
    if (m_OpenedFileNum != 0)
    {
        tab = new QWidget;
        index = ui->docTabWidget->addTab(tab, "");
    } else {
        setActionEnabled(true);
    }
    ui->docTabWidget->setCurrentIndex(index);
    tab = ui->docTabWidget->currentWidget();
    m_projectVec.at(m_curOpenedFileIndex).m_tabWidget = tab;    //设置project所指向的widget
    ui->docTabWidget->setTabText(index, m_projectVec.at(m_curOpenedFileIndex).name());
    CodeEditor * codeEditor = new CodeEditor(tab);

    m_projectVec.at(m_curOpenedFileIndex).setEditor(codeEditor);    //设置project所指向的codeEditor
    if (m_projectVec.at(m_curOpenedFileIndex).type() == 0)
        codeEditor->loadFile(file);
    QGridLayout * gridLayout = new QGridLayout(tab);
    gridLayout->setMargin(0);
    gridLayout->addWidget(codeEditor);
    Highlighter * highlighter = new Highlighter(codeEditor->document());
    m_OpenedFileNum++;

    ui->action_paste->setEnabled(true);
    connect(codeEditor, SIGNAL(copyAvailable(bool)), ui->action_cut, SLOT(setEnabled(bool)));
    connect(codeEditor, SIGNAL(copyAvailable(bool)), ui->action_copy, SLOT(setEnabled(bool)));
}

//代码运行结束处理
void MainWindow::codeRunFinished()
{
    //m_curRunProjectIndex = -1;
    if (m_codeExecute.isDebugFinished())
    {
        m_codeExecute.setIsDebugFinished(0);
        on_action_stopDebug_triggered();
    } else if (m_codeExecute.isRunFinished()) {
        m_codeExecute.setIsRunFinished(0);
        ui->outputBrowser->append("正常退出!");
        setRun(false);
        if (m_replWidget != NULL)
            m_replWidget->finishOutput();
    } else if (m_codeExecute.isExceptionHappened()) {
        m_codeExecute.setIsExceptionHappened(0);
    } else if (m_isDebug) {
        updateDebugArrow(true);
        updateStackView(m_codeExecute.getStackData());
        updateGlobalView(m_codeExecute.getGlobalData());
        updateConstView(m_codeExecute.getConstData());
    } else {
        setDebug(false);
        setRun(false);
        QMessageBox::critical(this, tr("Error"), "unknow error!", QMessageBox::Abort);
    }
}

//输出代码运行结果
void MainWindow::output(const QString &str)
{
    m_replWidget->append(str);
}

//输入请求处理
void MainWindow::input()
{
    m_replWidget->startInput();
}

//接收用户输入字符串
void MainWindow::reveiveUserInput(const QString &str)
{
    m_codeExecute.inputToStream(str.toStdString());
}

//运行代码
void MainWindow::on_action_run_triggered()
{
    if (m_isRunCurSelectedIndex)
    {
        QModelIndex index = ui->projectView->currentIndex();
        m_curRunProjectIndex = findProjectIndex(index);
    } else {
        m_curRunProjectIndex = m_curOpenedFileIndex;
    }
    Project project = m_projectVec.at(m_curRunProjectIndex);
    project.editor()->setIsAutoRun(true);
    if (generaterCode())
    {
        setRun(true);
        ui->outputBrowser->append("开始运行!");
        m_replWidget = new ReplWidget;
        m_replWidget->show();
        connect(m_replWidget, SIGNAL(sendUserInput(QString)), SLOT(reveiveUserInput(QString)));
        connect(m_replWidget, SIGNAL(replWidgetClosed()), SLOT(on_replWidget_closed()));
        if (m_codeExecute.init(m_qeVec))
            m_codeExecute.start();
    }
    project.editor()->updateErrorStatus();
    project.editor()->setIsAutoRun(false);
    m_isRunCurSelectedIndex = false;
}

//代码执行发生异常
void MainWindow::exceptionHappen(const QString &errorStr)
{

    ui->outputBrowser->append("异常退出!");
    QMessageBox::critical(this, tr("Exception"), errorStr, QMessageBox::Abort);
    if (m_isDebug) {
        setDebug(false);
        on_action_stopDebug_triggered();
    } else {
        setRun(false);
        if (m_replWidget != NULL)
            m_replWidget->deleteLater();
    }
}

//新建文件
void MainWindow::on_action_new_triggered()
{
    QString newFileName = getNewFileName();
    addProject(m_projectParentItem->child(1), newFileName, 1);

    QModelIndex index = m_projectVec.back().modelIndex();
    openProject(index);
}

//保存文件
void MainWindow::on_action_save_triggered()
{
    Project tempProject = m_projectVec.at(m_curOpenedFileIndex);
    if (tempProject.type() == 1)    //代表新建文件，未被保存过
    {
        on_action_saveAs_triggered();
        return;
    }
    if (tempProject.editor()->saveFile(tempProject.path()))
    {
        m_projectVec.at(m_curOpenedFileIndex).setType(0);
        ui->statusBar->showMessage("文件保存成功", 2000);
    }
}

//另存为
void MainWindow::on_action_saveAs_triggered()
{
    // 获取文件路径，如果为空，则返回false
    Project tempProject = m_projectVec.at(m_curOpenedFileIndex);
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"), tempProject.path(), "CMM Files (*.cmm)");
    if (fileName.isEmpty())
        return;
    if (tempProject.editor()->saveFile(fileName))
    {
        if (tempProject.type() == 1)    //如果是新建的文件
        {
            m_projectVec.at(m_curOpenedFileIndex).setPath(fileName);
            m_projectVec.at(m_curOpenedFileIndex).setName(QFileInfo(fileName).fileName());
            m_projectVec.at(m_curOpenedFileIndex).setType(0);
            QModelIndex index = m_projectVec.at(m_curOpenedFileIndex).modelIndex();
            if (QFileInfo(fileName).fileName() != index.data().toString())
                m_isRename = false;
            m_projectParentItem->child(1)->child(index.row())->setText(QFileInfo(fileName).fileName());
            ui->docTabWidget->setTabText(ui->docTabWidget->indexOf(m_projectVec.at(m_curOpenedFileIndex).m_tabWidget), QFileInfo(fileName).fileName());
        }
        ui->statusBar->showMessage("文件保存成功", 2000);
    }
}

//右击TreeView事件
void MainWindow::on_projectView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->projectView->currentIndex();
    if (index.parent().data().toString() != "examples" && index.parent().data().toString() != "projects")
        return;
    QMenu * menu = new QMenu;
    QAction * openAction = new QAction(QStringLiteral("打开"), menu);
    QAction * runAction = new QAction(QStringLiteral("运行"), menu);
    QAction * deleteAction = new QAction(QStringLiteral("删除"), menu);
    QAction * renameAction = new QAction(QStringLiteral("重命名"), menu);
    connect(openAction, SIGNAL(triggered()), this, SLOT(on_openAction_triggered()));
    connect(runAction, SIGNAL(triggered()), this, SLOT(on_runAction_triggered()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(on_deleteAction_triggered()));
    connect(renameAction, SIGNAL(triggered()), this, SLOT(on_renameAction_triggered()));
    menu->addAction(openAction);
    menu->addAction(runAction);
    menu->addAction(deleteAction);
    menu->addAction(renameAction);
    if (index.parent().data().toString() == "examples")
    {
        deleteAction->setEnabled(false);
        renameAction->setEnabled(false);
    }
    int projectIndex = findProjectIndex(index);
    if (projectIndex == m_curRunProjectIndex)
        deleteAction->setEnabled(false);
    menu->exec(QCursor::pos());
    delete menu;
}

//打开文件动作（右键菜单）
void MainWindow::on_openAction_triggered()
{
    QModelIndex index = ui->projectView->currentIndex();
    openProject(index);
}

//运行动作（右键菜单）
void MainWindow::on_runAction_triggered()
{
    m_isRunCurSelectedIndex = true;
    on_action_run_triggered();
}

//删除文件动作
void MainWindow::on_deleteAction_triggered()
{
    QModelIndex index = ui->projectView->currentIndex();
    int projectIndex = findProjectIndex(index);
    int tabIndex = ui->docTabWidget->indexOf(m_projectVec.at(projectIndex).m_tabWidget);
    on_docTabWidget_tabCloseRequested(tabIndex);
    m_projectParentItem->child(1)->removeRow(index.row());
    m_projectVec.erase(m_projectVec.begin() + projectIndex);
    int rowIndex = index.row();
    for (int i = projectIndex; i < m_projectVec.size(); ++i)
    {
        QModelIndex tempIndex = m_projectParentItem->child(1)->child(rowIndex++)->index();
        m_projectVec.at(i).setModelIndex(tempIndex);
    }
}

//重命名操作
void MainWindow::on_renameAction_triggered()
{
    QModelIndex index = ui->projectView->currentIndex();
    ui->projectView->edit(index);
}

//关闭文件editor
void MainWindow::on_docTabWidget_tabCloseRequested(int index)
{
    if (m_OpenedFileNum == 0)
        return;
    QWidget * tab = ui->docTabWidget->widget(index);
    int projectIndex = 0;
    for (projectIndex = m_projectVec.size() - 1; projectIndex >= 0; --projectIndex)
    {
        Project project = m_projectVec.at(projectIndex);
        if (project.m_tabWidget == tab)
            break;
    }
    if (projectIndex == m_curRunProjectIndex && m_isDebug)
        return;
    ui->docTabWidget->removeTab(index);
    tab->deleteLater();
    m_projectVec.at(projectIndex).m_tabWidget = NULL;
    m_projectVec.at(projectIndex).setEditor(NULL);
    m_OpenedFileNum--;
    if (m_OpenedFileNum == 0)
    {
        setActionEnabled(false);
        ui->action_cut->setEnabled(false);
        ui->action_copy->setEnabled(false);
        ui->action_paste->setEnabled(false);
        QWidget * tab = new QWidget;
        ui->docTabWidget->addTab(tab, "no documents");
    }
}

//文件名称改变处理
void MainWindow::on_m_projectModel_dataChanged(const QModelIndex &topleft, const QModelIndex &bottomright)
{
    if (!m_isRename)
    {
        m_isRename = true;
        return;
    }
    QString fileName = topleft.data().toString();
    int projectIndex = findProjectIndex(topleft);
    Project project = m_projectVec.at(projectIndex);
    QFile file(project.path());
    if (!file.rename(QFileInfo(project.path()).path() + "/" + fileName))
    {
        QMessageBox::warning(this, tr("cmm_studio"),
                             tr("file %1: rename failed!\n%2.")
                             .arg(project.name()).arg(file.errorString()));
        m_isRename = false;
        m_projectParentItem->child(1)->child(topleft.row())->setText(project.name());
        return;
    }
    m_projectVec.at(projectIndex).setPath(file.fileName());
    m_projectVec.at(projectIndex).setName(fileName);
    if (m_projectVec.at(projectIndex).m_tabWidget != NULL)
        ui->docTabWidget->setTabText(ui->docTabWidget->indexOf(m_projectVec.at(projectIndex).m_tabWidget), fileName);
}

//文件窗口改变
void MainWindow::on_docTabWidget_currentChanged(int index)
{
    for (int i = 0; i < m_projectVec.size(); ++i)
    {
        Project project = m_projectVec.at(i);
        if (project.m_tabWidget == ui->docTabWidget->widget(index))
        {
            m_curOpenedFileIndex = i;
        }
    }
}

//设置透明度
void MainWindow::setOpacity(int value)
{
    qreal opacity = qreal(value) / 100.0;
    setWindowOpacity(opacity);
}

//开始调试
void MainWindow::on_action_debug_triggered()
{
    if (!m_isDebug)
    {
        m_curRunProjectIndex = m_curOpenedFileIndex;
        Project project = m_projectVec.at(m_curRunProjectIndex);
        project.editor()->setIsAutoRun(true);
        if (generaterCode())
        {
            ui->outputBrowser->append("调试开始!");
            m_replWidget = new ReplWidget;
            m_replWidget->setWindowFlags(windowFlags() &~ Qt::WindowCloseButtonHint);
            m_replWidget->show();
            this->activateWindow();
            connect(m_replWidget, SIGNAL(sendUserInput(QString)), SLOT(reveiveUserInput(QString)));
            connect(m_replWidget, SIGNAL(replWidgetClosed()), SLOT(on_replWidget_closed()));
            if (m_codeExecute.init(m_qeVec))
            {
                Project project = m_projectVec.at(m_curRunProjectIndex);
                m_codeExecute.setBreakpointLineNoVec(project.editor()->breakpointLineNoVec);
                m_codeExecute.setRunMode(1);
                updateCodeView(m_codeExecute.getCodeData());
                m_codeExecute.start();
                setDebug(true);
            }
        }
        project.editor()->updateErrorStatus();
        project.editor()->setIsAutoRun(false);
    } else {
        Project project = m_projectVec.at(m_curRunProjectIndex);
        m_codeExecute.setBreakpointLineNoVec(project.editor()->breakpointLineNoVec);
        m_codeExecute.setRunMode(1);
        m_codeExecute.start();
    }
}

//逐过程
void MainWindow::on_action_singleProcess_triggered()
{
    m_codeExecute.setRunMode(2);
    m_codeExecute.start();
}

//逐语句
void MainWindow::on_action_singleStmt_triggered()
{
    m_codeExecute.setRunMode(3);
    m_codeExecute.start();
}

//停止调试
void MainWindow::on_action_stopDebug_triggered()
{
    if (m_codeExecute.isRunning())
        m_codeExecute.setIsOver(true);
    ui->outputBrowser->append("调试结束!");
    setDebug(false);
    updateDebugArrow(false);
    QList<QStringList> list;
    list.clear();
    updateStackView(list);
    updateGlobalView(list);
    updateConstView(list);
    updateCodeView(list);
    if (m_replWidget != NULL)
        m_replWidget->deleteLater();
}


//词法分析
bool MainWindow::lexicalAnalysis()
{
    QStringList lexerOutputList;
    QString text = "";
    if (m_isRunCurSelectedIndex)
    {
        QModelIndex index = ui->projectView->currentIndex();
        m_curRunProjectIndex = findProjectIndex(index);
        text = m_projectVec.at(m_curRunProjectIndex).editor()->toPlainText();
    } else {
        m_curRunProjectIndex = m_curOpenedFileIndex;
        text = m_projectVec.at(m_curOpenedFileIndex).editor()->toPlainText();
    }
    bool b_result = util.lexicalAnalysisProcess(text, lexerOutputList);
    if (b_result)
        ui->outputBrowser->setText("词法分析成功!");
    else
        ui->outputBrowser->setText("词法分析失败!");
    ui->lexerBrowser->clear();
    for (int i = 0; i < lexerOutputList.size(); ++i)
        ui->lexerBrowser->append(lexerOutputList.at(i));
    return b_result;
}

//语法分析
bool MainWindow::syntacticAnalysis()
{
    if (!lexicalAnalysis())
        return false;
    std::vector<QueueVec> outputVec;
    std::string errorStr = "";
    bool b_result = util.syntacticAnalysisProcess(outputVec, errorStr);
    if (b_result)
        ui->outputBrowser->append("语法分析成功!");
    else
        ui->outputBrowser->append("语法分析失败!\n " + QString::fromStdString(errorStr));

    //释放资源
    int count = m_parentItem->rowCount();
    for (int i = 0; i < count; ++i)
        m_parentItem->removeRow(0);

    //创建视图
    for (int i = 0; i < outputVec.size(); ++i)
    {
        QueueVec qVec = outputVec.at(i);
        initTreeView(m_parentItem, qVec, 0);
    }

    return b_result;
}

//中间代码生成
bool MainWindow::generaterCode()
{
    if (!syntacticAnalysis())
        return false;
    m_qeVec.clear();
    std::string errorStr = "";
    bool b_result = util.generaterCodeProcess(m_qeVec, errorStr);
    if (b_result)
        ui->outputBrowser->append("中间代码生成成功!");
    else
        ui->outputBrowser->append("中间代码生成失败!" + QString::fromStdString(errorStr));
    ui->codeGeneBrowser->clear();
    for (int i = 0; i < m_qeVec.size(); ++i)
    {
        std::string tempStr = m_qeVec.at(i).toString();
        int pos = 0;
        while ((pos = tempStr.find("\n", pos)) != std::string::npos)
            tempStr.replace(pos, 1, "\\n");
        ui->codeGeneBrowser->append(QString::fromStdString(tempStr));
    }
    return b_result;

}

//初始化语法树
void MainWindow::initTreeView(QStandardItem *pItem, QueueVec &qVec, int level)
{
    if (qVec.at(level).size() > 0)
    {
        TreeNodeStr tns = qVec.at(level).front();
        qVec.at(level).pop();
        QStandardItem * item = new QStandardItem;
        QFont font;
        font.setPointSize(12);
        item->setFont(font);
        item->setText(QString::fromStdString(tns.value));
        item->setIcon(QIcon(initPixmap(level)));
        pItem->appendRow(item);
        for (int i = 0; i < tns.childNum; ++i)
            initTreeView(item, qVec, level + 1);
    }
}

//初始化语法树方块颜色
QPixmap MainWindow::initPixmap(int level)
{
    QPixmap pixmap(40, 40);
    switch (level % 8) {
    case 0:
        pixmap.fill(Qt::green);
        break;
    case 1:
        pixmap.fill(Qt::blue);
        break;
    case 2:
        pixmap.fill(Qt::darkYellow);
        break;
    case 3:
        pixmap.fill(Qt::gray);
        break;
    case 4:
        pixmap.fill(Qt::red);
        break;
    case 5:
        pixmap.fill(Qt::cyan);
        break;
    case 6:
        pixmap.fill(Qt::yellow);
        break;
    case 7:
        pixmap.fill(Qt::white);
        break;
    default:
        break;
    }
    return pixmap;
}

//添加项目
void MainWindow::addProject(QStandardItem *pItem, const QString &fileName, int type)
{
    Project project(fileName, type);
    QStandardItem * item = new QStandardItem;
    QFont font;
    font.setPointSize(12);
    item->setFont(font);
    item->setText(project.name());
    item->setIcon(QIcon(":/resource/images/file.png"));
    pItem->appendRow(item);
    QModelIndex pModelIndex;
    if (pItem == m_projectParentItem->child(0))
        pModelIndex = m_projectModel->index(0, 0, QModelIndex());
    else
        pModelIndex = m_projectModel->index(1, 0, QModelIndex());
    QModelIndex modelIndex = m_projectModel->index(item->row(), 0, pModelIndex);
    project.setModelIndex(modelIndex);
    m_projectVec.push_back(project);
}

//获取项目索引
int MainWindow::findProjectIndex(const QModelIndex &modelIndex)
{
    for (int i = 0; i < m_projectVec.size(); ++i)
        if (m_projectVec.at(i).modelIndex() == modelIndex)
            return i;
    return -1;
}

//获取新建文件名
QString MainWindow::getNewFileName()
{
    QString name;
    bool isExist = true;
    while (isExist)
    {
        isExist = false;
        name = QString("%1%2.cmm").arg(PREFIX).arg(m_fileNameIndex);
        for (int i = 0; i < m_projectVec.size(); ++i)
        {
            Project tempPro = m_projectVec.at(i);
            if (tempPro.name() == name)
            {
                isExist = true;
                break;
            }
        }
        m_fileNameIndex++;
    }
    return name;
}

//设置动作是否可执行
void MainWindow::setActionEnabled(bool isEnable)
{

    ui->action_save->setEnabled(isEnable);
    ui->action_saveAs->setEnabled(isEnable);
    ui->action_debug->setEnabled(isEnable);
    setDebug(false);
    ui->action_run->setEnabled(isEnable);
    ui->action_codeGene->setEnabled(isEnable);
    ui->action_lexicalAnalysis->setEnabled(isEnable);
    ui->action_syntacticAnalysis->setEnabled(isEnable);
}

//设置是否在调试
void MainWindow::setDebug(bool isDebug)
{
    m_isDebug = isDebug;
    ui->action_singleProcess->setEnabled(isDebug);
    ui->action_singleStmt->setEnabled(isDebug);
    ui->action_stopDebug->setEnabled(isDebug);
    ui->action_run->setEnabled(!isDebug);
    ui->action_codeGene->setEnabled(!isDebug);
    ui->action_lexicalAnalysis->setEnabled(!isDebug);
    ui->action_syntacticAnalysis->setEnabled(!isDebug);
    if (m_OpenedFileNum > 0 && m_curRunProjectIndex >= 0 && m_curRunProjectIndex < m_projectVec.size())
    {
        Project project = m_projectVec.at(m_curRunProjectIndex);
        project.editor()->setReadOnly(isDebug);
    }
}

//设置是否在运行
void MainWindow::setRun(bool isRun)
{
    ui->action_run->setEnabled(!isRun);
    ui->action_debug->setEnabled(!isRun);
    ui->action_codeGene->setEnabled(!isRun);
    ui->action_lexicalAnalysis->setEnabled(!isRun);
    ui->action_syntacticAnalysis->setEnabled(!isRun);
    if (m_OpenedFileNum > 0 && m_curRunProjectIndex >= 0 && m_curRunProjectIndex < m_projectVec.size())
    {
        Project project = m_projectVec.at(m_curRunProjectIndex);
        project.editor()->setReadOnly(isRun);
    }
}

//更新调试窗口状态
void MainWindow::updateDebugArrow(bool isShow)
{
    Project project = m_projectVec.at(m_curRunProjectIndex);
    project.editor()->setDebugArrowIcon(nextExecuteStmtLineNo, isShow);
}

//更新栈窗口
void MainWindow::updateStackView(const QList<QStringList> &list)
{
    m_stackModel->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i)
    {
        for (int j = 0; j < list.at(i).size(); ++j)
        {
            m_stackModel->setData(m_stackModel->index(i, j), list.at(i).at(j));
            m_stackModel->item(i, j)->setTextAlignment(Qt::AlignCenter);
        }
    }
}

//更新全局区窗口
void MainWindow::updateGlobalView(const QList<QStringList> &list)
{
    m_globalModel->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i)
    {
        for (int j = 0; j < list.at(i).size(); ++j)
        {
            m_globalModel->setData(m_globalModel->index(i, j), list.at(i).at(j));
            m_globalModel->item(i, j)->setTextAlignment(Qt::AlignCenter);
        }
    }
}

//更新常量区
void MainWindow::updateConstView(const QList<QStringList> &list)
{
    m_constModel->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i)
    {
        for (int j = 0; j < list.at(i).size(); ++j)
        {
            m_constModel->setData(m_constModel->index(i, j), list.at(i).at(j));
            m_constModel->item(i, j)->setTextAlignment(Qt::AlignCenter);
        }
    }
}

//更新代码段
void MainWindow::updateCodeView(const QList<QStringList> &list)
{
    m_codeModel->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i)
    {
        for (int j = 0; j < list.at(i).size(); ++j)
        {
            m_codeModel->setData(m_codeModel->index(i, j), list.at(i).at(j));
            m_codeModel->item(i, j)->setTextAlignment(Qt::AlignCenter);
        }
    }
}



void MainWindow::on_action_exit_triggered()
{
    qApp->closeAllWindows();
}

void MainWindow::on_action_cut_triggered()
{
    Project project = m_projectVec.at(m_curOpenedFileIndex);
    project.editor()->cut();
}

void MainWindow::on_action_copy_triggered()
{
    Project project = m_projectVec.at(m_curOpenedFileIndex);
    project.editor()->copy();
}

void MainWindow::on_action_paste_triggered()
{
    Project project = m_projectVec.at(m_curOpenedFileIndex);
    project.editor()->paste();
}

void MainWindow::on_replWidget_closed()
{
    if (m_codeExecute.isRunning())
        m_codeExecute.setIsOver(true);
}
