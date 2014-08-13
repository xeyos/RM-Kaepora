#ifndef TCPRMREGISTERER_H
#define TCPRMREGISTERER_H
#include "TcpServerThread.h"
#include "TcpRmStatus.h"
#include "Security/BlowFish.h"
#include "RedMalk.pb.h"

class TcpRmRegisterer: public TcpRmStatus
{
public:
    TcpRmRegisterer();
    void setSocket(TcpServerThread *server, BlowFish *bf);
    void awake(std::string type, QByteArray byteArray);

private:
    bool checkUname(std::string aux);
    bool checkEMail(std::string aux);
    bool checkPassword(std::string aux);
    void invalidUserData(std::string msg);

private:
    TcpServerThread *server;
    BlowFish *bf;
    std::string code;
    rm::RegisteringData rd;
};

#endif // TCPRMREGISTERER_H
