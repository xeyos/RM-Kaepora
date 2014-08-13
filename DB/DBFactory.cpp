#include "DBFactory.h"

DBController *DBFactory::db;
DBFactory::DBFactory()
{
}

DBController* DBFactory::getDB(){
    if(db==NULL){
        db = new DBController();
    }
    return db;
}
