#ifndef BASEDBCONTROLLER_H
#define BASEDBCONTROLLER_H
#include <string>
#include <QString>
#include "RedMalk.pb.h"
#include "Security/BlowFish.h"
#include "InternalUserData.h"

class BaseDBController{
public:
    virtual bool setAccessData(std::string dbname, std::string user, std::string password, std::string host, std::string port)=0;
    virtual int createUser(rm::RegisteringData rd, std::string code)=0;
    virtual void setEncrypter(BlowFish bf)=0;
    virtual int login(rm::Auth auth, QString ip)=0;
    virtual rm::PersonalNews getPersonalNews(rm::Auth auth) = 0;
    virtual rm::News getNews(rm::VersionPackage vp) =0;
    virtual rm::UserDataBlock getFriendList(std::string uname) = 0;
    virtual bool createPendingInvitation(rm::Invitation invite) = 0;
    virtual bool createInvitationConfirmation(rm::serverInvitationVerif invite) = 0;
    virtual bool testUser(std::string user) =0;
    virtual InternalUserData getUserData(std::string user) = 0;
    virtual bool confirmSentInvitation(std::string localUser, rm::PersonalNew civ) = 0;
    virtual bool confirmReceivedInvitation(rm::serverInvitationVerif siv)=0;
    virtual void getInviteInfo(rm::PersonalNew *invite, std::string localUser)=0;
    virtual void createFriendShip(rm::PersonalNew invite, std::string localUser) =0;
    virtual bool createFriendShip(rm::serverInvitationVerif invite) =0;
    virtual void denyInvite(rm::serverInvitationVerif invite) =0;
    virtual rm::UserDataBlock getExternalFriendList(rm::User userData, std::string *key) = 0;
    //chatRooms
    virtual int roomRegister(rm::roomData rd) = 0;
    virtual rm::roomData getRoomData(std::string roomName) = 0;
    virtual rm::roomData getUsersRoom(std::string uname) = 0;
    virtual void removeUserRoom(rm::roomData) = 0;
    virtual int claimRoom(rm::roomData rd) = 0;
    //testing purposes
    virtual void testDb() = 0;
};

#endif // BASEDBCONTROLLER_H
