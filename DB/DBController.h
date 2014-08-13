#ifndef DBCONTROLLER_H
#define DBCONTROLLER_H
#include <pqxx/pqxx>
#include "BaseDBController.h"

class DBController : public BaseDBController
{
public:
    DBController();
    bool setAccessData(std::string dbname, std::string user, std::string password, std::string host, std::string port);
    int createUser(rm::RegisteringData rd, std::string code);
    void setEncrypter(BlowFish bf);
    int login(rm::Auth auth, QString ip);
    rm::PersonalNews getPersonalNews(rm::Auth auth);
    rm::News getNews(rm::VersionPackage vp);
    rm::UserDataBlock getFriendList(std::string uname);
    bool createPendingInvitation(rm::Invitation invite);
    bool createInvitationConfirmation(rm::serverInvitationVerif invite);
    bool testUser(std::string user);
    InternalUserData getUserData(std::string user);
    bool confirmSentInvitation(std::string localUser, rm::PersonalNew civ);
    bool confirmReceivedInvitation(rm::serverInvitationVerif siv);
    void getInviteInfo(rm::PersonalNew *invite, std::string localUser);
    void createFriendShip(rm::PersonalNew invite, std::string localUser);
    bool createFriendShip(rm::serverInvitationVerif invite);
    void denyInvite(rm::serverInvitationVerif invite);
    rm::UserDataBlock getExternalFriendList(rm::User userData, std::string *key);

   void testDb();
private:
    pqxx::connection *conn;
    BlowFish bf;
};

#endif // DBCONTROLLER_H
