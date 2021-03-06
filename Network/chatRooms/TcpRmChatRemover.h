#ifndef TCPRMCHATREMOVER_H
#define TCPRMCHATREMOVER_H
#include "Network/TcpServerThread.h"
#include "Network/TcpRmStatus.h"
#include "Security/BlowFish.h"
#include "RedMalk.pb.h"

class TcpRmChatRemover: public TcpRmStatus
{
public:
    TcpRmChatRemover();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    TcpServerThread *server;
    BlowFish *bf;
    int state;
};

#endif // TCPRMCHATREMOVER_H
