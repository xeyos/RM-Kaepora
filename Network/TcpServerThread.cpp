#include <QDebug>
#include <QHostAddress>
#include <QTime>
#include "TcpServerThread.h"
#include "Proto/RedMalk.pb.h"
#include "TcpRmRegisterer.h"
#include "TcpRmConnector.h"
#include "TcpRmInvitationListener.h"
#include "TcpRmInvitationValidator.h"
#include "TcpRmPersonalNoticesAdmin.h"
#include "TcpRmInvitationEnder.h"
#include "TcpRmInvitationResponser.h"
#include "TcpRmLocalFriendServer.h"
#include "TcpRmFriendSender.h"
#include "chatRooms/TcpRmChatResponser.h"
#include "chatRooms/TcpRmChatRegisterer.h"
#include "chatRooms/TcpRmChatRemover.h"
#include "chatRooms/TcpRmChatClamer.h"

using namespace rm;

TcpServerThread::TcpServerThread(qintptr ID, RsaKeys *k, CustomRsa *rsa, int serverMode){
    this->serverMode = serverMode;
    rm = NULL;
    crsa = rsa;
    keys = k;
    this->socketDescriptor = ID;
    blocked = false;
    state =0;
}

void TcpServerThread::work()
{
    socket = new QTcpSocket(this);
    if(!socket->setSocketDescriptor(this->socketDescriptor))
    {
        emit error(socket->error());
        return;
    }
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket,SIGNAL(disconnected()),socket,SLOT(deleteLater()));
    //Nada más crear el socket, enviamos al cliente nuestra clave RSA pública para
    //evitar pasar ningún tipo de información sin cifrar.
    QString aux = QString::number(keys->n)+" "+QString::number(keys->e);
    sendMsg(aux.toUtf8().constData());
}


void TcpServerThread::readyRead()
{
    switch(state){
    //Si estamos en el estado 0, quiere decir que no hemos recibudo aun las claves del
    //cliente
    case 0:{
        std::string reciever = getNetworkString();
        QString aux = QString::fromStdString(reciever);
        //divido el mensaje en las 2 claves y las paso a uint
        QStringList list;
        list = aux.split(QRegExp("\\s+"));
        remoteKeys.n=list[0].toUInt();
        remoteKeys.e=list[1].toUInt();
        //Ahora que tngo las claves del cliente, genero una clave para blowfish
        bfKey.a=rand()%25+65;
        bfKey.b=rand()%25+65;
        bfKey.c=rand()%25+65;
        bfKey.d=rand()%25+65;
        bfKey.e=rand()%25+65;
        bfKey.f=rand()%25+65;
        //la cifro, la firmo y la envío
        crsa->cypherSymmetricKey(remoteKeys,&bfKey);
        unsigned int sign = crsa->cypherSignature(7273);
        aux = QString::number(bfKey.cypha)+" "+QString::number(bfKey.cyphb)+" "+QString::number(bfKey.cyphc)+" "+QString::number(sign);
        //Creamos la instancia del blowfish y le pasamos su clave
        bf = new BlowFish();
        QString bfk="";
        bfk+=(char)bfKey.a;
        bfk+=(char)bfKey.b;
        bfk+=(char)bfKey.c;
        bfk+=(char)bfKey.d;
        bfk+=(char)bfKey.e;
        bfk+=(char)bfKey.f;
        bf->SetKey(bfk.toUtf8().constData());
        sendMsg(aux.toUtf8().constData());
        state++;
        break;
    }
    case 1:{
        //Escuchamos el tipo de operación que necesita realizar el cliente
        std::string aux = getNetworkString();
        std::string datoDescifrado;
        bf->Decrypt(&datoDescifrado,aux);
        try{            
            int type = atoi(datoDescifrado.c_str());
            QTime time = QTime::currentTime();
            QString timeString = time.toString();
            qDebug()<<timeString+"----------------------------------------------------------------------------------";
            qDebug()<<"Un cliente se conecta en busca de una operación de tipo "+QString::fromStdString(datoDescifrado.c_str());
            switch(type){
                case 0:{
                    qDebug()<<"Quiere registrarse";
                    state =2;
                    rm = new TcpRmRegisterer();
                    rm->setSocket(this, bf);
                    break;
                }
                case 1:{
                    qDebug()<<"Quiere logearse";
                    state=2;
                    rm = new TcpRmConnector();
                    rm->setSocket(this, bf);
                    break;
                }
                case 2:{
                    state=2;
                    qDebug()<<"Quiere invitar a un nuevo amigo";
                    rm = new TcpRmInvitationListener();
                    rm->setSocket(this, bf);
                    break;
                }
                case 3:{
                    qDebug()<<"Es otro server que pide que valide una invitación";
                    state=2;
                    rm = new TcpRmInvitationValidator();
                    rm->setSocket(this, bf);
                    break;
                }
                case 4:{
                    qDebug()<<"Quiere que le facilite la info de sus amigos en este server";
                    state=2;
                    rm = new TcpRmFriendSender();
                    rm->setSocket(this, bf);
                    break;
                }
                case 5:{//Peticion de amigos a server local
                    qDebug()<<"Es un usuario de este server, quiere quele facilite sus amigos locales";
                    state=2;
                    rm = new TcpRmLocalFriendServer();
                    rm->setSocket(this, bf);
                    break;
                }
                case 6:{
                    qDebug()<<"Es otro server que nos confirma una invitación válida";
                    state=2;
                    rm = new TcpRmInvitationEnder();
                    rm->setSocket(this, bf);
                    break;
                }
                case 8:{
                    qDebug()<<"Quiere responder a una noticia enviada";
                    state=2;
                    rm = new TcpRmPersonalNoticesAdmin();
                    rm->setSocket(this, bf);
                    break;
                }                
                case 9:{
                    qDebug()<<"Otro server nos envía la respuesta auna invitación de amistad";
                    state=2;
                    rm = new TcpRmInvitationResponser();
                    rm->setSocket(this, bf);
                    break;
                }
                case 12:{
                    qDebug()<<"Solicitud de creación de sala de chat pública";
                    state=2;
                    rm = new TcpRmChatRegisterer();
                    rm->setSocket(this, bf);
                    break;
                }
                case 13:{
                    qDebug()<<"Solicitud de datos de sala";
                    state=2;
                    rm = new TcpRmChatResponser();
                    rm->setSocket(this, bf);
                    break;
                }
                case 16:{
                    qDebug()<<"Solicitud de eliminación de sala";
                    state=2;
                    rm = new TcpRmChatRemover();
                    rm->setSocket(this, bf);
                    break;
                }
                case 17:{
                    qDebug()<<"Reclamo de sala";
                    state = 2;
                    rm = new TcpRmChatClamer();
                    rm->setSocket(this, bf);
                    break;
                }
                default:{
                    socket->disconnectFromHost();
                    qDebug()<<"Número de operación erroneo";
                    break;
                }
            }
        }catch(int e){
            socket->disconnectFromHost();
            qDebug()<<"No fue posible determinar la operación, desconectado";
        }
        break;
    }
    case 2:{
        std::string type = socket->read(3).constData();
        rm->awake(type, socket->readAll());
        break;
    }
    }
}

void TcpServerThread::blockThread(bool value){
    this->blocked = value;
}

void TcpServerThread::changeState(int type, QByteArray data, QString destinatary){
    emit clientThreadNeeded(type, data, destinatary);
    socket->disconnectFromHost();
}

void TcpServerThread::changeState(int type, QByteArray data){
    emit clientThreadNeeded(type, data);
    socket->disconnectFromHost();
}

std::string TcpServerThread::getNetworkString(){
    QByteArray receiveArray;
    const char* aux;
    receiveArray.append(socket->readAll());
    aux=receiveArray.constData();
    return aux;
}

void TcpServerThread::sendMsg(std::string msg){
    socket->write(msg.c_str());
}

void TcpServerThread::sendExternalMsg(QByteArray byteArray){
    socket->write(byteArray);
}

void TcpServerThread::closeConnection(){
    socket->disconnectFromHost();
}

int TcpServerThread::getServerMode(){
    return serverMode;
}

QString TcpServerThread::getClientIp(){
    return socket->peerAddress().toString();
}

void TcpServerThread::disconnected()
{

    if(bf != NULL){
        delete(bf);
        bf = NULL;
    }
    if(rm != NULL){
        delete(rm);
        rm = NULL;
    }
    if (!this->blocked){
        emit finished();
    }
}

void TcpServerThread::sendStatus(){
    rm::ServerStatus sStatus;
    std::string cypheredMsg;
    bf->Encrypt(&cypheredMsg, std::string()+(char)getServerMode());
    sStatus.set_registeringstatus(cypheredMsg);
    bf->Encrypt(&cypheredMsg, std::string()+(char)100);
    sStatus.set_avaliability(cypheredMsg);
    bf->Encrypt(&cypheredMsg, "Ready to serve");
    sStatus.set_generalstatus(cypheredMsg);
    QByteArray sendArray;
    sendArray.append("011");
    sendArray.append(sStatus.SerializeAsString().c_str(),sStatus.ByteSize());
    sendExternalMsg(sendArray);
}
