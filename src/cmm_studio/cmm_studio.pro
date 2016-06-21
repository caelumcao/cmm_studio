#-------------------------------------------------
#
# Project created by QtCreator 2015-09-30T20:58:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cmm_studio
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    token.cpp \
    lexer.cpp \
    error.cpp \
    treenode.cpp \
    parser.cpp \
    parserexception.cpp \
    highlighter.cpp \
    quaternaryexp.cpp \
    value.cpp \
    symbol.cpp \
    symboltable.cpp \
    codegeneraterexception.cpp \
    codegenerater.cpp \
    common.cpp \
    project.cpp \
    projecttreeview.cpp \
    codeexecute.cpp \
    replwidget.cpp \
    codeexecuteexception.cpp \
    funsymbol.cpp \
    util.cpp \
    codeeditor.cpp

HEADERS  += mainwindow.h \
    token.h \
    lexer.h \
    error.h \
    treenode.h \
    parser.h \
    parserexception.h \
    highlighter.h \
    quaternaryexp.h \
    value.h \
    symbol.h \
    symboltable.h \
    codegeneraterexception.h \
    codegenerater.h \
    common.h \
    project.h \
    projecttreeview.h \
    codeexecute.h \
    replwidget.h \
    codeexecuteexception.h \
    funsymbol.h \
    util.h \
    codeeditor.h

FORMS    += mainwindow.ui

RESOURCES += \
    resource.qrc

RC_ICONS = myico.ico
