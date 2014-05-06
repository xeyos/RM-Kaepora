#include <QDebug>
#include "TcpServerThread.h"
#include "Proto/RedMalk.pb.h"
using namespace rm;

TcpServerThread::TcpServerThread(qintptr ID, RsaKeys *k, CustomRsa *rsa)
{
    crsa = rsa;
    keys = k;
    this->socketDescriptor = ID;
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
    QString aux = QString::number(keys->n)+" "+QString::number(keys->e);
    sendMsg(aux.toUtf8().constData());

}


void TcpServerThread::readyRead()
{
    switch(state){
    case 0:{
        std::string reciever = obtenerMensaje();
        QString aux = QString::fromStdString(reciever);
        //divido el mensaje en las 2 claves y las paso a uint
        QStringList list;
        list = aux.split(QRegExp("\\s+"));
        remoteKeys.n=list[0].toUInt();
        remoteKeys.e=list[1].toUInt();
        //Gnero una clave para blowfish
        bfKey.a=rand()%(99-10)+10;
        bfKey.b=rand()%(99-10)+10;
        bfKey.c=rand()%(99-10)+10;
        bfKey.d=rand()%(99-10)+10;
        bfKey.e=rand()%(99-10)+10;
        bfKey.f=rand()%(99-10)+10;
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
        //Recibimos los datos de versión
        std::string aux = obtenerMensaje();
        std::string datoDescifrado;
        VersionPackage version;
        //std::string sVersion = aux.toUtf8().constData();
        version.ParseFromString(aux);
        bf->Decrypt(&datoDescifrado, version.system());
        version.set_system(datoDescifrado);
        bf->Decrypt(&datoDescifrado, version.so());
        version.set_so(datoDescifrado);
        bf->Decrypt(&datoDescifrado, version.concreteversion());
        version.set_concreteversion(datoDescifrado);
        qDebug()<<QString::fromStdString(version.system());
        qDebug()<<QString::fromStdString(version.so());
        qDebug()<<QString::fromStdString(version.concreteversion());
        break;
    }
    }


}

std::string TcpServerThread::obtenerMensaje(){
    QByteArray receiveArray;
    //TcpDataStruct structCall;
    QString aux;
    const char* aux2;
    receiveArray.append(socket->readAll());
    QDataStream in(&receiveArray,QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_8);
    in >> aux;
    aux2=receiveArray.constData();
    return aux2;
}

void TcpServerThread::sendMsg(std::string msg){
    socket->write(msg.c_str());
}

void TcpServerThread::disconnected()
{
    socket->deleteLater();
    emit finished();
}
