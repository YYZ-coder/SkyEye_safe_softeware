#-------------------------------------------------
#
# Project created by QtCreator 2017-03-19T16:16:06
#
#-------------------------------------------------

QT       += core gui
QT       += sql
LIBS     += -L/path/to -lpsapi

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SkyEye
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    conmysql.cpp \
    ndis_operate.cpp \
    wfp_operate.cpp \
    ssdt_operate.cpp \
    sqlmanager.cpp \
    verify.cpp \
    processopera_setting.cpp \
    netheadopera_setting.cpp \
    multibtn.cpp \
    wholefileopera_setting.cpp \
    blacktablelist.cpp \
    register.cpp \
    pipe_operation.cpp

HEADERS  += mainwindow.h \
    conmysql.h \
    ndis_operate.h \
    wfp_operate.h \
    conndriver.h \
    ssdt_operate.h \
    sqlmanager.h \
    verify.h \
    processopera_setting.h \
    netheadopera_setting.h \
    multibtn.h \
    wholefileopera_setting.h \
    blacktablelist.h \
    register.h \
    pipe_operation.h

DISTFILES +=

RESOURCES += \
    sorc.qrc

RC_FILE = icon.rc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-SkyEye-unknown-Release/release/ -llibmysql
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-SkyEye-unknown-Release/debug/ -llibmysql
else:unix: LIBS += -L$$PWD/../build-SkyEye-unknown-Release/ -llibmysql

INCLUDEPATH += $$PWD/../build-SkyEye-unknown-Release/release/include
DEPENDPATH += $$PWD/../build-SkyEye-unknown-Release/release/include
