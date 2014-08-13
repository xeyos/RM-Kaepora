#ifndef TCPRMCONNECTOR_H
#define TCPRMCONNECTOR_H
#include "TcpRmStatus.h"
#include "TcpServerThread.h"
#include "RedMalk.pb.h"
#include "Security/BlowFish.h"

class TcpRmConnector : public TcpRmStatus
{
public:
    TcpRmConnector();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    TcpServerThread *server;
    BlowFish *bf;
    rm::Auth auth;
    int state;
};

#endif // TCPRMCONNECTOR_H
