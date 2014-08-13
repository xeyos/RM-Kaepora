#ifndef TCPRMPERSONALNOTICESADMIN_H
#define TCPRMPERSONALNOTICESADMIN_H
#include "TcpRmStatus.h"
#include "Security/BlowFish.h"
#include "RedMalk.pb.h"
#include "TcpRmStatus.h"
#include "TcpServerThread.h"

class TcpRmPersonalNoticesAdmin:public TcpRmStatus
{
public:
    TcpRmPersonalNoticesAdmin();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    TcpServerThread *server;
    BlowFish *bf;
    int state;
    std::string ownUser;
};

#endif // TCPRMPERSONALNOTICESADMIN_H
