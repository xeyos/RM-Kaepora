#ifndef TCPRMINVITATONRESENDER_H
#define TCPRMINVITATONRESENDER_H
#include "TcpRmClient.h"
#include "RedMalk.pb.h"

class TcpRmInvitatonResender: public TcpRmClient
{
public:
    TcpRmInvitatonResender(QTcpSocket *s, TcpClientThread *tst, rm::serverInvitationVerif siv, RsaKeys *keys, CustomRsa *rsa, std::string connType);
    void sendConnType();
    void ownWork(std::string msgType, QByteArray byteArray);

private:
    rm::serverInvitationVerif siv;

};

#endif // TCPRMINVITATONRESENDER_H
