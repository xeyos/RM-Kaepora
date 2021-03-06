#include "Login.h"
#include "ui_Login.h"
#include "ConfigDialog.h"
#include "MainWindow.h"
#include <fstream>

using namespace std;
logIn::logIn(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::logIn)
{
    ui->setupUi(this);
}

logIn::~logIn()
{
    delete ui;
}

void logIn::on_buttonBox_accepted()
{
    pass = ui->txtPass->text();
    ifstream iSettings("Settings");
    if(iSettings){
        MainWindow *mw = new MainWindow(this);
        mw->setService(service);
        mw->setPassword(pass);
        mw->show();

    }else{
        iSettings.close();
        ConfigDialog *cfg = new ConfigDialog(this);
        cfg->setPass(pass);
        cfg->setService(service);
        cfg->show();
    }
}

void logIn::setService(GUIService *service){
    this->service = service;
}

void logIn::on_buttonBox_rejected()
{
    qApp->quit();
}
