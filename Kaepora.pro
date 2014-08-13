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
    UI/ConfigDialog.cpp \
    Network/TcpRmRegisterer.cpp \
    DB/DBController.cpp \
    DB/DBFactory.cpp \
    GUIService.cpp \
    Network/TcpRmConnector.cpp \
    Network/TcpRmInvitationListener.cpp \
    Network/TcpRmInvitationAdmin.cpp \
    LocalService.cpp \
    Network/TcpRmInvitationValidator.cpp \
    Network/TcpRmClient.cpp \
    DB/InternalUserData.cpp \
    Network/TcpRmStatus.cpp \
    Network/TcpRmInvitatonResender.cpp \
    Network/TcpRmInvitationEnder.cpp \
    Network/TcpClientThread.cpp \
    Network/TcpRmPersonalNoticesAdmin.cpp \
    Network/TcpRmInvitationResponser.cpp \
    Network/TcpRmPNSender.cpp \
    Network/TcpRmFriendSender.cpp \
    Network/TcpRmLocalFriendServer.cpp

HEADERS  += \
    Network/TcpServer.h \
    Network/TcpServerThread.h \
    Security/BlowFish.h \
    Security/CustomRsa.h \
    Security/SecurityData.h \
    Network/Proto/RedMalk.pb.h \
    UI/MainWindow.h \
    UI/Login.h \
    UI/ConfigDialog.h \
    Network/TcpRmRegisterer.h \
    Network/TcpRmStatus.h \
    DB/BaseDBController.h \
    DB/DBController.h \
    DB/DBFactory.h \
    GUIService.h \
    Network/TcpRmConnector.h \
    Network/TcpRmInvitationListener.h \
    Network/TcpRmInvitationAdmin.h \
    LocalService.h \
    Network/TcpRmInvitationValidator.h \
    Network/TcpRmClient.h \
    DB/InternalUserData.h \
    Network/TcpRmInvitatonResender.h \
    Network/TcpRmInvitationEnder.h \
    Network/TcpClientThread.h \
    Network/TcpRmPersonalNoticesAdmin.h \
    Network/TcpRmInvitationResponser.h \
    Network/TcpRmPNSender.h \
    Network/TcpRmFriendSender.h \
    Network/TcpRmLocalFriendServer.h

FORMS    += \
    UI/MainWindow.ui \
    UI/Login.ui \
    UI/ConfigDialog.ui


unix:!macx: LIBS += -L$$PWD/Network/Proto/ -lprotobuf

INCLUDEPATH += $$PWD/Network/Proto
DEPENDPATH += $$PWD/Network/Proto

unix:!macx: PRE_TARGETDEPS += $$PWD/Network/Proto/libprotobuf.a

unix:!macx: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lpqxx
unix:!macx: LIBS += -L$$PWD/../../../../../usr/local/lib/ -lpq
INCLUDEPATH += $$PWD/../../../../../usr/local/include/pqxx
DEPENDPATH += $$PWD/../../../../../usr/local/include/pqxx

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../../usr/local/lib/libpqxx.a
