#ifndef TCPRMINVITATIONRESPONSER_H
#define TCPRMINVITATIONRESPONSER_H
#include "TcpRmStatus.h"
#include "TcpServerThread.h"

class TcpRmInvitationResponser:public TcpRmStatus
{
public:
    TcpRmInvitationResponser();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    TcpServerThread *server;
    BlowFish *bf;
};

#endif // TCPRMINVITATIONRESPONSER_H
