#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


//-----FOR RS32-----//


//TO GET DATA from the Chassis

//DSPIC C code to show you the sending part to the PC:
bufsend[0]=(-speedlab);
bufsend[1]=(-speedlab >> 8);
//speed is a short and it is tics / 50 ms
bufsend[2]=(unsigned char)(tmpadc2 >> 2);
//Bat Volt:10.1V 1.28V 404/4->101
bufsend[3]=(unsigned char)(tmpadc4 >> 2);
//3.3v->255 2v-> 624/4 -> 156
bufsend[4]=(unsigned char)(tmpadc3 >> 2);
//3.3v->255 2v-> 624/4 -> 156
bufsend[5]=bufposition[0];
//Acumulated odometrie is a float
bufsend[6]=bufposition[1];
//12ppr x 4 x 51 gear box = 2448 tics/wheel turn
bufsend[7]=bufposition[2];
bufsend[8]=bufposition[3];
bufsend[9]=(speedlab2);
bufsend[10]=(speedlab2 >> 8);
bufsend[11]=(unsigned char)(tmpadc0 >> 2);
bufsend[12]=(unsigned char)(tmpadc1 >> 2);
bufsend[13]=bufposition2[0];
bufsend[14]=bufposition2[1];
bufsend[15]=bufposition2[2];
bufsend[16]=bufposition2[3];
bufsend[17]=0;
//robot current
// *0.194-37.5 = I in Amp / * 10 for the GUI : 10 -> 1 A (ACS712 30Amp chip) // *0.129-25 =I (for ACS712 20A chip)
bufsend[18]=14;
//firmware version
bufsend[19]=crc16 low;
bufsend[20]=crc16 high;


//Receive by using
int GetMotorRS23233f(HANDLE hUSB,SensorData *dataL,SensorData *dataR) {
    DWORD n;
    BYTE sbuf[30];
    bool res=false;
    do {
        ReadFile(hUSB, &sbuf, 1, &n, NULL);
    }
    while(sbuf[0]!=255);
    res = ReadFile(hUSB, &sbuf, 21, &n, NULL);
    short mycrcrcv = (short)((sbuf[20] << 8) + sbuf[19]);
    short mycrcsend = Crc16(sbuf,19);
    if (mycrcrcv!=mycrcsend)  {
        do {
            ReadFile(hUSB, &sbuf, 1, &n, NULL);
        }while(sbuf[0]!=255);
    }
    else {
        dataL->SpeedFront=(int)((sbuf[1] << 8) + sbuf[0]);
        if (dataL->SpeedFront > 32767) dataL->SpeedFront=dataL>SpeedFront-65536;
        dataL->BatLevel=sbuf[2];
        dataL->IR=sbuf[3];
        dataL->IR2=sbuf[4];
        dataL->odometry=((((long)sbuf[8] << 24))+(((long)sbuf[7] << 16))+(((long)sbuf[6] << 8))+((long)sbuf[5]));
        dataR->SpeedFront=(int)(sbuf[10] << 8) + sbuf[9];
        if (dataR->SpeedFront > 32767)
            dataR->SpeedFront=dataR>SpeedFront-65536;
        dataR->BatLevel=0;
        dataR->IR=sbuf[11];
        dataR->IR2=sbuf[12];
        dataR->odometry=((((long)sbuf[16] << 24))+(((long)sbuf[15] << 16))+(((long)sbuf[14] << 8))+((long)sbuf[13]));
        dataL->Current=sbuf[17];
        dataR->Current=sbuf[17];
        dataL->Version=sbuf[18];
        dataR->Version=sbuf[18];
    }
    return res;
}

//SET PID(If we want to set a new PID on the left & right side at P=0.77 I=0.01 D=0.30

SetMotorPIDRS23233f(hUSB,0x00,0x00,77,1,30,360);
int SetMotorPIDRS23233f(HANDLE hUSB,BYTE speed1,BYTE speed2,BYTE pp,BYTE ii,BYTE dd,short maxspeed) {
    DWORD n;
    BYTE sbuf[30];
    sbuf[0] = 255;
    sbuf[1] = 0x09;
    sbuf[2] = speed1;
    //Always 0
    sbuf[3] = speed2;
    //Always 0
    sbuf[4] = pp;
    sbuf[5] = ii;
    sbuf[6] = dd;
    sbuf[7] = (BYTE)maxspeed;
    sbuf[8] = (BYTE)(maxspeed >> 8);
    short mycrcsend = Crc16(sbuf+1,8);
    sbuf[9] = (BYTE)mycrcsend;
    sbuf[10] = (BYTE)(mycrcsend >> 8);
    WriteFile(hUSB, &sbuf, 11, &n, NULL);
    return sbuf[0];
}

int SetMotorRS23233f_low_res(HANDLE hUSB, BYTE speed1,BYTE speed2) {
    DWORD n;
    BYTE sbuf[30];
    BYTE tt=0;
    sbuf[0] = 255;
    sbuf[1] = 0x07;
    int tmp1 = 8*(speed1&0x3F);
    int tmp2 = 8*(speed2&0x3F);
    if (speed2&0x80) tt=tt+32;
    if (speed2&0x40) tt=tt+16;
    sbuf[2] = (BYTE)tmp1;
    sbuf[3] = (BYTE)(tmp1 >> 8);
    sbuf[4] = (BYTE)tmp2;
    sbuf[5] = (BYTE)(tmp2 >> 8);
    sbuf[6] = (speed1&0x80) + (speed1&0x40) + tt +1;//+1 Relay ON +8 10ms
    pid mode ;
    short mycrcsend = Crc16(sbuf+1,6);
    sbuf[7] = (BYTE)mycrcsend;
    sbuf[8] = (BYTE)(mycrcsend >> 8);
    bool res = WriteFile(hUSB, &sbuf, 9,&n, NULL);
    return res;
}

//SET SPEED TO MOVE
SetMotorRS23233f(hUSB,120,120,80);

/*
If we want the left side & right side to move at speed 20 forward without motor control we send:
128*0 + 64*1 + 20*1 = 84
SetMotorRS23233f_low_res(hUSB,84,84);

If we want the left side to move at speed 20 backward with motor control we send:
128*1 + 64*0 + 20*1 = 148
SetMotorRS23233f_low_res(hUSB,148,148);

*/
//hUSB is the serial port handle opened at 19200 baud
int SetMotorRS23233f(HANDLE hUSB, short speed1,short speed2,BYTE SpeedFlag) {
    DWORD n;
    BYTE sbuf[10];
    sbuf[0] = 255;
    sbuf[1] = 0x07;
    sbuf[2] = (BYTE)speed1;
    sbuf[3] = (BYTE)(speed1 >> 8);
    sbuf[4] = (BYTE)speed2;
    sbuf[5] = (BYTE)(speed2 >> 8);
    sbuf[6] = SpeedFlag+1;
    short mycrcsend = Crc16(sbuf+1,6);
    sbuf[7] = (BYTE)mycrcsend;
    sbuf[8] = (BYTE)(mycrcsend >> 8);
    bool res = WriteFile(hUSB, &sbuf, 9,&n, NULL);  return res;
}


//SERIAL PORT OPEN (Windows)
HANDLE SetupRS232CommPort( LPCSTR comport) {
    HANDLE hCom;
    DCB dcb;
    COMMTIMEOUTS ct;
    hCom = CreateFileA( comport, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    GetCommState(hCom, &dcb);
    dcb.BaudRate = RS232_SPEED;
    //19200 bauds
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fAbortOnError = FALSE;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = 0;
    SetCommState(hCom, &dcb);
    GetCommTimeouts(hCom, &ct);
    ct.ReadIntervalTimeout = 500;
    ct.ReadTotalTimeoutMultiplier =500;
    ct.ReadTotalTimeoutConstant = 500;
    SetCommTimeouts(hCom, &ct);
    SetCommMask(hCom, EV_RXCHAR);
    return hCom;
}

//CRC 16 bits (You should not use the starting bit "255")

short Crc16(unsigned char *Adresse_tab , unsigned char Taille_max) {
    unsigned int Crc = 0xFFFF;
    unsigned int Polynome = 0xA001;
    unsigned int CptOctet = 0;
    unsigned int CptBit = 0;
    unsigned int Parity= 0;
    Crc = 0xFFFF;
    Polynome = 0xA001;
    for ( CptOctet= 0 ; CptOctet < Taille_max ; CptOctet++)  {
        Crc ^= *( Adresse_tab + CptOctet);
        for ( CptBit = 0; CptBit <= 7 ; CptBit++)   {
            Parity= Crc;
            Crc >>= 1;
            if (Parity%2 == true) Crc ^= Polynome;
        }
    }
    return(Crc);
}





//-----FOR ETHERNET / WIFI-----//

//TO GET DATA FROM THE CHASSIS TDP

//DSPIC C code to show you the sending part to the PC:
//DSPIC send first a ‘255’ char to help synchronise your software and after (the ‘255’ is not sent in TCP or UDP):
bufsend[0]=(-speedlab);
bufsend[1]=(-speedlab >> 8);
//speed is a short and it is tics /50ms or 10ms
bufsend[2]=(unsigned char)(tmpadc2 >> 2);
//Bat Volt:10.1V 1.28V 404/4->101
bufsend[3]=(unsigned char)(tmpadc4 >> 2);
//3.3v->255 2v-> 624/4 -> 156
bufsend[4]=(unsigned char)(tmpadc3 >> 2);
//3.3v->255 2v-> 624/4 -> 156
bufsend[5]=bufposition[0];
//Acumulated odometrie is a float
bufsend[6]=bufposition[1];
//12ppr x 4 x 51 gear box = 2448 tics/wheel turn
bufsend[7]=bufposition[2];
bufsend[8]=bufposition[3];
bufsend[9]=(speedlab2);
bufsend[10]=(speedlab2 >> 8);
bufsend[11]=(unsigned char)(tmpadc0 >> 2);
bufsend[12]=(unsigned char)(tmpadc1 >> 2);
bufsend[13]=bufposition2[0];
bufsend[14]=bufposition2[1];
bufsend[15]=bufposition2[2];
bufsend[16]=bufposition2[3];
bufsend[17]=0;
//robot current
// *0.194-37.5 = I in Amp / * 10 for the GUI : 10 -> 1 A (ACS712 30Amp chip)
bufsend[18]=14;
//firmware version
bufsend[19]=crc16 low;
bufsend[20]=crc16 high;

//Receive like this:
int rcvnbr = recv(socket_handler,sbuf,21,0);
dataL->SpeedFront=(int)((sbuf[1] << 8) + sbuf[0]);
if (dataL->SpeedFront > 32767) dataL->SpeedFront=dataL>SpeedFront-65536;
dataL->BatLevel=sbuf[2];
dataL->IR=sbuf[3];
dataL->IR2=sbuf[4];
dataL->odometry=((((long)sbuf[8] << 24))+(((long)sbuf[7] << 16))+(((long)sbuf[6] << 8))+((long)sbuf[5]));
dataR->SpeedFront=(int)(sbuf[10] << 8) + sbuf[9];
if (dataR->SpeedFront > 32767) dataR->SpeedFront=dataR>SpeedFront-65536;
dataR->BatLevel=0;
dataR->IR=sbuf[11];
dataR->IR2=sbuf[12];
dataR->odometry=((((long)sbuf[16] << 24))+(((long)sbuf[15] << 16))+(((long)sbuf[14] << 8))+((long)sbuf[13]));
dataL->Current=sbuf[17];
dataR->Current=sbuf[17];
dataL->Version=sbuf[18];
dataR->Version=sbuf[18];



//TO GET DATA FROM THE CHASSIS UDP


//DSPIC C code to show you the sending part to the PC:
//DSPIC send first a ‘255’ char to help synchronise your software and after (the ‘255’ is not sent in TCP or UDP):
bufsend[0]=(-speedlab);
bufsend[1]=(-speedlab >> 8);
//speed is a short and it is tics / 50 ms
bufsend[2]=(unsigned char)(tmpadc2 >> 2);
//Bat Volt:10.1V 1.28V 404/4->101
bufsend[3]=(unsigned char)(tmpadc4 >> 2);
//3.3v->255 2v-> 624/4 -> 156
bufsend[4]=(unsigned char)(tmpadc3 >> 2);
//3.3v->255 2v-> 624/4 -> 156
bufsend[5]=bufposition[0];
//Acumulated odometrie is a float
bufsend[6]=bufposition[1];
//12ppr x 4 x 51 gear box = 2448 tics/wheel turn
bufsend[7]=bufposition[2];
bufsend[8]=bufposition[3];
bufsend[9]=(speedlab2);
bufsend[10]=(speedlab2 >> 8);
bufsend[11]=(unsigned char)(tmpadc0 >> 2);
bufsend[12]=(unsigned char)(tmpadc1 >> 2);
bufsend[13]=bufposition2[0];
bufsend[14]=bufposition2[1];
bufsend[15]=bufposition2[2];
bufsend[16]=bufposition2[3];
bufsend[17]=0;
//robot current
// *0.194-37.5 = I in Amp / * 10 for the GUI : 10 -> 1 A (ACS712 30Amp chip)
// *0.129-25 =I (for ACS712 20A chip)
bufsend[18]=14;
//firmware version
bufsend[19]=crc16 low;
bufsend[20]=crc16 high;


//You receive on your pc those  21 chars like this:
int rcvnbr = recvfrom(udp_socket_handler,sbuf,21,0);
dataL->SpeedFront=(int)((sbuf[1] << 8) + sbuf[0]);
if (dataL->SpeedFront > 32767)
dataL->SpeedFront=dataL>SpeedFront-65536;
dataL->BatLevel=sbuf[2];
dataL->IR=sbuf[3];
dataL->IR2=sbuf[4];
dataL->odometry=((((long)sbuf[8] << 24))+(((long)sbuf[7] << 16))+(((long)sbuf[6] << 8))+((long)sbuf[5]));
dataR->SpeedFront=(int)(sbuf[10] << 8) + sbuf[9];
if (dataR->SpeedFront > 32767)
dataR->SpeedFront=dataR>SpeedFront-65536;
dataR->BatLevel=0;
dataR->IR=sbuf[11];
dataR->IR2=sbuf[12];
dataR->odometry=((((long)sbuf[16] << 24))+(((long)sbuf[15] << 16))+(((long)sbuf[14] << 8))+((long)sbuf[13]));
dataL->Current=sbuf[17];
dataR->Current=sbuf[17];
dataL->Version=sbuf[18];
dataR->Version=sbuf[18];

//In sbuf[20] and sbuf[19] you have the 16bits CRC  Witch is important in UDP.


//CRC 16 bits (you should not use the starting bit “255”):
short Crc16(unsigned char *Adresse_tab , unsigned char Taille_max) {
    unsigned int Crc = 0xFFFF;
    unsigned int Polynome = 0xA001;
    unsigned int CptOctet = 0;
    unsigned int CptBit = 0;
    unsigned int Parity= 0;
    Crc = 0xFFFF;
    Polynome = 0xA001;
    for ( CptOctet= 0 ; CptOctet < Taille_max ; CptOctet++)  {
        Crc ^= *( Adresse_tab + CptOctet);
        for( CptBit = 0; CptBit <= 7 ; CptBit++)   {
            Parity= Crc;
            Crc >>= 1;
            if (Parity%2 == true) Crc ^= Polynome;
        }
    }
    return(Crc);
}
