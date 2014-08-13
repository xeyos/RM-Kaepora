#include "TcpRmLocalFriendServer.h"
#include "DB/BaseDBController.h"
#include "DB/DBFactory.h"

TcpRmLocalFriendServer::TcpRmLocalFriendServer()
{
}

void TcpRmLocalFriendServer::setSocket(TcpServerThread *server, BlowFish *bf){
    this->server = server;
    this->bf = bf;
    //Al crear e inicializar el objeto, devolvemos
    //el estado del servidor al cliente a modo de saludo.
    rm::ServerStatus sStatus;
    std::string aux, cypheredMsg;
    bf->Encrypt(&cypheredMsg, std::string()+(char)server->getServerMode());
    sStatus.set_registeringstatus(cypheredMsg);
    bf->Encrypt(&cypheredMsg, std::string()+(char)100);
    sStatus.set_avaliability(cypheredMsg);
    bf->Encrypt(&cypheredMsg, "Ready to serve");
    sStatus.set_generalstatus(cypheredMsg);
    QByteArray serialArray;
    serialArray.append("011");
    serialArray.append(sStatus.SerializeAsString().c_str(), sStatus.ByteSize());
    this->server->sendExternalMsg(serialArray);
}

void TcpRmLocalFriendServer::awake(std::string type, QByteArray byteArray){
    try{
        std::string decypheredData,encrypted;
        rm::Auth auth;
        auth.ParseFromArray(byteArray,byteArray.size());
        bf->Decrypt(&decypheredData, auth.user());
        std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
        auth.set_user(decypheredData);
        bf->Decrypt(&decypheredData, auth.password());
        auth.set_password(decypheredData);
        bf->Decrypt(&decypheredData, auth.port());
        auth.set_port(decypheredData);
        if(checkUname(auth.user()) && checkPassword(auth.password())){
            DBFactory dbfactory;
            BaseDBController *db = dbfactory.getDB();
            int result = db->login(auth, server->getClientIp());
            if(result ==0){
                DBFactory dbf;
                DBController *db = dbf.getDB();
                rm::UserDataBlock udb = db->getFriendList(auth.user());
                rm::UserDataBlock cudb;
                for(int i = 0; i<udb.localfriend_size(); i++){
                    const rm::UserData& ud = udb.localfriend(i);
                    rm::UserData *cud = cudb.add_localfriend();
                    bf->Encrypt(&encrypted, ud.user());
                    cud->set_user(encrypted);
                    bf->Encrypt(&encrypted, ud.ip());
                    cud->set_ip(encrypted);
                    bf->Encrypt(&encrypted, ud.port());
                    cud->set_port(encrypted);
                    bf->Encrypt(&encrypted, ud.date());
                    cud->set_date(encrypted);
                }
                QByteArray serialArray;
                serialArray.append("006");
                serialArray.append(cudb.SerializeAsString().c_str(),cudb.ByteSize());
                server->sendExternalMsg(serialArray);
           }else{
                qDebug()<<"Datos erroneos en solicitud de lista de amigos.";
                server->disconnected();
            }
        }else{
            qDebug()<<"Datos erroneos en solicitud de lista de amigos.";
            server->disconnected();
        }
    }catch(int e){
        qDebug("paquete incorrecto en solicitud de lista de amigos");
    }

}
