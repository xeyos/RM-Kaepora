#ifndef TCPRMINVITATIONVALIDATOR_H
#define TCPRMINVITATIONVALIDATOR_H
#include "TcpRmStatus.h"
#include "TcpServerThread.h"

class TcpRmInvitationValidator:public TcpRmStatus
{
public:
    TcpRmInvitationValidator();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);
};

#endif // TCPRMINVITATIONVALIDATOR_H
