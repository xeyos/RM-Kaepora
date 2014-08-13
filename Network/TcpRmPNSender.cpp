#include "TcpRmPNSender.h"
#include "DB/BaseDBController.h"
#include "DB/DBFactory.h"
#include "DB/InternalUserData.h"
#include "TcpClientThread.h"

TcpRmPNSender::TcpRmPNSender(QTcpSocket *s, TcpClientThread *tst, rm::PersonalNew invite, RsaKeys *keys, CustomRsa *rsa, QString destinatary)
{
    this->thread = tst;
    this->socket = s;
    crsa = rsa;
    this->keys = keys;
    state = 0;
    siv = invite;
    this->destinatary = destinatary.toUtf8().constData();
    DBFactory dbfactory;
    BaseDBController *db = dbfactory.getDB();
    InternalUserData iud = db->getUserData(this->destinatary);
    qDebug()<<"Tratando de conectar con el cliente";
    if(iud.ip.compare("")==0){
        thread->disconnected();
    }else{
        socket->connectToHost(QString::fromStdString(iud.ip),iud.port);
        if(!socket->waitForConnected(5000)){
            thread->disconnected();
        }
    }
}

void TcpRmPNSender::sendConnType(){
    std::string cypheredConnType;
    bf->Encrypt(&cypheredConnType, "7");
    thread->sendMsg(cypheredConnType);
    handshake = false;
    state=0;
}

void TcpRmPNSender::ownWork(std::string type, QByteArray byteArray){
    try{
        std::string decrypted;
        rm::Auth auth;
        auth.ParseFromArray(byteArray, byteArray.size());
        bf->Decrypt(&decrypted,auth.user());
        std::transform(decrypted.begin(), decrypted.end(), decrypted.begin(), ::tolower);
        auth.set_user(decrypted);
        bf->Decrypt(&decrypted,auth.password());
        auth.set_password(decrypted);
        bf->Decrypt(&decrypted,auth.port());
        auth.set_port(decrypted);
        qDebug()<<" Su usuario es "+QString::fromStdString(auth.user());
        if(checkUname(auth.user()) && checkPassword(auth.password())){
            std::string encrypted;
            DBFactory dbfactory;
            BaseDBController *db = dbfactory.getDB();
            int result = db->login(auth, thread->getClientIp());
            if((result ==0) && (destinatary.compare(auth.user())==0)){
                QByteArray serialArray;
                serialArray.append("002");
                if(siv.type().compare("invitation accepted")==0){
                    qDebug()<<"ahora sois amigos";
                    bf->Encrypt(&encrypted, siv.internaldata());
                    siv.set_internaldata(encrypted);
                }else if(siv.type().compare("invitation confirmation")==0){
                    qDebug()<<"Lo has invitado tu?";
                    bf->Encrypt(&encrypted, siv.internaldata());
                    siv.set_internaldata(encrypted);
                }else if(siv.type().compare("invitation")==0){
                    qDebug()<<"Tienes una invitación";
                    bf->Encrypt(&encrypted, siv.internaldata());
                    siv.set_internaldata(encrypted);
                    bf->Encrypt(&encrypted, siv.internaldataaux());
                    siv.set_internaldataaux(encrypted);
                    bf->Encrypt(&encrypted, siv.content());
                    siv.set_content(encrypted);
                }
                bf->Encrypt(&encrypted, siv.user());
                siv.set_user(encrypted);
                bf->Encrypt(&encrypted, siv.userserver());
                siv.set_userserver(encrypted);
                bf->Encrypt(&encrypted, siv.type());
                siv.set_type(encrypted);
                serialArray.append(siv.SerializeAsString().c_str(), siv.ByteSize());
                thread->sendArray(serialArray);
                thread->disconnectFromHost();
            }else{
                qDebug()<<"Datos incorrectos en invite";
                thread->disconnectFromHost();
            }
        }else{
            qDebug()<<"Datos incorrectos en login/invite";
            thread->disconnectFromHost();
        }
    }catch (int e){
        qDebug()<<"Error durante la interpretación del mensaje de verif.";
        thread->disconnectFromHost();
    }
}
