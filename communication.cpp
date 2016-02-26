#include "communication.h"

Communication::Communication(QObject *parent) :
    QObject(parent)
{
    this->connecte = false;
    this->webcam = false;
    this->foreward = false;
    this->backward = false;
    this->left = false;
    this->right = false;
    this->battery = 0;
    this->webcam = false;
    camera = new QNetworkAccessManager(this);
    timer = new QTimer();
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
}

bool Communication::Connexion(QString address, quint16 port){
    qDebug() << "Connexion à " << address << ":" << port;
    tcp.connectToHost(address, port);
    if(tcp.waitForConnected(5000)){
        qDebug() << "Connecté";
        this->connecte = true;
        timer->start();
        this->adresse = address;
        this->port = port;
        return true;
    }
    else{
        this->connecte = false;
        return false;
    }
}

void Communication::tick(){
    GenMessage();
    sendMessage();
    recvMessage();
}

void Communication::Deconnexion(){
    tcp.disconnectFromHost();
    this->connecte = false;
    timer->stop();
    this->cpt_ir1 = 0;
    this->cpt_ir2 = 0;
    this->battery = 0;
}

void Communication::GenMessage(){
    if(webcam){
        if(foreward){
            QUrl url("http://"+ this->adresse +":8080"cameraup);
            camera->get(QNetworkRequest(url));
         }
        else if(backward){
            QUrl url("http://" + this->adresse +":8080" + cameradown);
            camera->get(QNetworkRequest(url));
         }
        else if(left){
            QUrl url("http://" + this->adresse +":8080" + cameraleft);
            camera->get(QNetworkRequest(url));
         }
        else if(right){
            QUrl url("http://" + this->adresse +":8080" + cameraright);
            camera->get(QNetworkRequest(url));
         }
        buf.clear();
        buf.append((char)0xff);
        buf.append((char)0x07);
        buf.append((char)0x00);
        buf.append((char)0x00);
        buf.append((char)0x00);
        buf.append((char)0x00);
        buf.append((char)0b01010000);
        quint16 crc = this->crc16(buf, 1);
        buf.append((char)crc);
        buf.append((char)(crc>>8));
    }
    else{
        buf.clear();
        buf.append((char)0xff);
        buf.append((char)0x07);
        if(foreward || backward || left || right)
            if(left || right || (foreward && this->cpt_ir1 > 0))
                buf.append((char)vitesse);
            else if(backward && this->cpt_ir2 > 0)
                buf.append((char)vitesse);
            else
                buf.append((char)0x00);
        else
            buf.append((char)0x00);
        buf.append((char)0x00);
        if(foreward || backward || left || right)
            if(left || right || (foreward && this->cpt_ir1 > 0))
                buf.append((char)vitesse);
            else if(backward && this->cpt_ir2 > 0)
                buf.append((char)vitesse);
            else
                buf.append((char)0x00);
        else
            buf.append((char)0x00);
        buf.append((char)0x00);
        if(this->foreward)
        //avant / avant
            buf.append((char)0b01010000);
        //arrière / avant
        else if(this->left)
            buf.append((char)0b00010000);
        //avant / arrière
        else if(this->right)
            buf.append((char)0b01000000);
        //arrière / arrière
        else if(this->backward)
            buf.append((char)0b00000000);
        else
            buf.append((char)0b01010000);
        quint16 crc = this->crc16(buf, 1);
        buf.append((char)crc);
        buf.append((char)(crc>>8));
    }
}

void Communication::sendMessage(){
    tcp.write(buf);
    tcp.flush();
}

void Communication::recvMessage(){
    char recv[21];
    tcp.read(recv, 21);
    this->battery = (((unsigned int)((unsigned char)recv[2])) * 100.0 / 255.0);
    this->cpt_ir1 = (int) recv[3];
    this->cpt_ir2 = (int) recv[4];
}

quint16 Communication::crc16(QByteArray byteArray, int pos){
    unsigned char *data = (unsigned char* )byteArray.constData();
    quint16 crc = 0xFFFF;
    quint16 Polynome = 0xA001;
    quint16 Parity = 0;
    for(; pos < byteArray.length(); pos++){
        crc ^= *(data+pos);
        for (unsigned int CptBit = 0; CptBit <= 7 ; CptBit++){
            Parity= crc;
            crc >>= 1;
            if (Parity%2 == true) crc ^= Polynome;
        }
    }
    return crc;
}
