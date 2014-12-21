#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Security/BlowFish.h"
#include "LocalService.h"
#include <fstream>
#include <QDebug>

#include "DB/DBFactory.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::setPassword(QString p){
    pass = p;
    service->setPassword(p);
    ifstream iSettings("Settings");
    QPalette palette, paletteServer;
    string datoCifrado;
    QString db, user, password,ip, port, adress;
    palette.setColor(QPalette::WindowText, Qt::red);
    ui->lblDbStatus->setPalette(palette);
    BlowFish bf;
    bf.SetKey(pass.toUtf8().constData());
    if(iSettings){
        try{
            std::string acum="";
            int count = 0, prevCount = 0;
            while (getline(iSettings, datoCifrado)) {
                if(datoCifrado.substr(datoCifrado.length()-6,6).compare("/@data")==0){
                    count++;
                    datoCifrado = datoCifrado.substr(0,datoCifrado.length()-6);
                }
                if(acum.compare("")!=0){
                    acum +="\n" + datoCifrado;
                }else{
                    acum =datoCifrado;
                }
                if(count!=prevCount){
                    datoCifrado= acum;
                    bf.Decrypt(&acum, datoCifrado);
                    prevCount =count;
                    switch (count) {
                    case 1:{
                        db= QString::fromStdString(acum);
                        qDebug()<<db;
                        break;
                    }
                    case 2:{
                        user= QString::fromStdString(acum);
                        qDebug()<<user;
                        break;
                    }
                    case 3:{
                        password= QString::fromStdString(acum);
                        qDebug()<<password;
                        break;
                    }
                    case 4:{
                        ip= QString::fromStdString(acum);
                        qDebug()<<ip;
                        break;
                    }
                    case 5:{
                        port= QString::fromStdString(acum);
                        qDebug()<<port;
                        break;
                    }
                    case 6:{
                        adress= QString::fromStdString(acum);
                        qDebug()<<adress;
                        break;
                    }
                    }
                    acum = "";
                }
            }
            DBFactory dbfactory;
            if (dbfactory.getDB()->setAccessData(db.toUtf8().constData(), user.toUtf8().constData(), password.toUtf8().constData(), ip.toUtf8().constData(), port.toUtf8().constData())) {
                palette.setColor(QPalette::WindowText, Qt::green);
                ui->lblDbStatus->setPalette(palette);
                ui->lblDbStatus->setText("Conexión correcta");
                if(service->startServer()){
                    paletteServer.setColor(QPalette::WindowText, Qt::green);
                    ui->lblServerStatus->setPalette(paletteServer);
                    ui->lblServerStatus->setText(tr("Server iniciado correctamente"));
                }else{
                    paletteServer.setColor(QPalette::WindowText, Qt::red);
                    ui->lblServerStatus->setPalette(paletteServer);
                    ui->lblServerStatus->setText(tr("Problema iniciando server"));
                }
                dbfactory.getDB()->setEncrypter(bf);
            } else {ui->lblDbStatus->setPalette(palette);
                ui->lblDbStatus->setText("Error de conexión");
            }
        }catch(std::exception e){
                ui->lblDbStatus->setText("Error de conexión");
        }
    }else{
        ui->lblDbStatus->setText("Error de conexión");
    }
    iSettings.close();
}

void MainWindow::setService(GUIService *service){
    this->service = service;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_checkBox_clicked()
{
    if(ui->checkBox->isChecked()){
        service->setServerMode(1);
    }else{
        service->setServerMode(0);
    }
}

void MainWindow::on_pushButton_clicked()
{
    DBFactory dbf;
    DBController* dbc = dbf.getDB();
    dbc->testDb();

}
