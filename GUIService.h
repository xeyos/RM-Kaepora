#ifndef GUISERVICE_H
#define GUISERVICE_H
#include "Network/TcpServer.h"
#include "LocalService.h"

class GUIService
{
public:
    GUIService();
    void setServer(TcpServer *server);
    void setServerMode(int serverMode);
    void setPassword(QString pass);
    bool startServer();

private:
    TcpServer *server;
};

#endif // GUISERVICE_H
