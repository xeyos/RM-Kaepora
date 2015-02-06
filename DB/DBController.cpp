#include "DBController.h"
#include "LocalService.h"
#include <ctime>
#include <QDebug>

using namespace pqxx;
DBController::DBController()
{
}

bool DBController::setAccessData(std::string dbname, std::string user, std::string password, std::string host, std::string port){
    conn = new connection(std::string()+"dbname="+dbname+" user="+user+" password="+password+"  hostaddr="+host+" port="+port);
    conn->prepare("searchUser", "SELECT * FROM users WHERE UName = $1");
    conn->prepare("search", "SELECT * FROM users WHERE UName = $1 OR  Email = $2");
    conn->prepare("newUser","INSERT INTO users (UName, Email, Password, InvitationCode, LastConnection) VALUES ($1,$2,$3, $4, $5)");
    conn->prepare("loginSearch", "SELECT * FROM users WHERE UName = $1 AND password = $2");
    conn->prepare("loginUpdate","UPDATE users SET ip=$1, port=$2, lastconnection=$3 WHERE uname=$4");
    conn->prepare("personalNewsSearch", "SELECT * FROM friendnotices WHERE destinatary=$1 AND state =true");
    conn->prepare("systemNewsSearch", "SELECT * FROM servernotices WHERE (system=$1 AND os=$2 AND version>$3) OR (system ='kaepora')");
    conn->prepare("localFriendList", "SELECT * FROM localfriendships WHERE (usera = $1) OR (userb=$2)");
    conn->prepare("remoteFriendList", "SELECT * FROM friendships WHERE localuser =$1");
    conn->prepare("externalFriendList", "SELECT * FROM friendships WHERE remoteuser =$1 AND remoteserver = $2");
    conn->prepare("searchLocalfriendship", "SELECT * from localfriendships WHERE ((usera = $1) AND (userb = $2)) OR ((userb = $1) AND (usera = $2))");
    conn->prepare("searchRemoteFriendship", "SELECT * FROM friendships WHERE localuser = $1 AND remoteuser = $2 AND remoteserver = $3");
    //Invitations
    conn->prepare("searchUnconfirmedInvitation", "SELECT * FROM friendnotices WHERE (destinatary=$1) AND (owner = $2) AND (type = $3) AND (ownerserver = $4)");
    conn->prepare("deleteUnconfirmedInvitation", "DELETE FROM friendnotices WHERE (destinatary=$1) AND (owner = $2) AND (type = $3) AND (ownerserver = $4)");
    conn->prepare("addUnconfirmedInvitation", "INSERT INTO friendnotices (ownerserver, type, daterecieved, message, destinatary, owner, internaldata, state, internaldataaux) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9)");
    conn->prepare("confirmInvitation", "UPDATE friendnotices SET state = true, internaldataaux = $5 WHERE (destinatary=$1) AND (owner = $2) AND (type = $3) AND (ownerserver = $4)");
    conn->prepare("confirmInvitationSent", "UPDATE friendnotices SET state = false, internaldataaux = $5 WHERE (destinatary=$1) AND (owner = $2) AND (type = $3) AND (ownerserver = $4)");
    conn->prepare("newExternalFriend", "INSERT INTO friendships (usera, userb, keya, keyb) VALUES ($1, $2, $3, $4)");
    conn->prepare("newLocalFriend", "INSERT INTO localfriendships (usera, userb, keya, keyb) VALUES ($1, $2, $3, $4)");
    //Rooms
    conn->prepare("testExistingRoom","SELECT * FROM rooms WHERE (roomname = $1) OR (owner = $2)");
    conn->prepare("createChatRoom","INSERT INTO rooms (roomname, owner, password, permissions, accesspass) VALUES ($1, $2, $3, $4, $5)");
    conn->prepare("getRoomData", "SELECT * FROM rooms WHERE (roomname = $1)");
    conn->prepare("getRoomByOwner", "SELECT * FROM rooms WHERE owner = $1");
    conn->prepare("removeChatRoom", "DELETE FROM rooms WHERE owner = $1");
    conn->prepare("findClaimedRoom", "SELECT * FROM rooms WHERE roomname = $1 AND password = $2");
    conn->prepare("claimRoom", "UPDATE rooms SET owner = $1 WHERE roomname = $2 AND password = $3");
    //test
    conn->prepare("testdb", "INSERT INTO cyphtest VALUES ($1,$2,$3,$4)");
    conn->prepare("searchtest","SELECT * from cyphtest");
    conn->prepare("deletetest","DELETE from cyphtest");
    return (conn->is_open());
}

InternalUserData DBController::getUserData(std::string user){
    work q (*conn);
    InternalUserData iud;
    iud.ip = "";
    try{
        std::string uname, encryptedData, clearData;
        bf.Encrypt(&uname, user);
        pqxx::binarystring buser (uname);
        result r = q.prepared("searchUser")(buser).exec();
        if(!r.empty()){
            pqxx::binarystring bsip(r[0]["ip"]);
            encryptedData = bsip.str();
            bf.Decrypt(&clearData, encryptedData);
            iud.ip = clearData;
            pqxx::binarystring bsport(r[0]["port"]);
            encryptedData = bsport.str();
            bf.Decrypt(&clearData, encryptedData);
            iud.port = atoi(clearData.c_str());
        }
    }catch(const sql_error err){
        qDebug()<<"Fallo obteniendo datos de bd en verif de invite";
    }
    return iud;
}

int DBController::createUser(rm::RegisteringData rd, std::string code){
    work q(*conn);
    try{        
        std::string encryptedData;
        bf.Encrypt(&encryptedData, rd.uname());;
        pqxx::binarystring uname(encryptedData);
        bf.Encrypt(&encryptedData, rd.email());
        pqxx::binarystring email(encryptedData);
        bf.Encrypt(&encryptedData, rd.password());
        pqxx::binarystring password(encryptedData);
        result r=q.prepared("search")(uname)(email).exec();
        if(r.empty()){
            qDebug()<<"Datos registrados: "+QString::fromStdString(rd.uname());
            time_t now = time(0);     
            q.prepared("newUser")(uname)(email)(password)(code)(ctime(&now)).exec();
            q.commit();
        }else{
            qDebug()<<"El usuario o mail ya existe";
            return 1;
        }
    }catch (const sql_error err) {
        return 2;
    }
    return 0;
}

int DBController::login(rm::Auth auth, QString ip){
    work q(*conn);
    try{
        std::string encryptedData;
        bf.Encrypt(&encryptedData, auth.user());;
        pqxx::binarystring uname(encryptedData);
        bf.Encrypt(&encryptedData, auth.password());
        pqxx::binarystring password(encryptedData);
        bf.Encrypt(&encryptedData, auth.port());
        pqxx::binarystring port(encryptedData);
        bf.Encrypt(&encryptedData, ip.toUtf8().constData());
        pqxx::binarystring cIp(encryptedData);
        result r=q.prepared("loginSearch")(uname)(password).exec();
        if(!r.empty()){
            time_t now = time(0);
            q.prepared("loginUpdate")(cIp)(port)(ctime(&now))(uname).exec();
            q.commit();
            return 0;
        }else{
            qDebug()<<"Incorrect user, password or server";
            return 1;
        }
    }catch (const sql_error err) {
        return 2;
    }
    return 0;
}

rm::PersonalNews DBController::getPersonalNews(rm::Auth auth){
    work q(*conn);
    std::string encryptedData, clear;
    bf.Encrypt(&encryptedData, auth.user());;
    pqxx::binarystring uname(encryptedData);
    result r1 = q.prepared("personalNewsSearch")(uname).exec();
    rm::PersonalNews pns;
    if(!r1.empty()){
        for(result::size_type i = 0; i != r1.size(); ++i){
            rm::PersonalNew *pn = pns.add_personalnew();
            pqxx::binarystring bs(r1[i]["message"]);
            pqxx::binarystring owner(r1[i]["owner"]);
            pqxx::binarystring internalData(r1[i]["internaldataaux"]);
            pqxx::binarystring internalDataAux(r1[i]["internaldata"]);
            bf.Decrypt(&clear, bs.str());
            pn->set_content(clear);
            bf.Decrypt(&clear, internalData.str());
            pn->set_internaldata(clear);
            bf.Decrypt(&clear, internalDataAux.str());
            pn->set_internaldataaux(clear);
            std::string filter;
            filter = r1[i]["type"].as<std::string>();
            pn->set_type(filter);
            bf.Decrypt(&clear, owner.str());
            pn->set_user(clear);
            pn->set_userserver(r1[i]["ownerserver"].as<std::string>());

        }
    }
    return pns;
}

rm::News DBController::getNews(rm::VersionPackage vp){
    work q(*conn);
    result r1 = q.prepared("systemNewsSearch")(vp.system())(vp.so())(::atof(vp.concreteversion().c_str())).exec();
    rm::News ns;
    if(!r1.empty()){
        for(result::size_type i = 0; i != r1.size(); ++i){
            rm::Notice *n = ns.add_notice();
            n->set_content(r1[i]["message"].as<std::string>());
            n->set_type(r1[i]["type"].as<std::string>());
            n->set_relevance(r1[i]["severity"].as<std::string>());
        }
    }
    return ns;
}

rm::UserDataBlock DBController::getFriendList(std::string uname){
    work q(*conn);
    std::string encryptedData, clearData, user, encryptedUser;
    bool esA = true;
    bf.Encrypt(&encryptedUser, uname);;
    pqxx::binarystring cUname(encryptedUser);
    result r1 = q.prepared("localFriendList")(cUname)(cUname).exec();
    result r2;
    rm::UserDataBlock udb;
    if(!r1.empty()){
        for(result::size_type i = 0; i != r1.size(); ++i){
            pqxx::binarystring bskA(r1[i]["keya"]);
            pqxx::binarystring bskB(r1[i]["keyb"]);
            pqxx::binarystring userAux(r1[i]["usera"]);
            pqxx::binarystring userBux(r1[i]["userb"]);
            if(!encryptedUser.compare(userAux.str())==0){
                user = userAux.str();
            }
            else{
                user = userBux.str();
                esA = false;
            }
            pqxx::binarystring friendUser(user);
            r2 = q.prepared("searchUser")(friendUser).exec();
            if(!r2.empty()){
                rm::UserData *ud = udb.add_localfriend();
                if(esA){
                    encryptedData =bskA.str();
                    bf.Decrypt(&clearData, encryptedData);
                    ud->set_key(clearData.substr(0,6));
                }else{
                    encryptedData =bskB.str();
                    bf.Decrypt(&clearData, encryptedData);
                    ud->set_key(clearData.substr(0,6));
                }
                pqxx::binarystring bs(r2[0]["uname"]);
                encryptedData =bs.str();
                bf.Decrypt(&clearData, encryptedData);
                ud->set_user(clearData);
                pqxx::binarystring bsip(r2[0]["ip"]);
                encryptedData = bsip.str();
                bf.Decrypt(&clearData, encryptedData);
                ud->set_ip(clearData);
                pqxx::binarystring bsport(r2[0]["port"]);
                encryptedData =bsport.str();
                bf.Decrypt(&clearData, encryptedData);
                ud->set_port(clearData);
                //time_t date = r2[0]["lastconnection"].as<time_t>();
                //TODO arreglar esto de la fecha
                ud->set_date("");//+date);
            }
        }
    }
    r1 = q.prepared("remoteFriendList")(cUname).exec();
    if(!r1.empty()){
        for(result::size_type i=0; i!= r1.size(); i++){
            rm::ExternalUserData *ud = udb.add_externalfriend();
            pqxx::binarystring ename (r1[i]["remoteuser"]);
            pqxx::binarystring ekey (r1[i]["key"]);
            encryptedData = ename.str();
            bf.Decrypt(&clearData, encryptedData);
            ud->set_user(clearData);
            encryptedData = ekey.str();
            bf.Decrypt(&clearData, encryptedData);
            ud->set_key(clearData.substr(0,6));
            ud->set_server(r1[i]["remoteserver"].as<std::string>());
        }
    }

    return udb;
}

bool DBController::testUser(std::string user){
    work q (*conn);
    try{
        std::string encryptedData;
        bf.Encrypt(&encryptedData, user);;
        pqxx::binarystring uname(encryptedData);
        result r=q.prepared("searchUser")(uname).exec();
        return (!r.empty());
    }catch (const sql_error err) {
        return 2;
    }
}

//********************
// Invitation block
//********************
bool DBController::createPendingInvitation(rm::Invitation invite){
    work q(*conn);
    std::string encryptedData;
    bf.Encrypt(&encryptedData, invite.msg()+" ");
    pqxx::binarystring msg(encryptedData);
    bf.Encrypt(&encryptedData, invite.destinatary());
    pqxx::binarystring destinatary(encryptedData);
    bf.Encrypt(&encryptedData, invite.user());
    pqxx::binarystring owner(encryptedData);
    bf.Encrypt(&encryptedData, invite.key());
    pqxx::binarystring key(encryptedData);
    bf.Encrypt(&encryptedData, "empty");
    pqxx::binarystring aux(encryptedData);
    qDebug()<<"Invitación pendiente guardada de "+QString::fromStdString(invite.user()) + " a "+QString::fromStdString(invite.destinatary())+" con mensaje "+QString::fromStdString(invite.msg());
    result r = q.prepared("searchUnconfirmedInvitation")(destinatary)(owner)("invitation")(invite.server()).exec();
    if(r.empty()){
        LocalService ls;
        if(invite.server().compare(ls.getServerName().toUtf8().constData())==0){
            r = q.prepared("searchLocalfriendship")(destinatary)(owner).exec();
        }else{
            r = q.prepared("searchRemotefriendship")(destinatary)(owner)(invite.server()).exec();
        }
        if(r.empty()){
            bool b= false;
            time_t now = time(0);
            result r1 = q.prepared("addUnconfirmedInvitation")(invite.server())("invitation")(ctime(&now))(msg)(destinatary)(owner)(key)(b)(aux).exec();
            q.commit();
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

bool DBController::createInvitationConfirmation(rm::serverInvitationVerif invite){
    work q(*conn);
    time_t now = time(0);
    std::string encryptedData;
    bf.Encrypt(&encryptedData, invite.userinvited());
    pqxx::binarystring invited(encryptedData);
    bf.Encrypt(&encryptedData, invite.userinviting());
    pqxx::binarystring inviting(encryptedData);
    bf.Encrypt(&encryptedData, invite.key());
    pqxx::binarystring key(encryptedData);
    pqxx::binarystring aux("");
    result r = q.prepared("searchUnconfirmedInvitation")(inviting)(invited)("invitation confirmation")(invite.server()).exec();
    if(r.empty()){
        qDebug()<<"Añadida confirmación de invitación a DB invite de "+QString::fromStdString(invite.userinviting())+" a "+QString::fromStdString(invite.userinvited());
        LocalService ls;
        if(invite.server().compare(ls.getServerName().toUtf8().constData())==0){
            r = q.prepared("searchLocalfriendship")(invited)(inviting).exec();
        }else{
            r = q.prepared("searchRemotefriendship")(inviting)(invited)(invite.server()).exec();
        }
        if(r.empty()){
            result r1 = q.prepared("addUnconfirmedInvitation")(invite.server())("invitation confirmation")(ctime(&now))("")(inviting)(invited)(key)(true)(aux).exec();
            q.commit();
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

bool DBController::confirmSentInvitation(std::string localUser, rm::PersonalNew civ){
    work q(*conn);
    std::string encryptedData;
    bf.Encrypt(&encryptedData, civ.user());
    pqxx::binarystring remoteUser(encryptedData);
    bf.Encrypt(&encryptedData, localUser);
    pqxx::binarystring clocalUser(encryptedData);
    bf.Encrypt(&encryptedData, civ.internaldata());
    pqxx::binarystring ckey(encryptedData);
    result r = q.prepared("searchUnconfirmedInvitation")(clocalUser)(remoteUser)("invitation confirmation")(civ.userserver()).exec();
    if(!r.empty()){
        if(civ.response().compare("true")==0){
            r = q.prepared("confirmInvitationSent")(clocalUser)(remoteUser)("invitation confirmation")(civ.userserver())(ckey).exec();
            q.commit();
            return true;
        }else{
            r = q.prepared("deleteUnconfirmedInvitation")(clocalUser)(remoteUser)("invitation confirmation")(civ.userserver()).exec();
            q.commit();
            return false;
        }
    }else{
        return false;
    }
}

bool DBController::confirmReceivedInvitation(rm::serverInvitationVerif siv){
    work q(*conn);
    std::string encryptedData;
    bf.Encrypt(&encryptedData, siv.userinvited());
    pqxx::binarystring remoteUser(encryptedData);
    bf.Encrypt(&encryptedData, siv.userinviting());
    pqxx::binarystring clocalUser(encryptedData);
    bf.Encrypt(&encryptedData, siv.key());
    pqxx::binarystring ckey(encryptedData);
    result r = q.prepared("searchUnconfirmedInvitation")(remoteUser)(clocalUser)("invitation")(siv.server()).exec();
    if(!r.empty()){
        r = q.prepared("confirmInvitation")(remoteUser)(clocalUser)("invitation")(siv.server())(ckey).exec();
        q.commit();
        return true;
    }else{
        return false;
    }
}

void DBController::getInviteInfo(rm::PersonalNew *invite, std::string localUser){
    work q(*conn);
    std::string encryptedData,clearData;
    bf.Encrypt(&encryptedData, localUser);
    pqxx::binarystring clocalUser(encryptedData);
    bf.Encrypt(&encryptedData, invite->user());
    pqxx::binarystring remoteUser(encryptedData);
    result r = q.prepared("searchUnconfirmedInvitation")(clocalUser)(remoteUser)("invitation")(invite->userserver()).exec();
    if(!r.empty()){
        pqxx::binarystring bsmsg(r[0]["message"]);
        encryptedData =bsmsg.str();
        bf.Decrypt(&clearData, encryptedData);
        invite->set_content(clearData);
        pqxx::binarystring bskey(r[0]["internaldata"]);
        encryptedData =bskey.str();
        bf.Decrypt(&clearData, encryptedData);
        invite->set_internaldataaux(clearData);
    }
}

void DBController::createFriendShip(rm::PersonalNew invite, std::string localUser){
    work q(*conn);
    std::string encryptedData;
    bf.Encrypt(&encryptedData, invite.user());
    pqxx::binarystring remoteUser(encryptedData);
    bf.Encrypt(&encryptedData, localUser);
    pqxx::binarystring clocalUser(encryptedData);
    bf.Encrypt(&encryptedData, invite.internaldata());
    pqxx::binarystring ckey(encryptedData);
    bf.Encrypt(&encryptedData, invite.internaldataaux());
    pqxx::binarystring ckeylocal(encryptedData);
    result r = q.prepared("searchUnconfirmedInvitation")(clocalUser)(remoteUser)("invitation")(invite.userserver()).exec();
    if(!r.empty()){
        LocalService ls;
        if(invite.userserver().compare(ls.getServerName().toUtf8().constData())==0){
            result r1 = q.prepared("newLocalFriend")(clocalUser)(remoteUser)(ckeylocal)(ckey).exec();
        }else{
            result r1 = q.prepared("newExternalFriend")(clocalUser)(remoteUser)(ckey)(invite.userserver()).exec();
        }
        result r2 = q.prepared("deleteUnconfirmedInvitation")(clocalUser)(remoteUser)("invitation")(invite.userserver()).exec();
        q.commit();
    }
}

bool DBController::createFriendShip(rm::serverInvitationVerif invite){
    work q(*conn);
    std::string encryptedData,clearData;
    bf.Encrypt(&encryptedData, invite.userinviting());
    pqxx::binarystring userInviting(encryptedData);
    bf.Encrypt(&encryptedData, invite.userinvited());
    pqxx::binarystring userInvited(encryptedData);
    bf.Encrypt(&encryptedData, invite.key());
    pqxx::binarystring ckey(encryptedData);
    result r = q.prepared("searchUnconfirmedInvitation")(userInviting)(userInvited)("invitation confirmation")(invite.server()).exec();
    if(!r.empty()){
        LocalService ls;
        if(invite.server().compare(ls.getServerName().toUtf8().constData())==0){
            //si es local ya se ha creado la invitación
            //pqxx::binarystring bskey(r[0]["internaldataaux"]);
            //encryptedData =bskey.str();
            //result r1 = q.prepared("newLocalFriend")(clocalUser)(remoteUser)(bskey)(ckey).exec();
        }else{
            result r1 = q.prepared("newExternalFriend")(userInviting)(userInvited)(ckey)(invite.server()).exec();
        }
        result r2 = q.prepared("deleteUnconfirmedInvitation")(userInviting)(userInvited)("invitation confirmation")(invite.server()).exec();
        q.commit();
        return true;
    }else{
        return false;
    }
}

void DBController::denyInvite(rm::serverInvitationVerif invite){
    work q(*conn);
    std::string encryptedData;
    bf.Encrypt(&encryptedData, invite.userinviting());
    pqxx::binarystring inviting(encryptedData);
    bf.Encrypt(&encryptedData, invite.userinvited());
    pqxx::binarystring invited(encryptedData);
    result r = q.prepared("searchUnconfirmedInvitation")(invited)(inviting)("invitation")(invite.server()).exec();
    if(!r.empty()){
        r = q.prepared("deleteUnconfirmedInvitation")(invited)(inviting)("invitation")(invite.server()).exec();
        q.commit();
    }else{
        result r1 = q.prepared("searchUnconfirmedInvitation")(inviting)(invited)("invitation confirmation")(invite.server()).exec();
        if(!r1.empty()){
            r1 = q.prepared("deleteUnconfirmedInvitation")(inviting)(invited)("invitation confirmation")(invite.server()).exec();
            q.commit();
        }
    }
}

rm::UserDataBlock DBController::getExternalFriendList(rm::User userData, std::string *key){
    work q(*conn);
    std::string encryptedData, clearData, user, encryptedUser;
    bf.Encrypt(&encryptedUser, userData.user());
    pqxx::binarystring cUname(encryptedUser);
    result r1 = q.prepared("externalFriendList")(cUname)(userData.server()).exec();
    result r2;
    rm::UserDataBlock udb;
    if(!r1.empty()){
        pqxx::binarystring bskey(r1[0]["key"]);
        encryptedData =bskey.str();
        bf.Decrypt(&clearData, encryptedData);
        *key = clearData;
        for(result::size_type i = 0; i != r1.size(); ++i){
            user = r1[i]["localuser"].as<std::string>();
            r2 = q.prepared("searchUser")(user).exec();
            if(!r2.empty()){
                rm::UserData *ud = udb.add_localfriend();
                pqxx::binarystring bs(r2[0]["uname"]);
                encryptedData =bs.str();
                bf.Decrypt(&clearData, encryptedData);
                ud->set_user(clearData);
                pqxx::binarystring bsip(r2[0]["ip"]);
                encryptedData = bsip.str();
                bf.Decrypt(&clearData, encryptedData);
                ud->set_ip(clearData);
                pqxx::binarystring bsport(r2[0]["port"]);
                encryptedData =bsport.str();
                bf.Decrypt(&clearData, encryptedData);
                ud->set_port(clearData);
                ud->set_date("");
            }
        }
    }
    return udb;
}

//********************
// Room block
//********************

int DBController::roomRegister(rm::roomData rd){
    work q(*conn);
    std::string encrypted;
    bf.Encrypt(&encrypted, rd.room());
    pqxx::binarystring bsroom(encrypted);
    bf.Encrypt(&encrypted, rd.user());
    pqxx::binarystring bsuser(encrypted);
    result r1 = q.prepared("testExistingRoom")(bsroom)(bsuser).exec();
    if(r1.empty()){
        bf.Encrypt(&encrypted, rd.pass());
        pqxx::binarystring bspass(encrypted);
        if(rd.permissions().compare("passprotected")==0){
            bf.Encrypt(&encrypted, rd.accesspass());
        }else {
            encrypted = "";
        }
        pqxx::binarystring bsacesspass(encrypted);
        result r2 =q.prepared("createChatRoom")(bsroom)(bsuser)(bspass)(rd.permissions())(bsacesspass).exec();
        q.commit();
        return 0;
    }else{
        q.abort();
        return 1;
    }
}

rm::roomData DBController::getRoomData(std::string roomName){
    work q(*conn);
    std::string encrypted, decrypted;
    bf.Encrypt(&encrypted, roomName);
    pqxx::binarystring bsroom(encrypted);
    rm::roomData rd;
    result r1 = q.prepared("getRoomData")(bsroom).exec();
    if(!r1.empty()){
        rd.set_room(roomName);
        pqxx::binarystring bsUser(r1[0]["owner"]);
        bf.Decrypt(&decrypted,bsUser.str());
        rd.set_user(decrypted);
        pqxx::binarystring bsPass(r1[0]["password"]);
        bf.Decrypt(&decrypted,bsPass.str());
        rd.set_pass(decrypted);
        //bf.Decrypt(&decrypted,);
        rd.set_permissions(r1[0]["permissions"].as<std::string>());
        if(rd.permissions().compare("passprotected")==0){
            pqxx::binarystring bsAccess(r1[0]["accesspass"]);
            bf.Decrypt(&decrypted,bsAccess.str());
            rd.set_accesspass(decrypted);
        }
    }else{
        rd.set_room("");
    }
    return rd;
}

rm::roomData DBController::getUsersRoom(std::string uname){
    work q(*conn);
    std::string encrypted, decrypted;
    bf.Encrypt(&encrypted, uname);
    pqxx::binarystring bsUname(encrypted);
    rm::roomData rd;
    result r1 = q.prepared("getRoomByOwner")(bsUname).exec();
    if(!r1.empty()){
        rd.set_user(uname);
        pqxx::binarystring bsRoom(r1[0]["roomname"]);
        bf.Decrypt(&decrypted,bsRoom.str());
        rd.set_room(decrypted);
        pqxx::binarystring bsPass(r1[0]["password"]);
        bf.Decrypt(&decrypted,bsPass.str());
        rd.set_pass(decrypted);
        bf.Decrypt(&decrypted,r1[0]["permissions"].as<std::string>());
        rd.set_permissions(decrypted);
        if(rd.permissions().compare("passprotected")==0){
            pqxx::binarystring bsAccess(r1[0]["accesspass"]);
            bf.Decrypt(&decrypted,bsAccess.str());
            rd.set_accesspass(decrypted);
        }
    }else{
        rd.set_room("");
    }
    return rd;
}

void DBController::removeUserRoom(rm::roomData rd){
    work q(*conn);
    std::string encryptedData;
    bf.Encrypt(&encryptedData, rd.user());;
    pqxx::binarystring uname(encryptedData);
    bf.Encrypt(&encryptedData, rd.pass());
    pqxx::binarystring password(encryptedData);;
    result r=q.prepared("loginSearch")(uname)(password).exec();
    if(!r.empty()){
        result r1 = q.prepared("removeChatRoom")(uname).exec();
        q.commit();
    }
}

int DBController::claimRoom(rm::roomData rd){
    work q(*conn);
    std::string encryptedData;
    bf.Encrypt(&encryptedData, rd.pass());
    pqxx::binarystring bsPass(encryptedData);
    bf.Encrypt(&encryptedData, rd.room());
    pqxx::binarystring bsRoom(encryptedData);
    bf.Encrypt(&encryptedData, rd.user());
    pqxx::binarystring bsUname(encryptedData);
    result r = q.prepared("findClaimedRoom")(bsRoom)(bsPass).exec();
    if(!r.empty()){
        result r1 =  q.prepared("claimRoom")(bsUname)(bsRoom)(bsPass).exec();
        q.commit();
        return 0;
    }else{
        return 1;
    }

}

//testing
void DBController::testDb(){
    std::string ta, tb, tc, td,ea,eb,ec,ed;
    work q(*conn);
    ta = ".";
    tb = "user de prueba";
    tc = "oooooooooootro@Ussssssser:de_Prueba";
    td= "aaaaaaaaaaaaaaaaaaaaeeeeeeeeeeeeeeeiiiiiiiiiiioooooooouuuuuuu";
    ea = ta;
    bf.Encrypt(&eb, tb);
    bf.Encrypt(&ec, tc);
    bf.Encrypt(&ed, td);
    pqxx::binarystring cta(ea);
    pqxx::binarystring ctb(eb);
    pqxx::binarystring ctc(ec);

    const void * testMsg = ed.c_str();
    size_t testMsgsize = ed.size();
    pqxx::binarystring ctd(testMsg,testMsgsize);
    result r1 = q.prepared("testdb")(cta)(ctb)(ctc)(ctd).exec();
    q.commit();
    work k(*conn);
    result r2 = k.prepared("searchtest").exec();
    pqxx::binarystring ba(r2[0]["ta"]);
    ea =ba.str();
    bf.Decrypt(&ta, ea);
    qDebug()<<QString::fromStdString(ta);
    pqxx::binarystring bb(r2[0]["tb"]);
    eb =bb.str();
    bf.Decrypt(&tb, eb);
    qDebug()<<QString::fromStdString(tb);
    pqxx::binarystring bc(r2[0]["tc"]);
    ec =bc.str();
    bf.Decrypt(&ta, ec);
    qDebug()<<QString::fromStdString(tc);
    pqxx::binarystring bd(r2[0]["td"]);
    ed =bd.str();
    bf.Decrypt(&td, ed);
    qDebug()<<QString::fromStdString(td);
    result r3 = k.prepared("deletetest").exec();
    k.commit();
}

void DBController::setEncrypter(BlowFish bf){
    this->bf = bf;
}
