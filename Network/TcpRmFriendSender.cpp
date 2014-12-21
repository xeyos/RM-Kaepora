#include "TcpRmFriendSender.h"
#include "DB/BaseDBController.h"
#include "DB/DBFactory.h"

TcpRmFriendSender::TcpRmFriendSender()
{
}

void TcpRmFriendSender::setSocket(TcpServerThread *server, BlowFish *bf){
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

void TcpRmFriendSender::awake(std::string type, QByteArray byteArray){
    std::string decypheredData, encrypted;
    if(type.compare("009") == 0){
        rm::User client;
        client.ParseFromArray(byteArray,byteArray.size());
        bf->Decrypt(&decypheredData, client.user());
        client.set_user(decypheredData);
        bf->Decrypt(&decypheredData, client.server());
        client.set_server(decypheredData);
        DBFactory dbf;
        DBController *db = dbf.getDB();
        udb = db->getExternalFriendList(client, &userKey);
        rm::ServerResponse sr;
        QByteArray serialArray;
        serialArray.append("001");
        if(udb.localfriend_size()>0){
            bf->Encrypt(&encrypted, "0");
            sr.set_code(encrypted);
            bf->Encrypt(&encrypted, userKey.substr(0,3));
            sr.set_description(encrypted);
            serialArray.append(sr.SerializeAsString().c_str(), sr.ByteSize());
            this->server->sendExternalMsg(serialArray);
        }else{
            bf->Encrypt(&encrypted, "1");
            sr.set_code(encrypted);
            bf->Encrypt(&encrypted, "Youve got no friends here");
            sr.set_description(encrypted);
            serialArray.append(sr.SerializeAsString().c_str(), sr.ByteSize());
            this->server->sendExternalMsg(serialArray);
            server->closeConnection();
        }
    }else if(type.compare("012")==0){
        std::string decrypted, encrypted;
        rm::ExternalUserData eud;
        eud.ParseFromArray(byteArray,byteArray.size());
        bf->Decrypt(&decrypted, eud.key());
        rm::UserDataBlock cudb;
        if(decrypted.compare(userKey)==0){
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
                QByteArray serialArray;
                serialArray.append("006");
                serialArray.append(cudb.SerializeAsString().c_str(),cudb.ByteSize());
                server->sendExternalMsg(serialArray);
            }
        }
        server->closeConnection();
    }
}
