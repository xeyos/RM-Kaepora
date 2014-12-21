#include "TcpRmConnector.h"
#include "DB/BaseDBController.h"
#include "DB/DBFactory.h"
#include "LocalService.h"

TcpRmConnector::TcpRmConnector(){
    state = 0;
}

void TcpRmConnector::setSocket(TcpServerThread *server, BlowFish *bf){
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
    QString ipAdress;
    ipAdress = server->getClientIp();
    bf->Encrypt(&cypheredMsg,ipAdress.toUtf8().constData());
    sStatus.set_ip(cypheredMsg);
    QByteArray serialArray;
    serialArray.append("011");
    serialArray.append(sStatus.SerializeAsString().c_str(), sStatus.ByteSize());
    this->server->sendExternalMsg(serialArray);
}

void TcpRmConnector::awake(std::string type, QByteArray byteArray){
    try{
        switch(state){
        //Caso 0 implica inicio de autentificación
        case 0:{
            std::string decypheredData;
            std::string cypheredData;
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
                    rm::PersonalNews pns= db->getPersonalNews(auth);
                    rm::PersonalNews cyphPns;
                    if(pns.personalnew_size()>0){
                        for(int i = 0; i<pns.personalnew_size(); i++){
                            const rm::PersonalNew& pn = pns.personalnew(i);
                            rm::PersonalNew *cpn = cyphPns.add_personalnew();
                            bf->Encrypt(&cypheredData, pn.content());
                            cpn->set_content(cypheredData);
                            bf->Encrypt(&cypheredData, pn.type());
                            cpn->set_type(cypheredData);
                            bf->Encrypt(&cypheredData, pn.user());
                            cpn->set_user(cypheredData);
                            bf->Encrypt(&cypheredData, pn.userserver());
                            cpn->set_userserver(cypheredData);
                            bf->Encrypt(&cypheredData, pn.internaldata());
                            cpn->set_internaldata(cypheredData);
                            bf->Encrypt(&cypheredData, pn.internaldataaux());
                            cpn->set_internaldataaux(cypheredData);
                        }
                    }
                    QByteArray serialArray;
                    serialArray.append("002");
                    serialArray.append(cyphPns.SerializeAsString().c_str(),cyphPns.ByteSize());
                    server->sendExternalMsg(serialArray);
                    state++;
                }else if(result == 1){
                    rm::ServerResponse sr;
                    bf->Encrypt(&cypheredData, "1");
                    sr.set_code(cypheredData);
                    bf->Encrypt(&cypheredData, "Icorrect user, password or server.");
                    sr.set_description(cypheredData);
                    QByteArray serialArray;
                    serialArray.append("001");
                    serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
                    server->sendExternalMsg(serialArray);
                }else{
                    rm::ServerResponse sr;
                    bf->Encrypt(&cypheredData, "2");
                    sr.set_code(cypheredData);
                    bf->Encrypt(&cypheredData, "Unexpected server db exception.");
                    sr.set_description(cypheredData);
                    QByteArray serialArray;
                    serialArray.append("001");
                    serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
                    server->sendExternalMsg(serialArray);
                    server->closeConnection();
                }
            }else{
                rm::ServerResponse sr;
                bf->Encrypt(&cypheredData, "2");
                sr.set_code(cypheredData);
                bf->Encrypt(&cypheredData, "Invalid data");
                sr.set_description(cypheredData);
                QByteArray serialArray;
                serialArray.append("001");
                serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
                server->sendExternalMsg(serialArray);
                server->disconnect();
            }
            break;
        }
        case 1:{
        //EL usuario está identificado, procede a facilitarnos la información de su sistema
            std::string decypheredData;
            std::string cypheredData;
            //Desciframos el mensaje
            rm::VersionPackage vp;
            vp.ParseFromArray(byteArray, byteArray.size());
            bf->Decrypt(&decypheredData, vp.so());
            std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
            vp.set_so(decypheredData);
            bf->Decrypt(&decypheredData, vp.system());
            std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
            vp.set_system(decypheredData);
            bf->Decrypt(&decypheredData, vp.concreteversion());
            std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
            vp.set_concreteversion(decypheredData);
            //Accedemos a la bd en busca de noticias
            DBFactory dbfactory;
            BaseDBController *db = dbfactory.getDB();
            rm::News ns = db->getNews(vp);
            //ciframos y enviamos la lista de noticias
            rm::News cyphNs;
            if(ns.notice_size()>0){
                for(int i = 0; i<ns.notice_size(); i++){
                    const rm::Notice& n = ns.notice(i);
                    rm::Notice *cn = cyphNs.add_notice();
                    bf->Encrypt(&cypheredData, n.type());
                    cn->set_type(cypheredData);
                    bf->Encrypt(&cypheredData, n.content());
                    cn->set_content(cypheredData);
                    bf->Encrypt(&cypheredData, n.relevance());
                    cn->set_relevance(cypheredData);
                }
            }
            QByteArray serialArray;
            serialArray.append("004");
            serialArray.append(cyphNs.SerializeAsString().c_str(),cyphNs.ByteSize());
            server->sendExternalMsg(serialArray);
            state++;
            break;
        }
        case 2:{
            //Penúltimo paso de la conexión, enviamos toda la lista de amigos
            //actualizada
            std::string cypheredData;
            //Accedemos a la bd en busca de l alista de amigos
            DBFactory dbfactory;
            BaseDBController *db = dbfactory.getDB();
            rm::UserDataBlock udb = db->getFriendList(auth.user());
            //ciframos y enviamos el userDataBlock
            rm::UserDataBlock cudb;
            if(udb.localfriend_size()>0){
                for(int i = 0; i<udb.localfriend_size(); i++){
                    const rm::UserData& ud = udb.localfriend(i);
                    rm::UserData *cud = cudb.add_localfriend();
                    bf->Encrypt(&cypheredData, ud.user());
                    cud->set_user(cypheredData);
                    bf->Encrypt(&cypheredData, ud.ip());
                    cud->set_ip(cypheredData);
                    bf->Encrypt(&cypheredData, ud.port());
                    cud->set_port(cypheredData);
                    bf->Encrypt(&cypheredData, ud.date());
                    cud->set_date(cypheredData);
                    bf->Encrypt(&cypheredData, ud.key());
                    cud->set_key(cypheredData);
                }
            }
            if(udb.externalfriend_size()>0){
                for(int i = 0; i<udb.externalfriend_size(); i++){
                    const rm::ExternalUserData& ud = udb.externalfriend(i);
                    rm::ExternalUserData *cud =cudb.add_externalfriend();
                    bf->Encrypt(&cypheredData, ud.user());
                    cud->set_user(cypheredData);
                    bf->Encrypt(&cypheredData, ud.server());
                    cud->set_server(cypheredData);
                    bf->Encrypt(&cypheredData, ud.key());
                    cud->set_key(cypheredData);
                }
            }
            QByteArray serialArray;
            serialArray.append("006");
            serialArray.append(cudb.SerializeAsString().c_str(),cudb.ByteSize());
            server->sendExternalMsg(serialArray);
            state++;
            break;
        }
        case 3:{
            //Para finalizar le enviamos sus salas abiertas para que continue con el host
            std::string cypheredData;
            //Accedemos a la bd en busca de l alista de amigos
            DBFactory dbfactory;
            BaseDBController *db = dbfactory.getDB();
            rm::roomData rd = db->getUsersRoom(auth.user());
            if(rd.room().compare("")==0){
                rm::ServerResponse sr;
                bf->Encrypt(&cypheredData,"0");
                sr.set_code(cypheredData);
                bf->Encrypt(&cypheredData,"You have not active rooms.");
                sr.set_description(cypheredData);
                QByteArray serialArray;
                serialArray.append("001");
                serialArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
                server->sendExternalMsg(serialArray);
            }else{
                bf->Encrypt(&cypheredData,rd.user());
                rd.set_user(cypheredData);
                LocalService ls;
                bf->Encrypt(&cypheredData,rd.room()+"#"+ls.getServerName().toUtf8().constData());
                rd.set_room(cypheredData);
                bf->Encrypt(&cypheredData,rd.pass());
                rd.set_pass(cypheredData);
                if(rd.permissions().compare("passprotected")){
                    bf->Encrypt(&cypheredData,rd.accesspass());
                    rd.set_accesspass(cypheredData);
                }
                bf->Encrypt(&cypheredData,rd.permissions());
                rd.set_permissions(cypheredData);
                QByteArray serialArray;
                serialArray.append("015");
                serialArray.append(rd.SerializeAsString().c_str(),rd.ByteSize());
                server->sendExternalMsg(serialArray);
            }
            state++;
            break;
        }default:{
            server->closeConnection();
        }
        }
    }catch(int e){
        this->server->closeConnection();
    }
}
