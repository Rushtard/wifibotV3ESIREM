#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QtNetwork>
#include <QUrl>

#define cameraup "/?action=command&dest=0&plugin=0&id=10094853&group=1&value=-200"
#define cameradown "/?action=command&dest=0&plugin=0&id=10094853&group=1&value=200"
#define cameraleft "/?action=command&dest=0&plugin=0&id=10094852&group=1&value=200"
#define cameraright "/?action=command&dest=0&plugin=0&id=10094852&group=1&value=-200"
#define camerareset "/?action=command&dest=0&plugin=0&id=168062211&group=1&value=3"

class Communication : public QObject
{
    Q_OBJECT
public:
    Communication(QObject *parent = 0);
    QByteArray buf;
    QString adresse;
    QString port;
    QNetworkAccessManager *camera;
    bool connecte;
    bool webcam;
    bool foreward;
    bool backward;
    bool left;
    bool right;
    int cpt_ir1;
    int cpt_ir2;
    QTcpSocket tcp;
    QTimer *timer;
    bool Connexion(QString address, quint16 port);
    void Deconnexion();
    void GenMessage();
    void sendMessage();
    void recvMessage();
    quint16 vitesse;
    quint16 battery;
    quint16 crc16(QByteArray byteArray, int pos);

public slots:
         void tick();
signals:

};

#endif // COMMUNICATION_H
