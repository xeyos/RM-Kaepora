#include "TcpRmChatClamer.h"
#include "DB/DBFactory.h"
#include "DB/DBController.h"

TcpRmChatClamer::TcpRmChatClamer()
{
    state=0;
}

void TcpRmChatClamer::setSocket(TcpServerThread *server, BlowFish *bf){
    this->server = server;
    this->bf = bf;
    server->sendStatus();
}

void TcpRmChatClamer::awake(std::string type, QByteArray byteArray){
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
                //If everything was ok
                DBFactory dbf;
                DBController *db = dbf.getDB();
                if(db->claimRoom(rd) ==0){
                    rm::ServerResponse sr;
                    bf->Encrypt(&cypheredData, "0");
                    sr.set_code(cypheredData);
                    QByteArray serialArray;
                    serialArray.append("001");
                    serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
                    server->sendExternalMsg(serialArray);
                    server->closeConnection();
                }else{
                    rm::ServerResponse sr;
                    bf->Encrypt(&cypheredData, "1");
                    sr.set_code(cypheredData);
                    bf->Encrypt(&cypheredData, "Wrong room or password.");
                    sr.set_description(cypheredData);
                    QByteArray serialArray;
                    serialArray.append("001");
                    serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
                    server->sendExternalMsg(serialArray);
                    server->closeConnection();
                }
            }else{
                qDebug()<<"Datos erroneos en verificación de reclamo de sala.";
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
