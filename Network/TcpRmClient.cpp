#include "TcpRmClient.h"
#include <QString>
#include <QStringList>
#include "TcpClientThread.h"

TcpRmClient::TcpRmClient()
{
    handshake = true;
}

void TcpRmClient::awake(std::string message){
    handleHandshake(message);
}

void TcpRmClient::awake(std::string msgType, QByteArray byteArray){
    ownWork(msgType, byteArray);
}

bool TcpRmClient::isHandshake(){
    return handshake;
}

void TcpRmClient::handleHandshake(std::string message){
    try{
        switch(state){
        case(0):{
            QString aux = QString::fromStdString(message);
            QStringList list;
            list = aux.split(QRegExp("\\s+"));
            remoteKeys.n = list[0].toUInt();
            remoteKeys.e = list[1].toUInt();
            aux = QString::number(keys->n)+" "+QString::number(keys->e);
            thread->sendMsg(aux.toUtf8().constData());
            state++;
            break;
        }
        case(1):{
            QString aux = QString::fromStdString(message);
            QStringList list;
            list = aux.split(QRegExp("\\s"));
            bfk.cypha = list[0].toUInt();
            bfk.cyphb = list[1].toUInt();
            bfk.cyphc = list[2].toUInt();
            unsigned int sign = list[3].toUInt();
            if(crsa->decypherSignature(remoteKeys, sign)==7273){
                crsa->decypherSymmetricKey(remoteKeys, &bfk);
                QString bfKey = "";
                bfKey+=(char)bfk.a;
                bfKey+=(char)bfk.b;
                bfKey+=(char)bfk.c;
                bfKey+=(char)bfk.d;
                bfKey+=(char)bfk.e;
                bfKey+=(char)bfk.f;
                bf = new BlowFish();
                bf->SetKey(bfKey.toUtf8().constData());
                //enviamos el tipo de conexión
                sendConnType();

            }else{
                qDebug()<<"firma del servidor remoto incorrecta";
                socket->disconnectFromHost();
            }
            state=0;
            handshake = false;
            break;
        }
        }
    }catch(int i){
        qDebug()<<"Se ha producido un error en el proceso de confirmación de invitación";
        socket->disconnectFromHost();
    }
}

bool TcpRmClient::checkUname(std::string uname){
    //Comprobamos la longitud de los nombres de usuario
    if((uname.length()<4)||(uname.length()>22)){
        return false;
    }
    //Comprobamos la validez de los caracteres
    int ulenght = uname.length();
    for(int i = 0; i<ulenght; i++){
        if((((int)uname.at(i))<33) || (((int)uname.at(i))>126) || (((int)uname.at(i))==34) || (((int)uname.at(i))==39) || (((int)uname.at(i))==64)){
            return false;
        }
    }
    return true;
}

bool TcpRmClient::checkPassword(std::string pass){
    //Comprobamos la longitud de los nombres de usuario
    if((pass.length()<9)||(pass.length()>22)){
       return false;
    }
    //Comprobamos la validez de los caracteres
    int plenght = pass.length();
    for(int i = 0; i<plenght; i++){
        if((((int)pass.at(i))<33) || (((int)pass.at(i))>126) || (((int)pass.at(i))==34) || (((int)pass.at(i))==39) || (((int)pass.at(i))==64)){
            return false;
        }
    }
    return true;
}
