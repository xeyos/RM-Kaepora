#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "GUIService.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setPassword(QString p);
    void setService(GUIService *service);

private slots:
    void on_checkBox_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QString pass;
    GUIService *service;
};

#endif // MAINWINDOW_H
