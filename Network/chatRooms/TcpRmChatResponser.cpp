#include "TcpRmChatResponser.h"
#include "DB/DBFactory.h"
#include "DB/DBController.h"
#include "LocalService.h"
#include <QUrl>
#include <sstream>

TcpRmChatResponser::TcpRmChatResponser()
{
    state=0;
}

void TcpRmChatResponser::setSocket(TcpServerThread *server, BlowFish *bf){
    this->server = server;
    this->bf = bf;
    server->sendStatus();
}

void TcpRmChatResponser::awake(std::string type, QByteArray byteArray){
    try{
        switch(state){
        //Caso 0 implica inicio de autentificación
            case 0:{
                std::string decypheredData, cypheredData;
                rm::roomData rd;
                rd.ParseFromArray(byteArray, byteArray.size());
                bf->Decrypt(&decypheredData, rd.room());
                std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
                rd.set_room(decypheredData);
                bf->Decrypt(&decypheredData, rd.user());
                std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
                rd.set_user(decypheredData);
                bf->Decrypt(&decypheredData, rd.server());
                std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
                rd.set_server(decypheredData);
                QUrl serverURL;
                serverURL.setUrl(QString::fromStdString(rd.server()), QUrl::StrictMode);
                userAsking = rd.user();
                if((checkUname(userAsking)) && serverURL.isValid()){
                    if(checkUname(rd.room())){
                        DBFactory dbf;
                        DBController *db = dbf.getDB();
                        rm::roomData room = db->getRoomData(rd.room());
                        if(room.room().compare("")!=0){
                            if(room.permissions().compare("public")==0){
                                rm::UserData ud;
                                InternalUserData iud = db->getUserData(room.user());
                                bf->Encrypt(&cypheredData, room.user());
                                ud.set_user(cypheredData);
                                bf->Encrypt(&cypheredData, iud.ip);
                                ud.set_ip(cypheredData);
                                std::ostringstream convert;
                                convert << iud.port;
                                bf->Encrypt(&cypheredData, convert.str());
                                ud.set_port(cypheredData);
                                QByteArray serialArray;
                                serialArray.append("016");
                                serialArray.append(ud.SerializeAsString().c_str(),ud.ByteSize());
                                server->sendExternalMsg(serialArray);
                            }else if(room.permissions().compare("passprotected")==0){
                                bf->Decrypt(&decypheredData, rd.accesspass());
                                rd.set_accesspass(decypheredData);
                                if(rd.accesspass().compare(room.accesspass())==0){
                                    rm::UserData ud;
                                    InternalUserData iud = db->getUserData(room.user());
                                    bf->Encrypt(&cypheredData, room.user());
                                    ud.set_user(cypheredData);
                                    bf->Encrypt(&cypheredData, iud.ip);
                                    ud.set_ip(cypheredData);
                                    std::ostringstream convert;
                                    convert << iud.port;
                                    bf->Encrypt(&cypheredData, convert.str());
                                    ud.set_port(cypheredData);
                                    QByteArray serialArray;
                                    serialArray.append("016");
                                    serialArray.append(ud.SerializeAsString().c_str(),ud.ByteSize());
                                    server->sendExternalMsg(serialArray);
                                }else{
                                    sendError("Incorrect access password.","1");
                                }
                            }else if(room.permissions().compare("friends")==0){
                                //TODO salas para amigos solo
                                server->closeConnection();
                            }
                        }else{
                            sendError("Room not found", "1");
                        }
                    }else{
                        sendError("Invalid room name", "1");
                    }
                }else{
                    server->closeConnection();
                }
                break;
        }
        }
    }catch(int e){
        qDebug()<<"Error while looking for a room";
        server->closeConnection();
    }
}

void TcpRmChatResponser::sendError(std::string error, std::string code){
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
