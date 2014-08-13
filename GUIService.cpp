#include "GUIService.h"

GUIService::GUIService()
{
}

void GUIService::setServer(TcpServer *server){
    this->server = server;
}

void GUIService::setServerMode(int serverMode){
    server->setServerMode(serverMode);
}

bool GUIService::startServer(){
    return server->startServer();
}

void GUIService::setPassword(QString pass){
    LocalService ls;
    ls.setPassword(pass);
}
