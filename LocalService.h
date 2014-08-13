#ifndef LOCALSERVICE_H
#define LOCALSERVICE_H
#include <QString>
class LocalService
{
public:
    LocalService();
    void setPassword(QString p);
    QString getServerName();

private:
    static QString pass;
    static QString server;
};

#endif // LOCALSERVICE_H
