#include "TcpRmInvitationEnder.h"
#include "Network/Proto/RedMalk.pb.h"
#include "DB/BaseDBController.h"
#include "DB/DBFactory.h"

TcpRmInvitationEnder::TcpRmInvitationEnder()
{
}

void TcpRmInvitationEnder::setSocket(TcpServerThread *server, BlowFish *bf){
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

void TcpRmInvitationEnder::awake(std::string type, QByteArray byteArray){
    std::string decypheredData;
    rm::serverInvitationVerif siv;
    siv.ParseFromArray(byteArray,byteArray.size());
    bf->Decrypt(&decypheredData, siv.userinvited());
    siv.set_userinvited(decypheredData);
    bf->Decrypt(&decypheredData, siv.userinviting());
    siv.set_userinviting(decypheredData);
    bf->Decrypt(&decypheredData, siv.server());
    siv.set_server(decypheredData);
    bf->Decrypt(&decypheredData, siv.key());
    siv.set_key(decypheredData);
    bf->Decrypt(&decypheredData, siv.response());
    siv.set_response(decypheredData);
    qDebug()<<"el usuario "+QString::fromStdString(siv.userinviting())+" "+QString::fromStdString(siv.response())+" ha invitado a "+QString::fromStdString(siv.userinvited());
    DBFactory dbf;
    DBController *db = dbf.getDB();
    if(siv.response().compare("true")==0){
        QByteArray serialArray;
        if (db->confirmReceivedInvitation(siv)){
            rm::PersonalNew pn;
            pn.set_user(siv.userinviting());
            pn.set_userserver(siv.server());
            pn.set_internaldata(siv.key());
            pn.set_type("invitation");
            db->getInviteInfo(&pn,siv.userinvited());
            serialArray.append(pn.SerializeAsString().c_str(),pn.ByteSize());
            server->changeState(6, serialArray, QString::fromStdString(siv.userinvited()));
        }else{
            server->closeConnection();
        }
    }else{
        db->denyInvite(siv);
    }
}
