#include "connexion.h"
#include "mainwindow.h"
#include "ui_connexion.h"
#include "ui_mainwindow.h"
#include <QDebug>

Connexion::Connexion(QWidget *parent, Communication *co) :
    QDialog(parent),
    ui(new Ui::Connexion)
{
    ui->setupUi(this);
    this->co = co;
    this->parent = parent;
}

Connexion::~Connexion()
{
    delete ui;
}

void Connexion::on_buttonBox_accepted()
{
    QString address = ui->lineEdit_address->text();
    quint16 port = quint16(ui->lineEdit_port->text().toInt());
    Ui::MainWindow* ui = ((MainWindow*) parentWidget())->getUI();

    ui->statusBar->showMessage(tr("Connecting..."));

    if(co->Connexion(address, port))
    {
        ui->statusBar->showMessage(tr("Connected:") + address + tr(" ") + QString::number(port));
        this->accept();
    }
    else
    {
        ui->statusBar->showMessage(tr("/!\\ ") + co->tcp.errorString());
        this->reject();
    }
}
