#include "TcpRmInvitationListener.h"
#include "DB/BaseDBController.h"
#include "DB/DBFactory.h"

TcpRmInvitationListener::TcpRmInvitationListener()
{
}

void TcpRmInvitationListener::setSocket(TcpServerThread *server, BlowFish *bf){
    this->server = server;
    this->bf = bf;
    //Al crear e inicializar el objeto, devolvemos
    //el estado del servidor al cliente a modo de saludo.
    rm::ServerStatus sStatus;
    std::string cypheredMsg;
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

void TcpRmInvitationListener::awake(std::string type, QByteArray byteArray){
    try{
        server->blockThread(true);
        rm::Invitation invite;
        std::string decypheredData;
        invite.ParseFromArray(byteArray, byteArray.size());
        bf->Decrypt(&decypheredData, invite.user());
        std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
        invite.set_user(decypheredData);
        bf->Decrypt(&decypheredData, invite.server());
        std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
        invite.set_server(decypheredData);
        bf->Decrypt(&decypheredData, invite.key());
        invite.set_key(decypheredData);
        bf->Decrypt(&decypheredData, invite.destinatary());
        std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
        invite.set_destinatary(decypheredData);
        bf->Decrypt(&decypheredData, invite.msg());
        invite.set_msg(decypheredData);
        qDebug()<<"El usuario "+QString::fromStdString(invite.user())+" del servidor "+QString::fromStdString(invite.server())+" quiere invitar a "+QString::fromStdString(invite.destinatary());
        DBFactory dbf;
        DBController *db = dbf.getDB();
        if(!db->testUser(invite.destinatary())){
            qDebug()<<"Que NO es un usuario de este server";
            server->blockThread(false);
            this->server->closeConnection();
        }else{
            qDebug()<<"Que es un usuario de este server";
            QByteArray serialArray;
            serialArray.append(invite.SerializeAsString().c_str(),invite.ByteSize());
            server->changeState(1,serialArray);
        }
    }catch(int i){
        server->blockThread(false);
        this->server->closeConnection();
    }
}
