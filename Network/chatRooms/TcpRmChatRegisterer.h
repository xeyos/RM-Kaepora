#ifndef TCPRMCHATREGISTERER_H
#define TCPRMCHATREGISTERER_H
#include "Network/TcpServerThread.h"
#include "Network/TcpRmStatus.h"
#include "Security/BlowFish.h"
#include "RedMalk.pb.h"

class TcpRmChatRegisterer: public TcpRmStatus
{
public:
    TcpRmChatRegisterer();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    void sendError(std::string error, std::string code);

private:
    TcpServerThread *server;
    BlowFish *bf;
    int state;
    std::string localUser;
};

#endif // TCPRMCHATREGISTERER_H
