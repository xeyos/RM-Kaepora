#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QString>
#include "GUIService.h"

namespace Ui {
class logIn;
}

class logIn : public QDialog
{
    Q_OBJECT

public:
    explicit logIn(QWidget *parent = 0);
    void setService(GUIService *service);
    ~logIn();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::logIn *ui;
    QString pass;
    GUIService *service;
};

#endif // LOGIN_H
