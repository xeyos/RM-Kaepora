#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"
#include "Security/BlowFish.h"
#include "MainWindow.h"
#include <QDebug>
#include <fstream>

using namespace std;
ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::setPass(QString p){
    pass = p;
}

void ConfigDialog::on_buttonBox_rejected()
{
    qApp->quit();
}

void ConfigDialog::on_buttonBox_accepted()
{
    QString db, user, password, ip, port;
    try{
        db = ui->txtDb->toPlainText();
        user = ui->txtUser->toPlainText();
        password = ui->txtPass->toPlainText();
        ip = ui->txtIp->toPlainText();
        port = ui->txtPort->toPlainText();
        BlowFish bf;
        string datoCifrado;
        bf.SetKey(pass.toUtf8().constData());
        ofstream oSettings("Settings");
        bf.Encrypt(&datoCifrado, db.toUtf8().constData());
        oSettings<<datoCifrado+"\n";
        bf.Encrypt(&datoCifrado, user.toUtf8().constData());
        oSettings<<datoCifrado+"\n";
        bf.Encrypt(&datoCifrado, password.toUtf8().constData());
        oSettings<<datoCifrado+"\n";
        bf.Encrypt(&datoCifrado, ip.toUtf8().constData());
        oSettings<<datoCifrado+"\n";
        bf.Encrypt(&datoCifrado, port.toUtf8().constData());
        oSettings<<datoCifrado+"\n";
        oSettings.close();
        MainWindow *mw = new MainWindow(this);
        mw->setPassword(pass);
        mw->show();

    }catch(int e){
        qDebug()<<"Se ha producido un error cargando los datos del formulario";
    }
}
