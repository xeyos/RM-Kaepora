#ifndef TCPRMLOCALFRIENDSERVER_H
#define TCPRMLOCALFRIENDSERVER_H
#include "Network/Proto/RedMalk.pb.h"
#include "TcpRmStatus.h"
#include "TcpServerThread.h"

class TcpRmLocalFriendServer:public TcpRmStatus
{
public:
    TcpRmLocalFriendServer();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    rm::UserDataBlock udb;
    std::string userKey;
};

#endif // TCPRMLOCALFRIENDSERVER_H
