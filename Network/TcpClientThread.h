#ifndef TCPCLIENTTHREAD_H
#define TCPCLIENTTHREAD_H
#include <QObject>
#include "TcpRmClient.h"

class TcpClientThread: public QObject
{
    Q_OBJECT
public:
    explicit TcpClientThread(RsaKeys *k, CustomRsa *rsa, int type, QByteArray data, QString destinatary);
    explicit TcpClientThread(RsaKeys *k, CustomRsa *rsa, int type, QByteArray data);
    void sendArray(QByteArray byteArray);
    void sendMsg(std::string msg);
    QString getClientIp();
    void disconnectFromHost();

private:
    std::string getNerworkString();

signals:
    void error(QTcpSocket::SocketError socketerror);
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
    TcpRmClient *rmc;
    int type;
    QByteArray data;
    QString destinatary;
};

#endif // TCPCLIENTTHREAD_H
