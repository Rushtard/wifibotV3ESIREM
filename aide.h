#ifndef AIDE_H
#define AIDE_H

#include <QDialog>

namespace Ui {
class Aide;
}

class Aide : public QDialog
{
    Q_OBJECT

public:
    explicit Aide(QWidget *parent = 0);
    ~Aide();

private:
    Ui::Aide *ui;
};

#endif // AIDE_H