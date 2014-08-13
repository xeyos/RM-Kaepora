#ifndef TCPRMCLIENT_H
#define TCPRMCLIENT_H
#include <string>
#include "Security/SecurityData.h"
#include "Security/CustomRsa.h"
#include "Security/BlowFish.h"
#include <qtcpsocket.h>

class TcpClientThread;
class TcpRmClient
{
public:
    TcpRmClient();
    void handleHandshake(std::string message);
    void awake(std::string message);
    void awake(std::string msgType, QByteArray byteArray);
    virtual void ownWork(std::string msgType, QByteArray byteArray) = 0;
    virtual void sendConnType() = 0;
    bool isHandshake();

protected:
    bool checkUname(std::string uname);
    bool checkPassword(std::string pass);

protected:
    bool handshake;
    int state;
    RsaKeys remoteKeys, *keys;
    BlowFishKey bfk;
    BlowFish *bf;
    CustomRsa *crsa;
    QTcpSocket *socket;
    TcpClientThread *thread;
    std::string connType;
};

#endif // TCPRMCLIENT_H
