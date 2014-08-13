#include "LocalService.h"
#include "Security/BlowFish.h"
#include <fstream>
#include <QDebug>

QString LocalService::pass;
QString LocalService::server = "";

LocalService::LocalService()
{
}

void LocalService::setPassword(QString p){
    pass = p;
}

QString LocalService::getServerName(){
    if (server.compare("")==0){
        std::ifstream iSettings("Settings");
        std::string datoCifrado;
        std::string acum="";
        int count = 0, prevCount = 0;
        QString serverName;
        BlowFish bf;
        bf.SetKey(pass.toUtf8().constData());
        if(iSettings){
            while(getline(iSettings, datoCifrado)){
                if(datoCifrado.substr(datoCifrado.length()-6,6).compare("/@data")==0){
                    count++;
                    datoCifrado = datoCifrado.substr(0,datoCifrado.length()-6);
                }
                if(acum.compare("")!=0){
                    acum +="\n" + datoCifrado;
                }else{
                    acum =datoCifrado;
                }
                if(count!=prevCount){
                    datoCifrado= acum;
                    bf.Decrypt(&acum, datoCifrado);
                    prevCount =count;
                    switch (count) {
                    case 6:{
                        serverName= QString::fromStdString(acum);
                        break;
                    }
                    }
                    acum = "";
                }
                server = serverName;
            }
        }
    }
    return server;
}
