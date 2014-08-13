#ifndef DBFACTORY_H
#define DBFACTORY_H

#include "DBController.h"
class DBFactory
{
public:
    DBFactory();
    static DBController* getDB();

private:
    static DBController *db;
};

#endif // DBFACTORY_H
