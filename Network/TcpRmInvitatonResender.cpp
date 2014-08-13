#include "TcpRmInvitatonResender.h"
#include "LocalService.h"
#include "TcpClientThread.h"

TcpRmInvitatonResender::TcpRmInvitatonResender(QTcpSocket *s, TcpClientThread *tst, rm::serverInvitationVerif siv, RsaKeys *k, CustomRsa *rsa, std::string connType)
{
    this->connType = connType;
    this->thread = tst;
    this->socket = s;
    crsa = rsa;
    keys = k;
    state = 0;
    this->siv = siv;
    socket->connectToHost(QString::fromStdString(siv.server()),1432);
    if(!socket->waitForConnected(5000)){
        thread->disconnected();
    }
}

void TcpRmInvitatonResender::ownWork(std::string msgType, QByteArray byteArray){
    std::string cyphered;
    bf->Encrypt(&cyphered, siv.userinvited());
    siv.set_userinvited(cyphered);
    bf->Encrypt(&cyphered, siv.userinviting());
    siv.set_userinviting(cyphered);
    bf->Encrypt(&cyphered, siv.key());
    siv.set_key(cyphered);
    bf->Encrypt(&cyphered, siv.response());
    siv.set_response(cyphered);
    LocalService ls;
    bf->Encrypt(&cyphered, ls.getServerName().toUtf8().constData());
    siv.set_server(cyphered);
    QByteArray serialArray;
    serialArray.append("008");
    serialArray.append(siv.SerializeAsString().c_str(), siv.ByteSize());
    thread->sendArray(serialArray);
}

void TcpRmInvitatonResender::sendConnType(){
    handshake = false;
    std::string cypheredConnType;
    bf->Encrypt(&cypheredConnType, connType);
    thread->sendMsg(cypheredConnType);
}
