#include "aide.h"
#include "ui_aide.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <QDebug>

Aide::Aide(QWidget *parent, Aide* hp):
    QDialog(parent),
    ui(new Ui::Aide)
{
    ui->setupUi(this);
    this->hp = hp;
    this->parent = parent;
}

Aide::~Aide()
{
    delete ui;
}

void Aide::on_buttonBoxhp_accepted()
{
    this->accept();
}
