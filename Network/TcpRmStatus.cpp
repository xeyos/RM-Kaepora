#include "TcpRmStatus.h"

bool TcpRmStatus::checkUname(std::string aux){
    std::string uname, code;
    uname=aux;
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

bool TcpRmStatus::checkPassword(std::string aux){
    //Comprobamos la longitud de los nombres de usuario
    if((aux.length()<9)||(aux.length()>22)){
       return false;
    }
    //Comprobamos la validez de los caracteres
    int plenght = aux.length();
    for(int i = 0; i<plenght; i++){
        if((((int)aux.at(i))<33) || (((int)aux.at(i))>126) || (((int)aux.at(i))==34) || (((int)aux.at(i))==39) || (((int)aux.at(i))==64)){
            return false;
        }
    }
    return true;
}

bool TcpRmStatus::checkShortPassword(std::string aux){
    //Comprobamos la longitud de los nombres de usuario
    if((aux.length()<5)||(aux.length()>22)){
       return false;
    }
    //Comprobamos la validez de los caracteres
    int plenght = aux.length();
    for(int i = 0; i<plenght; i++){
        if((((int)aux.at(i))<33) || (((int)aux.at(i))>126) || (((int)aux.at(i))==34) || (((int)aux.at(i))==39) || (((int)aux.at(i))==64)){
            return false;
        }
    }
    return true;
}
