#-------------------------------------------------
#
# Project created by QtCreator 2014-03-20T13:46:26
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Kaepora
TEMPLATE = app


SOURCES += main.cpp \
    Network/TcpServer.cpp \
    Network/TcpServerThread.cpp \
    Security/BlowFish.cpp \
    Security/CustomRsa.cpp \
    Network/Proto/RedMalk.pb.cc \
    UI/MainWindow.cpp \
    UI/Login.cpp \
    UI/ConfigDialog.cpp

HEADERS  += \
    Network/TcpServer.h \
    Network/TcpServerThread.h \
    Security/BlowFish.h \
    Security/CustomRsa.h \
    Security/SecurityData.h \
    Network/Proto/RedMalk.pb.h \
    UI/MainWindow.h \
    UI/Login.h \
    UI/ConfigDialog.h

FORMS    += \
    UI/MainWindow.ui \
    UI/Login.ui \
    UI/ConfigDialog.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libpqxx

unix:!macx: LIBS += -L$$PWD/Network/Proto/ -lprotobuf

INCLUDEPATH += $$PWD/Network/Proto
DEPENDPATH += $$PWD/Network/Proto

unix:!macx: PRE_TARGETDEPS += $$PWD/Network/Proto/libprotobuf.a
