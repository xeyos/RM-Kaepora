#include <QApplication>
#include <iostream>
#include <pqxx/pqxx>
#include "UI/Login.h"
#include "Network/TcpServer.h"

using namespace std;
using namespace pqxx;


int main(int argc, char** argv) {
QApplication a (argc, argv);
logIn lgw;
lgw.show();
/*
connection C("dbname=Kaepora user=Kaepora password=Gaebora  hostaddr=127.0.0.1 port=5432");

string tableName("Kaepora");

if (C.is_open()) {

cout << "We are connected to" << C.dbname() << endl;

} else {

cout << "We are not connected!" << endl;

return 0;

}*/
TcpServer server;
server.startServer();
/**
work Q(C);

try {

Q.exec("DROP TABLE " + tableName);

Q.commit();

} catch (const sql_error &) {

}

work T(C);

T.exec("CREATE TABLE "+tableName+" (id integer NOT NULL, name character varying(32) NOT NULL, salary integer DEFAULT 0);");

tablewriter W(T, tableName);

string load[][3] = {

{"1","John","0"},

{"2","Jane","1"},

{"3","Rosa","2"},

{"4","Danica","3"}

};

for (int i=0;i< 4; ++i)

W.insert(&load[i][0], &load[i][3]);

W.complete();

T.exec("ALTER TABLE ONLY "+tableName+" ADD CONSTRAINT \"PK_IDT\" PRIMARY KEY (id);");

T.commit();

nontransaction N(C);

result R(N.exec("select * from "+tableName));

if (!R.empty()) {

for (result::const_iterator c = R.begin(); c != R.end(); ++c) {

cout << '\t' << c[0].as(string()) << '\t' << c[1].as(string()) <<'\t' << c[2].as(string()) <<endl;

}

}
**/
return a.exec();

}
