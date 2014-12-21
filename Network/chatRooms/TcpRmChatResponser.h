#ifndef TCPRMCHATRESPONSER_H
#define TCPRMCHATRESPONSER_H
#include "Network/TcpServerThread.h"
#include "Network/TcpRmStatus.h"
#include "Security/BlowFish.h"
#include "RedMalk.pb.h"

class TcpRmChatResponser: public TcpRmStatus
{
public:
    TcpRmChatResponser();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    void sendError(std::string error, std::string code);

private:
    TcpServerThread *server;
    BlowFish *bf;
    int state;
    std::string userAsking;
};

#endif // TCPRMCHATRESPONSER_H
