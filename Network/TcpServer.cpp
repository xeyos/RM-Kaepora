#include <QThread>
#include "TcpServerThread.h"
#include "TcpServer.h"

TcpServer::TcpServer(QObject *parent) :
    QTcpServer(parent)
{
    crsa = new CustomRsa();
    keysS = crsa->getKeys();
}

void TcpServer::startServer()
{
    int port = 1432;

    if(!this->listen(QHostAddress::Any, port))
    {
        qDebug("Error lanzando el servidor");
    }
    else
    {
        qDebug("Servidor iniciado");
    }
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug("Alguien viene");
    TcpServerThread* worker = new TcpServerThread(socketDescriptor, &keysS, crsa);
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    //connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(thread, SIGNAL(started()), worker, SLOT(work()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), this, SLOT(threadFinished()));
    thread->start();
}

void TcpServer::threadFinished(){
    qDebug()<<"bb";
    //ni->write("Alguien ha desconectado\r");
}

