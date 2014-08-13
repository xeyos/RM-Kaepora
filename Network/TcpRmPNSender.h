#ifndef TCPRMPNSENDER_H
#define TCPRMPNSENDER_H
#include "TcpRmClient.h"
#include "RedMalk.pb.h"

class TcpRmPNSender: public TcpRmClient
{
public:
    TcpRmPNSender(QTcpSocket *s, TcpClientThread *tst, rm::PersonalNew invite, RsaKeys *keys, CustomRsa *rsa, QString destinatary);
    void sendConnType();
    void ownWork(std::string type, QByteArray byteArray);

private:
    void disconnect();


private:
    rm::PersonalNew siv;
    std::string destinatary;
};

#endif // TCPRMPNSENDER_H
