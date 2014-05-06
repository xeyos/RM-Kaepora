#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QString>
#include <QDebug>
#include "Security/SecurityData.h"
#include "Security/CustomRsa.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    void startServer();

public slots:
    void threadFinished();

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    RsaKeys keysS;
    CustomRsa* crsa;
};

#endif // TCPSERVER_H
