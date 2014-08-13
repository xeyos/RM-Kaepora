#ifndef TCPRMINVITATIONADMIN_H
#define TCPRMINVITATIONADMIN_H
#include "RedMalk.pb.h"
#include "TcpRmClient.h"

class TcpRmInvitationAdmin: public TcpRmClient
{
public:
    TcpRmInvitationAdmin(QTcpSocket *s, TcpClientThread *tst, rm::Invitation invite, RsaKeys *keys, CustomRsa *rsa);
    void sendConnType();

private:
    void ownWork(std::string type, QByteArray byteArray);

private:
    rm::Invitation invite;


};

#endif // TCPRMINVITATIONADMIN_H
