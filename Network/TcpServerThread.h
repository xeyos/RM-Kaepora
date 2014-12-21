
#ifndef TCPSERVERTHREAD_H
#define TCPSERVERTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QString>
#include "Security/SecurityData.h"
#include "Security/CustomRsa.h"
#include "Security/BlowFish.h"
#include "TcpRmStatus.h"
#include "TcpRmClient.h"

class TcpServerThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerThread(qintptr ID, RsaKeys *k, CustomRsa *rsa, int serverMode);
    void sendExternalMsg(QByteArray byteArray);
    int getServerMode();
    void changeState(int type, QByteArray data, QString destinatary);
    void changeState(int type, QByteArray data);
    QString getClientIp();
    void closeConnection();
    void blockThread(bool value);
    void sendStatus();

private:
    std::string getNetworkString();
    void sendMsg(std::string msg);

signals:
     void error(QTcpSocket::SocketError socketerror);
     void clientThreadNeeded(int type, QByteArray data, QString destinatary);
     void clientThreadNeeded(int type, QByteArray data);
     void finished();

public slots:
     void readyRead();
     void disconnected();
     void work();

private:
    QTcpSocket *socket;
    qintptr socketDescriptor;
    RsaKeys *keys, remoteKeys;
    BlowFishKey bfKey;
    CustomRsa *crsa;
    BlowFish *bf;
    int state, serverMode;
    TcpRmStatus *rm;
    bool blocked;
};

#endif // TCPSERVERTHREAD_H
