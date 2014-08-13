#ifndef TCPRMINVITATIONLISTENER_H
#define TCPRMINVITATIONLISTENER_H
#include "TcpRmStatus.h"
#include "TcpServerThread.h"
#include "TcpRmInvitationAdmin.h"
#include "Security/BlowFish.h"
#include "RedMalk.pb.h"

class TcpRmInvitationListener:public TcpRmStatus
{
public:
    TcpRmInvitationListener();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string msg);
    void awake(std::string type, QByteArray byteArray);

private:
    TcpServerThread *server;
    BlowFish *bf;
};

#endif // TCPRMINVITATIONLISTENER_H
