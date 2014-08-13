#include "TcpRmInvitationAdmin.h"
#include <QRegExp>
#include <time.h>
#include "LocalService.h"
#include "DB/BaseDBController.h"
#include "DB/DBFactory.h"
#include "TcpClientThread.h"

TcpRmInvitationAdmin::TcpRmInvitationAdmin(QTcpSocket *s, TcpClientThread *tst, rm::Invitation invite, RsaKeys *k, CustomRsa *rsa)
{
    this->thread = tst;
    this->socket = s;
    crsa = rsa;
    keys = k;
    state = 0;
    this->invite = invite; 
}

void TcpRmInvitationAdmin::ownWork(std::string type, QByteArray byteArray){
    std::string cyphered;
    rm::serverInvitationVerif siv;
    qDebug()<<"Verificamos que realmente "+QString::fromStdString(invite.user())+" quiere invitar a "+QString::fromStdString(invite.destinatary());
    bf->Encrypt(&cyphered, invite.user());
    siv.set_userinviting(cyphered);
    bf->Encrypt(&cyphered, invite.destinatary());
    siv.set_userinvited(cyphered);
    LocalService ls;
    std::string serverName;
    serverName = ls.getServerName().toUtf8().constData();
    bf->Encrypt(&cyphered, serverName);
    siv.set_server(cyphered);
    int a = rand()%9+48;
    int b = rand()%9+48;
    int c = rand()%9+48;
    std::string localpass="";
    localpass+=a;
    localpass+=b;
    localpass+=c;
    bf->Encrypt(&cyphered, localpass);
    siv.set_key(cyphered);
    QByteArray serialArray;
    serialArray.append("008");
    serialArray.append(siv.SerializeAsString().c_str(),siv.ByteSize());
    DBFactory dbfactory;
    BaseDBController *db = dbfactory.getDB();
    db->createPendingInvitation(invite);
    thread->sendArray(serialArray);
}

void TcpRmInvitationAdmin::sendConnType(){
    handshake = false;
    std::string connType = "3", cypheredConnType;
    bf->Encrypt(&cypheredConnType, connType);
    thread->sendMsg(cypheredConnType);
}
