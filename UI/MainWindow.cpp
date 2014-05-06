#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Security/BlowFish.h"
#include <fstream>
#include <QDebug>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::setPassword(QString p){
    pass = p;
    ifstream iSettings("Settings");
    QPalette palette;
    string datoCifrado, datoDescifrado;
    QString db, user, password,ip, port;
    palette.setColor(QPalette::WindowText, Qt::red);
    ui->lblDbStatus->setPalette(palette);
    BlowFish bf;
    bf.SetKey(pass.toUtf8().constData());
    if(iSettings){
        try{
            int count = 1;
            while(count<6){
                switch(count){
                case 1:{
                    getline(iSettings,datoCifrado);
                    bf.Decrypt(&datoDescifrado, datoCifrado);
                    db= QString::fromStdString(datoDescifrado);
                    break;
                }
                case 2:{
                    getline(iSettings,datoCifrado);
                    bf.Decrypt(&datoDescifrado, datoCifrado);
                    user= QString::fromStdString(datoDescifrado);
                    break;
                }
                case 3:{
                    getline(iSettings,datoCifrado);
                    bf.Decrypt(&datoDescifrado, datoCifrado);
                    password= QString::fromStdString(datoDescifrado);
                    break;
                }
                case 4:{
                    getline(iSettings,datoCifrado);
                    bf.Decrypt(&datoDescifrado, datoCifrado);
                    ip= QString::fromStdString(datoDescifrado);
                    break;
                }
                case 5:{
                    getline(iSettings,datoCifrado);
                    bf.Decrypt(&datoDescifrado, datoCifrado);
                    port= QString::fromStdString(datoDescifrado);
                    break;
                }
                }
                count++;
            }
            connection C(string()+"dbname="+db.toUtf8().constData()+" user="+user.toUtf8().constData()+" password="+password.toUtf8().constData()+"  hostaddr="+ip.toUtf8().constData()+" port="+port.toUtf8().constData());
            if (C.is_open()) {
                palette.setColor(QPalette::WindowText, Qt::green);
                ui->lblDbStatus->setPalette(palette);
                ui->lblDbStatus->setText("Conexión correcta");
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

MainWindow::~MainWindow()
{
    delete ui;
}
