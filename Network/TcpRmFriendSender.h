#ifndef TCPRMFRIENDSENDER_H
#define TCPRMFRIENDSENDER_H
#include "Network/Proto/RedMalk.pb.h"
#include "TcpRmStatus.h"
#include "TcpServerThread.h"

class TcpRmFriendSender:public TcpRmStatus
{
public:
    TcpRmFriendSender();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    rm::UserDataBlock udb;
    std::string userKey;
};

#endif // TCPRMFRIENDSENDER_H
