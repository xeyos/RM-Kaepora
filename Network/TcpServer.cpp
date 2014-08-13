#include <QThread>
#include "TcpServerThread.h"
#include "TcpClientThread.h"
#include "TcpServer.h"

TcpServer::TcpServer(QObject *parent) :
    QTcpServer(parent)
{
    crsa = new CustomRsa();
    keysS = crsa->getKeys();
    serverMode = 0;
}

bool TcpServer::startServer()
{
    int port = 1432;
    if(!this->listen(QHostAddress::Any, port))
        return false;
    else
        return true;
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    TcpServerThread* worker = new TcpServerThread(socketDescriptor, &keysS, crsa, serverMode);
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    //connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(thread, SIGNAL(started()), worker, SLOT(work()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(worker, SIGNAL(clientThreadNeeded(int,QByteArray,QString)), this, SLOT(clientThreadNeeded(int,QByteArray,QString)));
    connect(worker, SIGNAL(clientThreadNeeded(int,QByteArray)), this, SLOT(clientThreadNeeded(int,QByteArray)));
    thread->start();
}

void TcpServer::clientThreadNeeded(int type, QByteArray data, QString destinatary){
    TcpClientThread* worker = new TcpClientThread(&keysS, crsa, type, data, destinatary);
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    //connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(thread, SIGNAL(started()), worker, SLOT(work()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

void TcpServer::clientThreadNeeded(int type, QByteArray data){
    TcpClientThread* worker = new TcpClientThread(&keysS, crsa, type, data);
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    //connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(thread, SIGNAL(started()), worker, SLOT(work()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

void TcpServer::setServerMode(int i){
    this->serverMode = i;
}


