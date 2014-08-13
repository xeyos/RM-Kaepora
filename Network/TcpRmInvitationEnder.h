#ifndef TCPRMINVITATIONENDER_H
#define TCPRMINVITATIONENDER_H
#include "TcpRmStatus.h"
#include "TcpServerThread.h"

class TcpRmInvitationEnder:public TcpRmStatus
{
public:
    TcpRmInvitationEnder();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    TcpServerThread *server;
    BlowFish *bf;

};

#endif // TCPRMINVITATIONENDER_H
