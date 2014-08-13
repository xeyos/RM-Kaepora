#include "TcpRmPersonalNoticesAdmin.h"
#include "DB/DBFactory.h"
#include "LocalService.h"

TcpRmPersonalNoticesAdmin::TcpRmPersonalNoticesAdmin()
{
    state = 0;
}

void TcpRmPersonalNoticesAdmin::setSocket(TcpServerThread *server, BlowFish *bf){
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
    server->sendExternalMsg(serialArray);
}

void TcpRmPersonalNoticesAdmin::awake(std::string type, QByteArray byteArray){
    try{
        switch(state){
        //Caso 0 implica inicio de autentificación
            case 0:{
                std::string decypheredData,cypheredData;
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
                        ownUser = auth.user();
                        bf->Encrypt(&cypheredData,"0");
                        rm::ServerResponse sr;
                        sr.set_code(cypheredData);
                        QByteArray serialArray;
                        serialArray.append("001");
                        serialArray.append(sr.SerializeAsString().c_str(), sr.ByteSize());
                        server->sendExternalMsg(serialArray);
                        state++;
                    }else{
                        qDebug()<<"Datos erroneos en verificación de cliente confirmando invite.";
                        server->disconnected();
                    }
                }else{
                    qDebug()<<"Datos erroneos en verificación de cliente confirmando invite.";
                    server->disconnected();
                }
                break;
            }
            case 1:{
            //EL usuario está identificado, procede a facilitarnos la información del invite
                std::string decypheredData;
                //Desciframos el mensaje
                rm::PersonalNew civ;
                civ.ParseFromArray(byteArray,byteArray.size());
                bf->Decrypt(&decypheredData,civ.type());
                civ.set_type(decypheredData);
                bf->Decrypt(&decypheredData, civ.user());
                civ.set_user(decypheredData);
                bf->Decrypt(&decypheredData, civ.userserver());
                civ.set_userserver(decypheredData);
                qDebug()<<"Llega respuesta a noticia tipo "+QString::fromStdString(civ.type())+" respondida por "+QString::fromStdString(ownUser)+" referente a "+QString::fromStdString(civ.user())+"@"+QString::fromStdString(civ.userserver());
                if(civ.type().compare("invitation confirmation")==0){
                    bf->Decrypt(&decypheredData, civ.internaldata());
                    civ.set_internaldata(decypheredData);
                    bf->Decrypt(&decypheredData, civ.response());
                    civ.set_response(decypheredData);
                    DBFactory dbfactory;
                    BaseDBController *db = dbfactory.getDB();
                    if(db->confirmSentInvitation(ownUser,civ)){
                        qDebug()<<"Confirmamos la invitación";
                        rm::serverInvitationVerif siv;
                        siv.set_userinvited(civ.user());
                        siv.set_userinviting(ownUser);
                        siv.set_key(civ.internaldata());
                        siv.set_server(civ.userserver());
                        siv.set_response("true");
                        QByteArray serialArray;
                        serialArray.append(siv.SerializeAsString().c_str(), siv.ByteSize());
                        server->changeState(3,serialArray);
                    }else if (civ.response().compare("false")==0){
                        qDebug()<<"rechazamos la invitación";
                        rm::serverInvitationVerif siv;
                        siv.set_userinvited(civ.user());
                        siv.set_userinviting(ownUser);
                        siv.set_key("none");
                        siv.set_server(civ.userserver());
                        siv.set_response("false");
                        QByteArray serialArray;
                        serialArray.append(siv.SerializeAsString().c_str(), siv.ByteSize());
                        server->changeState(3,serialArray);
                    }
                    break;
                }else if(civ.type().compare("invitation")==0){
                    DBFactory dbfactory;
                    BaseDBController *db = dbfactory.getDB();
                    bf->Decrypt(&decypheredData, civ.internaldata());
                    civ.set_internaldata(decypheredData);
                    bf->Decrypt(&decypheredData, civ.internaldataaux());
                    civ.set_internaldataaux(decypheredData);
                    bf->Decrypt(&decypheredData, civ.response());
                    civ.set_response(decypheredData);
                    rm::serverInvitationVerif siv;
                    siv.set_userinvited(ownUser);
                    siv.set_userinviting(civ.user());
                    LocalService ls;
                    siv.set_server(ls.getServerName().toUtf8().constData());
                    if(civ.response().compare("true")==0){
                        qDebug()<<"Aceptamos la invitación;"+QString::fromStdString(civ.internaldata())+";"+QString::fromStdString(civ.internaldataaux());
                        siv.set_response("true");
                        siv.set_key(civ.internaldataaux());
                        db->createFriendShip(civ, ownUser);    
                    }else{
                        qDebug()<<"Rechazamos la invitación";
                        siv.set_key("none");
                        siv.set_response("false");
                        db->denyInvite(siv);
                    }
                    QByteArray serialArray;
                    serialArray.append(siv.SerializeAsString().c_str(), siv.ByteSize());
                    server->changeState(5,serialArray);
                }
            }
        }
    }catch(int i){
        this->server->disconnected();
    }
}
