#include "TcpRmChatRegisterer.h"
#include "DB/DBFactory.h"
#include "DB/DBController.h"
#include "LocalService.h"

TcpRmChatRegisterer::TcpRmChatRegisterer()
{
    state=0;
}

void TcpRmChatRegisterer::setSocket(TcpServerThread *server, BlowFish *bf){
    this->server = server;
    this->bf = bf;
    server->sendStatus();
}

void TcpRmChatRegisterer::awake(std::string type, QByteArray byteArray){
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
                        this->localUser = auth.user();
                        bf->Encrypt(&cypheredData,"0");
                        rm::ServerResponse sr;
                        sr.set_code(cypheredData);
                        QByteArray serialArray;
                        serialArray.append("001");
                        serialArray.append(sr.SerializeAsString().c_str(), sr.ByteSize());
                        server->sendExternalMsg(serialArray);
                        state++;
                    }else{
                        qDebug()<<"Datos erroneos en verificación de cliente creando sala.";
                        server->closeConnection();
                    }
                }else{
                    qDebug()<<"Datos erroneos en verificación de cliente creando sala.";
                    server->closeConnection();
                }
                break;
        }case 1:{
            std::string decypheredData, cypheredData;
            rm::roomData rd;
            rd.ParseFromArray(byteArray, byteArray.size());
            bf->Decrypt(&decypheredData, rd.room());
            std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
            rd.set_room(decypheredData);
            bf->Decrypt(&decypheredData, rd.pass());
            rd.set_pass(decypheredData);
            rd.set_user(localUser);
            if((checkUname(rd.room())) &&(checkShortPassword(rd.pass()))){
                bf->Decrypt(&decypheredData, rd.permissions());
                std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
                rd.set_permissions(decypheredData);
                if(rd.permissions().compare("passprotected")==0){
                    bf->Decrypt(&decypheredData, rd.accesspass());
                    rd.set_accesspass(decypheredData);
                    if(!checkShortPassword(rd.accesspass())){
                        sendError("Icorrect access password.", "1");
                    }else{
                        //If everything was ok
                        DBFactory dbf;
                        DBController *db = dbf.getDB();
                        if(db->roomRegister(rd) ==0){
                            rm::ServerResponse sr;
                            bf->Encrypt(&cypheredData, "0");
                            sr.set_code(cypheredData);
                            LocalService ls;
                            std::string auxRoomName = rd.room();
                            bf->Encrypt(&cypheredData, auxRoomName+"#"+ls.getServerName().toUtf8().constData());
                            sr.set_description(cypheredData);
                            QByteArray serialArray;
                            serialArray.append("001");
                            serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
                            server->sendExternalMsg(serialArray);
                            server->closeConnection();
                        }else{
                            sendError("You already have a room at your name or the selected roomname is already in use", "1");
                        }
                    }
                }else if(rd.permissions().compare("public")==0){
                    //If everything was ok
                    DBFactory dbf;
                    DBController *db = dbf.getDB();
                    if(db->roomRegister(rd) ==0){
                        rm::ServerResponse sr;
                        bf->Encrypt(&cypheredData, "0");
                        sr.set_code(cypheredData);
                        LocalService ls;
                        std::string auxRoomName = rd.room();
                        bf->Encrypt(&cypheredData, auxRoomName+"#"+ls.getServerName().toUtf8().constData());
                        sr.set_description(cypheredData);
                        QByteArray serialArray;
                        serialArray.append("001");
                        serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
                        server->sendExternalMsg(serialArray);
                        server->closeConnection();
                    }else{
                        sendError("You already have a room at your name or the selected roomname is already in use", "1");
                    }
                }
            }else{
               sendError("Icorrect room name or password.", "1");
            }
            break;
        }
        }
    }catch(int e){
        qDebug()<<"Error en el registro de salas";
    }
}

void TcpRmChatRegisterer::sendError(std::string error, std::string code){
    std::string cypheredData;
    rm::ServerResponse sr;
    bf->Encrypt(&cypheredData, code);
    sr.set_code(cypheredData);
    bf->Encrypt(&cypheredData, error);
    sr.set_description(cypheredData);
    QByteArray serialArray;
    serialArray.append("001");
    serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
    server->sendExternalMsg(serialArray);
}
