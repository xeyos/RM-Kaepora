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
    conn->prepare("newExternalFriend", "INSERT INTO friendships (localuser, remoteuser, key, remoteserver) VALUES ($1, $2, $3, $4)");
    conn->prepare("newLocalFriend", "INSERT INTO localfriendships (usera, userb, keya, keyb) VALUES ($1, $2, $3, $4)");
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
        //bf.Encrypt(&uname, user);
        //pqxx::binarystring buser (uname);
        result r = q.prepared("searchUser")(user).exec();
        if(!r.empty()){
            //pqxx::binarystring bsip(r[0]["ip"]);
            //encryptedData = bsip.get();
            //bf.Decrypt(&clearData, encryptedData);
            iud.ip = r[0]["ip"].as<std::string>();
            //pqxx::binarystring bsport(r[0]["port"]);
            //encryptedData = bsport.get();
            //bf.Decrypt(&clearData, encryptedData);
            iud.port = atoi(r[0]["port"].as<std::string>().c_str());
        }
    }catch(const sql_error err){
        qDebug()<<"Fallo obteniendo datos de bd en verif de invite";
    }
    return iud;
}

int DBController::createUser(rm::RegisteringData rd, std::string code){
    work q(*conn);
    try{        
        //std::string encryptedData;
        //bf.Encrypt(&encryptedData, rd.uname());;
        //pqxx::binarystring uname(encryptedData);
        //bf.Encrypt(&encryptedData, rd.email());
        //pqxx::binarystring email(encryptedData);
        //bf.Encrypt(&encryptedData, rd.password());
        //pqxx::binarystring password(encryptedData);
        result r=q.prepared("search")(rd.uname())(rd.email()).exec();
        if(r.empty()){
            qDebug()<<"Datos registrados: "+QString::fromStdString(rd.uname());
            time_t now = time(0);     
            q.prepared("newUser")(rd.uname())(rd.email())(rd.password())(code)(ctime(&now)).exec();
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
        //std::string encryptedData;
        //bf.Encrypt(&encryptedData, auth.user());;
        //pqxx::binarystring uname(encryptedData);
        //bf.Encrypt(&encryptedData, auth.password());
        //pqxx::binarystring password(encryptedData);
        //bf.Encrypt(&encryptedData, auth.port());
        //pqxx::binarystring port(encryptedData);
        //bf.Encrypt(&encryptedData, ip.toUtf8().constData());
        //pqxx::binarystring cIp(encryptedData);
        result r=q.prepared("loginSearch")(auth.user())(auth.password()).exec();
        if(!r.empty()){
            time_t now = time(0);
            q.prepared("loginUpdate")(ip.toUtf8().constData())(auth.port())(ctime(&now))(auth.user()).exec();
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
    //bf.Encrypt(&encryptedData, auth.user());;
   // pqxx::binarystring uname(encryptedData);
    result r1 = q.prepared("personalNewsSearch")(auth.user()).exec();
    rm::PersonalNews pns;
    if(!r1.empty()){
        for(result::size_type i = 0; i != r1.size(); ++i){
            rm::PersonalNew *pn = pns.add_personalnew();
            //pqxx::binarystring bs(r1[i]["message"]);
            //pqxx::binarystring owner(r1[i]["owner"]);
            pqxx::binarystring internalData(r1[i]["internaldata"]);
            pqxx::binarystring internalDataAux(r1[i]["internaldataaux"]);
            //bf.Decrypt(&clear, bs.get());
            pn->set_content(r1[i]["message"].as<std::string>());
            //bf.Decrypt(&clear, internalData.get());
            pn->set_internaldata(internalData.get());
            //bf.Decrypt(&clear, internalDataAux.get());
            pn->set_internaldataaux(internalDataAux.get());
            std::string filter;
            filter = r1[i]["type"].as<std::string>();
            pn->set_type(filter);
            //bf.Decrypt(&clear, owner.get());
            pn->set_user(r1[i]["owner"].as<std::string>());
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
    //bf.Encrypt(&encryptedUser, uname);;
    //pqxx::binarystring cUname(encryptedUser);
    result r1 = q.prepared("localFriendList")(uname)(uname).exec();
    result r2;
    rm::UserDataBlock udb;
    if(!r1.empty()){
        for(result::size_type i = 0; i != r1.size(); ++i){
            pqxx::binarystring bskA(r1[i]["keya"]);
            pqxx::binarystring bskB(r1[i]["keyb"]);
            if(uname.compare(r1[i]["usera"].as<std::string>())){
                user = r1[i]["usera"].as<std::string>();
            }
            else{
                user = r1[i]["userb"].as<std::string>();
                esA = false;
            }

            r2 = q.prepared("searchUser")(user).exec();
            if(!r2.empty()){
                rm::UserData *ud = udb.add_localfriend();
                if(esA){
                    //encryptedData =bskA.get();
                    //bf.Decrypt(&clearData, encryptedData);
                    std::string cleaner = bskA.str();
                    ud->set_key(cleaner.substr(0,6));
                }else{
                    //encryptedData =bskB.get();
                    //bf.Decrypt(&clearData, encryptedData);
                    std::string cleaner = bskB.str();
                    ud->set_key(cleaner.substr(0,6));
                }
                //pqxx::binarystring bs(r2[0]["uname"]);
                //encryptedData =bs.get();
                //bf.Decrypt(&clearData, encryptedData);
                ud->set_user(r2[0]["uname"].as<std::string>());
                //pqxx::binarystring bsip(r2[0]["ip"]);
                //encryptedData = bsip.get();
                //bf.Decrypt(&clearData, encryptedData);
                ud->set_ip(r2[0]["ip"].as<std::string>());
               // pqxx::binarystring bsport(r2[0]["port"]);
               // encryptedData =bsport.get();
               // bf.Decrypt(&clearData, encryptedData);
                ud->set_port(r2[0]["port"].as<std::string>());
                //time_t date = r2[0]["lastconnection"].as<time_t>();
                //TODO arreglar esto de la fecha
                ud->set_date("");//+date);
            }
        }
    }
    r1 = q.prepared("remoteFriendList")(uname).exec();
    if(!r1.empty()){
        for(result::size_type i=0; i!= r1.size(); i++){
            rm::ExternalUserData *ud = udb.add_externalfriend();
            //pqxx::binarystring ename (r1[i]["remoteuser"]);
            pqxx::binarystring ekey (r1[i]["key"]);
            //encryptedData = ename.get();
            //bf.Decrypt(&clearData, encryptedData);
            ud->set_user(r1[i]["remoteuser"].as<std::string>());
            //encryptedData = ekey.get();
            //bf.Decrypt(&clearData, encryptedData);
            std::string cleaner = ekey.str();
            ud->set_key(cleaner.substr(0,6));
            ud->set_server(r1[i]["remoteserver"].as<std::string>());
        }
    }

    return udb;
}

bool DBController::testUser(std::string user){
    work q (*conn);
    try{
        //std::string encryptedData;
        //bf.Encrypt(&encryptedData, user);;
        //pqxx::binarystring uname(encryptedData);
        result r=q.prepared("searchUser")(user).exec();
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
    //std::string encryptedData;
    //bf.Encrypt(&encryptedData, invite.msg());
    //const void * testMsg = encryptedData.c_str();
    //size_t testMsgsize = encryptedData.size();
    //pqxx::binarystring msg(testMsg,testMsgsize);
    //bf.Encrypt(&encryptedData, invite.destinatary());
    //pqxx::binarystring destinatary(encryptedData);
    //bf.Encrypt(&encryptedData, invite.user());
    //pqxx::binarystring owner(encryptedData);
    //bf.Encrypt(&encryptedData, invite.key());
    pqxx::binarystring key(invite.key());
    //bf.Encrypt(&encryptedData, "empty");
    pqxx::binarystring aux("");
    qDebug()<<"Invitación pendiente guardada de "+QString::fromStdString(invite.user()) + " a "+QString::fromStdString(invite.destinatary())+" con mensaje "+QString::fromStdString(invite.msg());
    result r = q.prepared("searchUnconfirmedInvitation")(invite.destinatary())(invite.user())("invitation")(invite.server()).exec();
    if(r.empty()){
        LocalService ls;
        if(invite.server().compare(ls.getServerName().toUtf8().constData())==0){
            r = q.prepared("searchLocalfriendship")(invite.destinatary())(invite.user()).exec();
        }else{
            r = q.prepared("searchRemotefriendship")(invite.destinatary())(invite.user())(invite.server()).exec();
        }
        if(r.empty()){
            bool b= false;
            time_t now = time(0);
            result r1 = q.prepared("addUnconfirmedInvitation")(invite.server())("invitation")(ctime(&now))(invite.msg())(invite.destinatary())(invite.user())(key)(b)(aux).exec();
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
   // std::string encryptedData;
    //bf.Encrypt(&encryptedData, invite.userinvited());
    //pqxx::binarystring invited(encryptedData);
    //bf.Encrypt(&encryptedData, invite.userinviting());
    //pqxx::binarystring inviting(encryptedData);
    //bf.Encrypt(&encryptedData, invite.key());
    pqxx::binarystring key(invite.key());
    //pqxx::binarystring aux("");
    result r = q.prepared("searchUnconfirmedInvitation")(invite.userinvited())(invite.userinviting())("invitation confirmation")(invite.server()).exec();
    if(r.empty()){        
        LocalService ls;
        if(invite.server().compare(ls.getServerName().toUtf8().constData())==0){
            r = q.prepared("searchLocalfriendship")(invite.userinvited())(invite.userinviting()).exec();
        }else{
            r = q.prepared("searchRemotefriendship")(invite.userinviting())(invite.userinvited())(invite.server()).exec();
        }
        if(r.empty()){
            result r1 = q.prepared("addUnconfirmedInvitation")(invite.server())("invitation confirmation")(ctime(&now))("")(invite.userinvited())(invite.userinviting())(key)(true)("").exec();
            q.commit();
            qDebug()<<"Añadida confirmación de invitación a DB invite de "+QString::fromStdString(invite.userinviting())+" a "+QString::fromStdString(invite.userinvited());
            return true;
        }else{
            qDebug()<<"Ya hay una invitación pendiente";
            return false;
        }
    }else{
        return false;
    }
}

bool DBController::confirmSentInvitation(std::string localUser, rm::PersonalNew civ){
    work q(*conn);
  //  std::string encryptedData;
  //  bf.Encrypt(&encryptedData, civ.user());
  //  pqxx::binarystring remoteUser(encryptedData);
  //  bf.Encrypt(&encryptedData, localUser);
  //  pqxx::binarystring clocalUser(encryptedData);
  //  bf.Encrypt(&encryptedData, civ.internaldata());
    pqxx::binarystring ckey(civ.internaldata());
    result r = q.prepared("searchUnconfirmedInvitation")(civ.user())(localUser)("invitation confirmation")(civ.userserver()).exec();
    if(!r.empty()){
        if(civ.response().compare("true")==0){
            r = q.prepared("confirmInvitationSent")(civ.user())(localUser)("invitation confirmation")(civ.userserver())(ckey).exec();
            q.commit();
            return true;
        }else{
            r = q.prepared("deleteUnconfirmedInvitation")(civ.user())(localUser)("invitation confirmation")(civ.userserver()).exec();
            q.commit();
            return false;
        }
    }else{
        return false;
    }
}

bool DBController::confirmReceivedInvitation(rm::serverInvitationVerif siv){
    work q(*conn);
//    std::string encryptedData;
//    bf.Encrypt(&encryptedData, siv.userinvited());
//    pqxx::binarystring remoteUser(encryptedData);
//    bf.Encrypt(&encryptedData, siv.userinviting());
//    pqxx::binarystring clocalUser(encryptedData);
//    bf.Encrypt(&encryptedData, siv.key());
    pqxx::binarystring ckey(siv.key());
    result r = q.prepared("searchUnconfirmedInvitation")(siv.userinvited())(siv.userinviting())("invitation")(siv.server()).exec();
    if(!r.empty()){
        qDebug()<<"Confirmado en db";
        r = q.prepared("confirmInvitation")(siv.userinvited())(siv.userinviting())("invitation")(siv.server())(ckey).exec();
        q.commit();
        return true;
    }else{
        return false;
    }
}

void DBController::getInviteInfo(rm::PersonalNew *invite, std::string localUser){
    work q(*conn);
    //std::string encryptedData,clearData;
    //bf.Encrypt(&encryptedData, localUser);
    //pqxx::binarystring clocalUser(encryptedData);
    //bf.Encrypt(&encryptedData, invite->user());
    //pqxx::binarystring remoteUser(encryptedData);
    result r = q.prepared("searchUnconfirmedInvitation")(localUser)(invite->user())("invitation")(invite->userserver()).exec();
    if(!r.empty()){
        //pqxx::binarystring bsmsg(r[0]["message"]);
        //encryptedData =bsmsg.get();
        //bf.Decrypt(&clearData, encryptedData);
        invite->set_content(r[0]["message"].as<std::string>());
        pqxx::binarystring bskey(r[0]["internaldata"]);
        //encryptedData =bskey.get();
        //bf.Decrypt(&clearData, encryptedData);
        std::string cleaner =bskey.str();
        invite->set_internaldataaux(cleaner.substr(0,3));
    }
}

void DBController::createFriendShip(rm::PersonalNew invite, std::string localUser){
    work q(*conn);
    //std::string encryptedData;
    //bf.Encrypt(&encryptedData, invite.user());
    //pqxx::binarystring remoteUser(encryptedData);
    //bf.Encrypt(&encryptedData, localUser);
    //pqxx::binarystring clocalUser(encryptedData);
    //bf.Encrypt(&encryptedData, invite.internaldata());
    pqxx::binarystring ckey(invite.internaldata());
    //bf.Encrypt(&encryptedData, invite.internaldataaux());
    pqxx::binarystring ckeylocal(invite.internaldataaux());
    result r = q.prepared("searchUnconfirmedInvitation")(localUser)(invite.user())("invitation")(invite.userserver()).exec();
    if(!r.empty()){
        LocalService ls;
        if(invite.userserver().compare(ls.getServerName().toUtf8().constData())==0){
            result r1 = q.prepared("newLocalFriend")(localUser)(invite.user())(ckeylocal)(ckey).exec();
        }else{
            r = q.prepared("deleteUnconfirmedInvitation")(localUser)(invite.user())("invitation")(invite.userserver()).exec();
            result r1 = q.prepared("newExternalFriend")(localUser)(invite.user())(ckey)(invite.userserver()).exec();
        }
        result r2 = q.prepared("deleteUnconfirmedInvitation")(localUser)(invite.user())("invitation")(invite.userserver()).exec();
        q.commit();
    }
}

bool DBController::createFriendShip(rm::serverInvitationVerif invite){
    work q(*conn);
    //std::string encryptedData,clearData;
    //bf.Encrypt(&encryptedData, invite.userinviting());
    //pqxx::binarystring clocalUser(encryptedData);
    //bf.Encrypt(&encryptedData, invite.userinvited());
    //pqxx::binarystring remoteUser(encryptedData);
    //bf.Encrypt(&encryptedData, invite.key());
    pqxx::binarystring ckey(invite.key());
    result r = q.prepared("searchUnconfirmedInvitation")(invite.userinvited())(invite.userinviting())("invitation confirmation")(invite.server()).exec();
    if(!r.empty()){
        LocalService ls;
        if(invite.server().compare(ls.getServerName().toUtf8().constData())==0){
            //si es local ya se ha creado la invitación
            //pqxx::binarystring bskey(r[0]["internaldataaux"]);
            //encryptedData =bskey.get();
            //result r1 = q.prepared("newLocalFriend")(clocalUser)(remoteUser)(bskey)(ckey).exec();
        }else{
            result r1 = q.prepared("newExternalFriend")(invite.userinviting())(invite.userinvited())(ckey)(invite.server()).exec();
        }
        result r2 = q.prepared("deleteUnconfirmedInvitation")(invite.userinvited())(invite.userinviting())("invitation confirmation")(invite.server()).exec();
        q.commit();
        return true;
    }else{
        return false;
    }
}

void DBController::denyInvite(rm::serverInvitationVerif invite){
    work q(*conn);
    //std::string encryptedData;
    //bf.Encrypt(&encryptedData, invite.userinviting());
    //pqxx::binarystring inviting(encryptedData);
    //bf.Encrypt(&encryptedData, invite.userinvited());
    //pqxx::binarystring invited(encryptedData);
    result r = q.prepared("searchUnconfirmedInvitation")(invite.userinvited())(invite.userinviting())("invitation")(invite.server()).exec();
    if(!r.empty()){
        r = q.prepared("deleteUnconfirmedInvitation")(invite.userinvited())(invite.userinviting())("invitation")(invite.server()).exec();
        q.commit();
    }else{
        result r1 = q.prepared("searchUnconfirmedInvitation")(invite.userinvited())(invite.userinviting())("invitation confirmation")(invite.server()).exec();
        if(!r1.empty()){
            r1 = q.prepared("deleteUnconfirmedInvitation")(invite.userinvited())(invite.userinviting())("invitation confirmation")(invite.server()).exec();
            q.commit();
        }
    }
}

rm::UserDataBlock DBController::getExternalFriendList(rm::User userData, std::string *key){
    work q(*conn);
    std::string encryptedData, clearData, user, encryptedUser;
//    bf.Encrypt(&encryptedUser, userData.user());
//    pqxx::binarystring cUname(encryptedUser);
    result r1 = q.prepared("externalFriendList")(userData.user())(userData.server()).exec();
    result r2;
    rm::UserDataBlock udb;
    if(!r1.empty()){
        pqxx::binarystring bskey(r1[0]["key"]);
        //encryptedData =bskey.get();
        //bf.Decrypt(&clearData, encryptedData);
        *key = bskey.get();
        for(result::size_type i = 0; i != r1.size(); ++i){
            user = r1[i]["localuser"].as<std::string>();
            r2 = q.prepared("searchUser")(user).exec();
            if(!r2.empty()){
                rm::UserData *ud = udb.add_localfriend();
                //pqxx::binarystring bs(r2[i]["uname"]);
                //encryptedData =bs.get();
                //bf.Decrypt(&clearData, encryptedData);
                ud->set_user(r2[i]["uname"].as<std::string>());
                //pqxx::binarystring bsip(r2[i]["ip"]);
                //encryptedData = bsip.get();
                //bf.Decrypt(&clearData, encryptedData);
                ud->set_ip(r2[i]["ip"].as<std::string>());
                //pqxx::binarystring bsport(r2[i]["port"]);
                //encryptedData =bsport.get();
                //bf.Decrypt(&clearData, encryptedData);
                ud->set_port(r2[i]["port"].as<std::string>());
                ud->set_date("");
            }
        }
    }
    return udb;
}

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
    eb =bb.get();
    bf.Decrypt(&tb, eb);
    qDebug()<<QString::fromStdString(tb);
    pqxx::binarystring bc(r2[0]["tc"]);
    ec =bc.get();
    bf.Decrypt(&ta, ec);
    qDebug()<<QString::fromStdString(tc);
    pqxx::binarystring bd(r2[0]["td"]);
    ed =bd.get();
    bf.Decrypt(&td, ed);
    qDebug()<<QString::fromStdString(td);
    result r3 = k.prepared("deletetest").exec();
    k.commit();
}

void DBController::setEncrypter(BlowFish bf){
    this->bf = bf;
}
