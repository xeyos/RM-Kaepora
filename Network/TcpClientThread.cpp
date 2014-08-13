#include "TcpClientThread.h"
#include "RedMalk.pb.h"
#include "TcpRmInvitationAdmin.h"
#include "TcpRmInvitatonResender.h"
#include "TcpRmPNSender.h"
#include <QHostAddress>

TcpClientThread::TcpClientThread(RsaKeys *k, CustomRsa *rsa, int type, QByteArray data, QString destinatary)
{
    this->destinatary = destinatary;
    this->type = type;
    qDebug()<<"Creamos cliente de tipo "+this->type;
    this->data = data;
    crsa = rsa;
    keys = k;
    rmc = NULL;
}

TcpClientThread::TcpClientThread(RsaKeys *k, CustomRsa *rsa, int type, QByteArray data){
    this->type = type;
    qDebug()<<"Creamos cliente de tipo "+this->type;
    this->data = data;
    crsa = rsa;
    keys = k;
    rmc = NULL;
}

void TcpClientThread::work(){
    if((this->type ==1)||(this->type ==3)||(this->type ==5)){
        switch(this->type){
            case 1:{
                //New invitacion recived, sending to kaepora
                rm::Invitation invite;
                invite.ParseFromArray(data,data.size());
                socket = new QTcpSocket(this);
                socket->abort();
                connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
                connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
                socket->connectToHost(QString::fromStdString(invite.server()),1432);
                if(!socket->waitForConnected(5000)){
                    this->disconnected();
                }
                rmc = new TcpRmInvitationAdmin(socket, this, invite, keys, crsa);
                break;
            }
            case 3:{
                //Invitation confirmation to other kaepora
                rm::serverInvitationVerif siv;
                siv.ParseFromArray(data, data.size());
                socket = new QTcpSocket(this);
                socket->abort();
                connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
                connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
                rmc = new TcpRmInvitatonResender(socket, this, siv, keys, crsa, "6");
                break;
            }
            case 5:{
                //Invitation confirmation to other kaepora
                rm::serverInvitationVerif siv;
                siv.ParseFromArray(data, data.size());
                socket = new QTcpSocket(this);
                socket->abort();
                connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
                connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
                rmc = new TcpRmInvitatonResender(socket, this, siv, keys, crsa, "9");
                break;
            }
        }
    }else{
        switch(this->type){
        case 6:{
            //Invitation confirmation to other kaepora
            rm::PersonalNew siv;
            siv.ParseFromArray(data, data.size());
            socket = new QTcpSocket(this);
            socket->abort();
            connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
            connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
            rmc = new TcpRmPNSender(socket, this, siv, keys, crsa, destinatary);
            break;
        }
    }
    }
}

void TcpClientThread::readyRead(){
    if(rmc->isHandshake()){
        rmc->awake(getNerworkString());
    }else{
        std::string type = socket->read(3).constData();
        rmc->awake(type, socket->readAll());
    }
}

std::string TcpClientThread::getNerworkString(){
    QByteArray receiveArray;
    const char* aux;
    receiveArray.append(socket->readAll());
    aux=receiveArray.constData();
    return aux;
}

void TcpClientThread::sendMsg(std::string msg){
    socket->write(msg.c_str());
}

void TcpClientThread::sendArray(QByteArray byteArray){
    socket->write(byteArray);
}

QString TcpClientThread::getClientIp(){
    return socket->peerAddress().toString();
}

void TcpClientThread::disconnectFromHost(){
    socket->disconnectFromHost();
}

void TcpClientThread::disconnected(){
    if(rmc != NULL){
        delete (rmc);
        rmc = NULL;
    }
    socket->deleteLater();
    socket = NULL;
    emit finished();
}
