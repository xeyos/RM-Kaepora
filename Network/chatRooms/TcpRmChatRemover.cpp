#include "TcpRmChatRemover.h"
#include "DB/DBFactory.h"
#include "DB/DBController.h"

TcpRmChatRemover::TcpRmChatRemover()
{
    state = 0;
}

void TcpRmChatRemover::setSocket(TcpServerThread *server, BlowFish *bf){
    this->server = server;
    this->bf = bf;
    server->sendStatus();
}

void TcpRmChatRemover::awake(std::string type, QByteArray byteArray){
    try{
        switch(state){
        //Caso 0 implica inicio de autentificación
            case 0:{
                std::string decypheredData;
                rm::roomData rd;
                rd.ParseFromArray(byteArray,byteArray.size());
                bf->Decrypt(&decypheredData, rd.user());
                std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
                rd.set_user(decypheredData);
                bf->Decrypt(&decypheredData, rd.pass());
                rd.set_pass(decypheredData);
                bf->Decrypt(&decypheredData, rd.room());
                std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
                rd.set_room(decypheredData.substr(0,decypheredData.find("#")));
                if(checkUname(rd.user()) && checkPassword(rd.pass()) && checkUname(rd.room())){
                    DBFactory dbfactory;
                    BaseDBController *db = dbfactory.getDB();
                    db->removeUserRoom(rd);
                }
                server->closeConnection();
                break;
        }
        }
    }catch(int e){
        qDebug()<<"Error en el registro de salas";
    }
}
