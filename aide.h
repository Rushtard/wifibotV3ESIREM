#ifndef AIDE_H
#define AIDE_H

#include <QDialog>
#include "aide.h"

namespace Ui {
class Aide;
}

class Aide : public QDialog
{
    Q_OBJECT

public:
    explicit Aide(QWidget *parent, Aide* hp);
    ~Aide();

private slots:
    void on_buttonBoxhp_accepted();

private:
    Ui::Aide *ui;
    Aide* hp;
    QWidget* parent;
};

#endif // AIDE_H
