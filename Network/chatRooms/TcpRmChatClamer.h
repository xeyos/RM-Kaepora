#ifndef TCPRMCHATCLAMER_H
#define TCPRMCHATCLAMER_H
#include "Network/TcpServerThread.h"
#include "Network/TcpRmStatus.h"
#include "Security/BlowFish.h"
#include "RedMalk.pb.h"

class TcpRmChatClamer: public TcpRmStatus
{
public:
    TcpRmChatClamer();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    TcpServerThread *server;
    BlowFish *bf;
    int state;
    std::string userAsking;
    std::string localUser;
};

#endif // TCPRMCHATCLAMER_H
