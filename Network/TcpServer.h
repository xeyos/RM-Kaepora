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
    bool startServer();
    void setServerMode(int i);
    int getServerMode();

public slots:
    void clientThreadNeeded(int type, QByteArray data, QString destinatary);
    void clientThreadNeeded(int type, QByteArray data);

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    RsaKeys keysS;
    CustomRsa* crsa;
    int serverMode;
};

#endif // TCPSERVER_H
