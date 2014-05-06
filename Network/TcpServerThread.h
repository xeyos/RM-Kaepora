
#ifndef TCPSERVERTHREAD_H
#define TCPSERVERTHREAD_H

#include <QObject>
#include <QSslSocket>
#include <QDebug>
#include "Security/SecurityData.h"
#include "Security/CustomRsa.h"
#include "Security/BlowFish.h"

class TcpServerThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerThread(qintptr ID, RsaKeys *k, CustomRsa *rsa);

private:
    std::string obtenerMensaje();

signals:
     void error(QTcpSocket::SocketError socketerror);
     void finished();

public slots:
     void readyRead();
     void disconnected();
     void work();
     void sendMsg(std::string msg);

private:
    QTcpSocket *socket;
    qintptr socketDescriptor;
    RsaKeys *keys, remoteKeys;
    BlowFishKey bfKey;
    CustomRsa *crsa;
    BlowFish *bf;
    QString bfText;
    int state;
};

#endif // TCPSERVERTHREAD_H
