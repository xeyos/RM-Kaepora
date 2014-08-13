#include "TcpRmRegisterer.h"
#include "DB/BaseDBController.h"
#include "DB/DBFactory.h"
#include "QDebug"
#include <fstream>
#include <QApplication>
using namespace rm;
TcpRmRegisterer::TcpRmRegisterer()
{
}

void TcpRmRegisterer::setSocket(TcpServerThread *server, BlowFish *bf){
    this->server = server;
    this->bf = bf;
    //Al crear e inicializar el objeto, devolvemos
    //el estado del servidor al cliente a modo de saludo.
    rm::ServerStatus sStatus;
    std::string cypheredMsg;
    bf->Encrypt(&cypheredMsg, std::string()+(char)server->getServerMode());
    sStatus.set_registeringstatus(cypheredMsg);
    bf->Encrypt(&cypheredMsg, std::string()+(char)100);
    sStatus.set_avaliability(cypheredMsg);
    bf->Encrypt(&cypheredMsg, "Ready to serve");
    sStatus.set_generalstatus(cypheredMsg);
    QByteArray sendArray;
    sendArray.append("011");
    sendArray.append(sStatus.SerializeAsString().c_str(),sStatus.ByteSize());
    this->server->sendExternalMsg(sendArray);
}

void TcpRmRegisterer::awake(std::string type, QByteArray byteArray){
    try{
        code ="";
        std::string decypheredData;
        rd.ParseFromArray(byteArray, byteArray.size());
        bf->Decrypt(&decypheredData, rd.uname());
        std::transform(decypheredData.begin(), decypheredData.end(), decypheredData.begin(), ::tolower);
        rd.set_uname(decypheredData);
        bf->Decrypt(&decypheredData, rd.email());
        rd.set_email(decypheredData);
        bf->Decrypt(&decypheredData, rd.password());
        rd.set_password(decypheredData);
        std::string cypheredData;
        ServerResponse sr;
        if((checkUname(rd.uname())) && (checkEMail(rd.email())) && (checkPassword(rd.password()))){
            qDebug()<<"Petición de usuario válido";
            DBFactory dbfactory;
            BaseDBController *db = dbfactory.getDB();
            int result = db->createUser(rd,code) == 0;
            if(result){
                qDebug()<<"Usuario creado";
                sr.set_code("0");
            }else if(result){
                sr.set_code("2");
                sr.set_description("Server is having some database problems, try later.");
            }else{
                sr.set_code("1");
                sr.set_description("User or email in already use.");
            }
            bf->Encrypt(&cypheredData, sr.code());
            sr.set_code(cypheredData);
            bf->Encrypt(&cypheredData, sr.description());
            sr.set_description(cypheredData);
            QByteArray sendArray;
            sendArray.append("001");
            sendArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
            server->sendExternalMsg(sendArray);
            if((sr.code()=="2")||(sr.code() == "0"))
                server->disconnect();
        }
    }catch(int e){
        qDebug()<<"error recibiendo datos de registro";
        server->disconnect();
    }
}

bool TcpRmRegisterer::checkEMail(std::string aux){
    QString pattern = "^[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,4})$";
    QString email = QString::fromStdString(aux);
    QRegExp regexp(pattern);
    if ((aux.length()>0)  && (regexp.exactMatch(email))){
        rd.set_email(aux);
        return true;
    }
    invalidUserData("Invalid e-mail adress.");
    return false;
}

bool TcpRmRegisterer::checkPassword(std::string aux){
    //Comprobamos la longitud de los nombres de usuario
    if((aux.length()<10)||(aux.length()>22)){
        invalidUserData("Invalid password size (5,21)");
        return false;
    }
    //Comprobamos la validez de los caracteres
    int plenght = aux.length();
    for(int i = 0; i<plenght; i++){
        if((((int)aux.at(i))<33) || (((int)aux.at(i))>126) || (((int)aux.at(i))==34) || (((int)aux.at(i))==39) || (((int)aux.at(i))==64)){
            invalidUserData("The password contains invalid characters.");
            return false;
        }
    }
    rd.set_password(aux);
    return true;
}

bool TcpRmRegisterer::checkUname(std::string aux){
    std::string uname, code;
    //Comprobamos si existe un código de invitación
    if(aux.find('@')== std::string::npos){
        if(server->getServerMode() == 0)
            uname=aux;
        else{
            invalidUserData("Server is in private registering mode only");
            return false;
        }
    }else{
        uname = aux.substr(0, aux.find('@'));
        code = aux.substr(aux.find('@')+1, aux.size()-1);
    }
    //Comprobamos la longitud de los nombres de usuario
    if((uname.length()<4)||(uname.length()>22)){
        invalidUserData("Invalid nickname size "+uname);
        return false;
    }
    //Comprobamos la validez de los caracteres
    int ulenght = uname.length();
    for(int i = 0; i<ulenght; i++){
        if((((int)uname.at(i))<33) || (((int)uname.at(i))>126) || (((int)uname.at(i))==34) || (((int)uname.at(i))==39) || (((int)uname.at(i))==64)){
            invalidUserData("The nickname contains invalid characters.");
            return false;
        }
    }
    ulenght = code.length();
    if(code!=""){
        for(int i = 0; i<ulenght; i++){
            if((((int)code.at(i))<33) || (((int)code.at(i))>126) || (((int)code.at(i))==34) || (((int)code.at(i))==39) || (((int)code.at(i))==64)){
                invalidUserData("The nicknbame contains @ witch is a code marker, but code is invalid.");
                return false;
            }
        }
    }

    //Validamos el código de registro
    if(server->getServerMode()==1){
        if(code==""){
            invalidUserData("@ is an access code marker(for private servers), cant be used on stardar nicknames");
            return false;
        }
        std::ifstream codes (qApp->applicationDirPath().toUtf8()+"/accessKeys");
        std::string codigo;
        if(codes){
            bool valid = false;
            while(std::getline(codes,codigo)){
                if(codigo.compare(code)==0){
                    valid = true;
                    break;
                }
            }
            if(!valid){
                invalidUserData("Invalid registering code");
                return false;
            }
        }
    }else{
        code ="";
    }
    this->code = code;
    rd.set_uname(uname);
    return true;
}

void TcpRmRegisterer::invalidUserData(std::string msg){
    qDebug()<<QString::fromStdString(msg);
    std::string cypheredResult;
    rm::ServerResponse sr;
    bf->Encrypt(&cypheredResult,"1");
    sr.set_code(cypheredResult);
    bf->Encrypt(&cypheredResult,msg);
    sr.set_description(cypheredResult);
    QByteArray sendArray;
    sendArray.append("001");
    sendArray.append(sr.SerializeAsString().c_str(),sr.ByteSize());
    server->sendExternalMsg(sendArray);
}
