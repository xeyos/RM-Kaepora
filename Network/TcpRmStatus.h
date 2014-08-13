#ifndef TCPRMSTATUS_H
#define TCPRMSTATUS_H
#include "Security/BlowFish.h"
#include <string>
#include <QByteArray>

class TcpServerThread;
class TcpRmStatus{
public:
    virtual void setSocket(TcpServerThread *server, BlowFish *bf)=0;
    virtual void awake(std::string type, QByteArray byteArray)=0;

protected:
    bool checkUname(std::string aux);
    bool checkPassword(std::string aux);

protected:
    TcpServerThread *server;
    BlowFish *bf;
};

#endif // TCPRMSTATUS_H
