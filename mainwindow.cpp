#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    counter = 0;

    QTimer1 = new QTimer(this);
    connect(QTimer1, &QTimer::timeout, this, &MainWindow::onQTimer1);

    //QPaintBox1 = new QPaintBox(0, 0, ui->widget);
    //backGroundClock = new QPixmap(QPaintBox1->width(), QPaintBox1->height());

    ui->statusbar->showMessage("Powered by Maxi");

    QSerialPort1 = new QSerialPort(this);
    QSerialPort1->setPortName("COM10");
    QSerialPort1->setBaudRate(115200);
    QSerialPort1->setDataBits(QSerialPort::Data8);
    QSerialPort1->setParity(QSerialPort::NoParity);
    QSerialPort1->setFlowControl(QSerialPort::NoFlowControl);
    connect(QSerialPort1, &QSerialPort::readyRead, this, &MainWindow::onQSerialPort1Rx);

    header = 0; //Esperando la 'U';

    ui->comboBox->addItem("ALIVE",ALIVE);
    ui->comboBox->addItem("MOTOR TEST",MOTOR_ACTION);
    ui->comboBox->addItem("SERVO",SERVO_ACTION);
//    ui->comboBox->addItem("HORQUILLA",HORQUILLA);
//    ui->comboBox->addItem("ULTRA SONICO",ULTRA_SONIC);
//    ui->comboBox->addItem("SENSOR IR",IR_SENSOR);

    ui->encodeData->setEnabled(false);

    ui->label_selec->setEnabled(false);
    ui->checkBox->setEnabled(false);
    ui->checkBox_2->setEnabled(false);

    ui->label_giro->setEnabled(false);
    ui->radioButton_4->setEnabled(false);
    ui->radioButton_5->setEnabled(false);
    ui->doubleSpinBox->setEnabled(false);

    ui->label_veloc_izq->setEnabled(false);
//    ui->lineEdit->setEnabled(false);
//    ui->label_veloc_der->setEnabled(false);
//    ui->lineEdit_2->setEnabled(false);
//    ui->label_dist->setEnabled(false);
//    ui->lineEdit_3->setEnabled(false);
    ui->lcdNumber->setEnabled(false);
    ui->label_veloc_der->setEnabled(false);
    ui->lcdNumber_2->setEnabled(false);
    ui->label_dist->setEnabled(false);
    ui->lcdNumber_3->setEnabled(false);


    ui->label_irizq->setEnabled(false);
    ui->lcdNumber_4->setEnabled(false);
    ui->label_irder->setEnabled(false);
    ui->lcdNumber_5->setEnabled(false);
    ui->label_irmed->setEnabled(false);
    ui->lcdNumber_6->setEnabled(false);

    ui->plainTextEdit->setVisible(false);

    ui->progressBar->setEnabled(false);
    ui->progressBar_2->setEnabled(false);
    ui->progressBar_3->setEnabled(false);
    ui->progressBar_4->setEnabled(false);
    ui->progressBar_5->setEnabled(false);
    ui->progressBar_6->setEnabled(false);

    ui->progressBar->setRange(0,50);
    ui->progressBar->setValue(0);

    ui->progressBar_2->setRange(0,50);
    ui->progressBar_2->setValue(0);

    ui->progressBar_3->setRange(0,40);
    ui->progressBar_3->setValue(0);

    ui->progressBar_4->setRange(0,200);
    ui->progressBar_4->setValue(0);

    ui->progressBar_5->setRange(0,200);
    ui->progressBar_5->setValue(0);

    ui->progressBar_6->setRange(0,200);
    ui->progressBar_6->setValue(0);



//    ui->addWidget(QToolButton);
    setWindowTitle("Panel de Control del AUTITO");

    QTimer1->start(50);
}

MainWindow::~MainWindow()
{
    delete QTimer1;
    delete QPaintBox1;
    delete ui;
}

//void MainWindow::paintEvent(QPaintEvent *event)
//{

//}

void MainWindow::onQTimer1(){
    if(header){
        timeoutRx--;
        if(!timeoutRx)
            header = 0;
    }
}

//'<' header
//byte1
//byte2
//byte3
//byte4
//checksum = suma de todos los bytes transmitidos
//'>' tail

//  4      1      1    1    N     1
//HEADER NBYTES TOKEN ID PAYLOAD CKS

//HEADER 4 bytes
//'U' 'N' 'E' 'R'

//NBYTES = ID+PAYLOAD+CKS = 2 + nbytes de payload

//TOKEN: ':'

//CKS: xor de todos los bytes enviados menos el CKS
void MainWindow::drawBackGround(QPixmap *bkPixmap){
    QPainter paint(bkPixmap);
}

void MainWindow::on_pushButton_3_clicked()
{
    if(QSerialPort1->isOpen()){
        QSerialPort1->close();
        ui->pushButton_3->setText("OPEN");
        ui->encodeData->setEnabled(false);
    }
    else{
        if(QSerialPort1->open(QSerialPort::ReadWrite)){
            ui->encodeData->setEnabled(true);
            ui->pushButton_3->setText("CLOSE");
            //Pregunto version del firmware
            ID = FIRMWARE;
            length = 0;
            sendData();
        }else{
            QMessageBox::information(this, "PORT", "NO se pudo abrir el PUERTO");
        }
    }
}

void MainWindow::onQSerialPort1Rx(){
    int count;
    uint8_t *buf;
    QString strHex;
    count = QSerialPort1->bytesAvailable();
    if(count <= 0)
        return;


    buf = new uint8_t[count];
    QSerialPort1->read((char *)buf, count);

    strHex = "<-0x";
    for (int i=0; i<count; i++) {
        strHex = strHex + QString("%1").arg(buf[i], 2, 16, QChar('0')).toUpper();
    }
    ui->plainTextEdit->appendPlainText(strHex);

    for (int i=0; i<count; i++) {
        switch (header) {
        case 0://Esperando la 'U'
            if(buf[i] == 'U'){
                header = 1;
                timeoutRx = 3;
            }
            break;
        case 1://'N'
            if(buf[i] == 'N')
                header = 2;
            else{
                header = 0;
                i--;
            }
            break;
        case 2://'E'
            if(buf[i] == 'E')
                header = 3;
            else{
                header = 0;
                i--;
            }
            break;
        case 3://'R'
            if(buf[i] == 'R')
                header = 4;
            else{
                header = 0;
                i--;
            }
            break;
        case 4://Cantidad de Bytes
            nbytes = buf[i];
            header = 5;
            break;
        case 5://El TOKEN ':'
            if(buf[i] == ':'){
                header = 6;
                cks = 'U' ^ 'N' ^ 'E' ^ 'R' ^ nbytes ^ ':';
                index = 0;
            }
            else{
                header = 0;
                i--;
            }
            break;
        case 6://ID + PAYLOAD + CKS
            bufRX[index++] = buf[i];
            if(nbytes != 1)
                cks ^= buf[i];
            nbytes--;
            if(!nbytes){
                header = 0;
                if(buf[i] == cks)
                    decodeData();
            }
            break;
        default:
            header = 0;
        }
    }

    delete [] buf;
}

void MainWindow::decodeData(){
    strRxProcess = "Made by Bonnin ---- Version: 0x";
    uint16_t valueIRIzq=0, valueIRDer=0, valueIRMed=0;
    uint32_t speedM1 = 0, speedM2 = 0;
    float dataRecive;


    switch (bufRX[0]) {
    case 0x11:
        ui->plainTextEdit->appendPlainText(QString("%1").arg(bufRX[1], 2, 16, QChar('0')).toUpper());
        ui->plainTextEdit->appendPlainText(QString("%1").arg(bufRX[2], 2, 16, QChar('0')).toUpper());
        break;

    case FIRMWARE:
        strRxProcess += QString("%1").arg(bufRX[1], 2, 16, QChar('0')).toUpper();
        ui->statusbar->showMessage(strRxProcess);
        break;

    case IR_SENSOR:
        myWord.ui8[0] = bufRX[1];
        myWord.ui8[1] = bufRX[2];
        valueIRIzq = myWord.ui32;

        myWord.ui8[0] = bufRX[3];
        myWord.ui8[1] = bufRX[4];
        valueIRDer = myWord.ui32;

        myWord.ui8[0] = bufRX[5];
        myWord.ui8[1] = bufRX[6];
        valueIRMed = myWord.ui32;


        if (isEnable.irsensor){
//            ui->lineEdit_4->setText(QString("%1").arg(valueIRIzq));
//            ui->lineEdit_5->setText(QString("%1").arg(valueIRDer));
            ui->lcdNumber_4->display(QString("%1").arg(valueIRIzq));
            ui->lcdNumber_5->display(QString("%1").arg(valueIRDer));
            ui->lcdNumber_6->display(QString("%1").arg(valueIRMed));
            ui->progressBar_4->setValue(valueIRIzq);
            ui->progressBar_5->setValue(valueIRDer);
            ui->progressBar_6->setValue(valueIRMed);
        }else{
            ui->lcdNumber_4->display("0");
            ui->lcdNumber_5->display("0");
            ui->lcdNumber_6->display("0");
            ui->progressBar_4->setValue(0);
            ui->progressBar_5->setValue(0);
            ui->progressBar_6->setValue(0);
        }
        break;

    case ULTRA_SONIC:
        myWord.ui8[0] = bufRX[1];
        myWord.ui8[1] = bufRX[2];
        myWord.ui8[2] = bufRX[3];
        myWord.ui8[3] = bufRX[4];
        dataRecive = myWord.ui32/58.0;
        //ui->plainTextEdit->appendPlainText(QString("%1").arg(dataRecive));
        if (isEnable.ultrasonic){
//            ui->lineEdit_3->setText(QString("%1").arg(dataRecive));
            ui->lcdNumber_3->display(QString("%1").arg(dataRecive));
            ui->progressBar_3->setValue(dataRecive);
        }else{
//            ui->lineEdit_3->clear();
            ui->lcdNumber_3->display("0");
            ui->progressBar_3->setValue(0);
        }
        break;

    case HORQUILLA:
        myWord.ui8[0] = bufRX[1];
        myWord.ui8[1] = bufRX[2];
        myWord.ui8[2] = bufRX[3];
        myWord.ui8[3] = bufRX[4];
        speedM1 = myWord.ui32;

        myWord.ui8[0] = bufRX[5];
        myWord.ui8[1] = bufRX[6];
        myWord.ui8[2] = bufRX[7];
        myWord.ui8[3] = bufRX[8];
        speedM2 = myWord.ui32;
        if (isEnable.horquilla){
//            ui->lineEdit->setText(QString("%1").arg(speedM1));
//            ui->lineEdit_2->setText(QString("%1").arg(speedM2));
            ui->lcdNumber->display(QString("%1").arg(speedM1));
            ui->lcdNumber_2->display(QString("%1").arg(speedM2));
            ui->progressBar->setValue(speedM1);
            ui->progressBar_2->setValue(speedM2);
        }else{
//            ui->lineEdit_2->clear();
            ui->lcdNumber->display("0");
            ui->lcdNumber_2->display("0");
            ui->progressBar->setValue(0);
            ui->progressBar_2->setValue(0);
        }
        break;
    }
}

void MainWindow::on_encodeData_clicked()
{
    uint8_t cmd;
    _udat dataPayload;
    bool ok;
    bool readyToSend = false;

    cmd = ui->comboBox->currentData().toInt();

    ui->plainTextEdit->appendPlainText(QString("%1").arg(cmd, 2, 16, QChar('0')).toUpper());

    switch (cmd) {
    case ALIVE:
        ID = ALIVE;
        length = 0;
        readyToSend = true;
        break;

    case IR_SENSOR:
//        ID = IR_SENSOR;
//        length = 0;
//        readyToSend = true;

//        if(isEnable.irsensor){
//            isEnable.irsensor=false;
//            ui->label_irizq->setEnabled(false);
//            ui->label_irder->setEnabled(false);
//            ui->lcdNumber_4->setEnabled(false);
//            ui->lcdNumber_5->setEnabled(false);
//        }else {
//            isEnable.irsensor=true;
//            ui->label_irizq->setEnabled(true);
//            ui->label_irder->setEnabled(true);
//            ui->lcdNumber_4->setEnabled(true);
//            ui->lcdNumber_5->setEnabled(true);
//        }


        break;

    case MOTOR_ACTION:
        //Motor a mover, 0x01 motor derecha, 0x10 motor izquierda, 0x11 ambos motores
        //Sentido de giro, 0x01 atras, 0x10 adelante
        //Intervalo de movimiento
        payLoad[0] = 0x00;
        if (ui->checkBox->isChecked())
            payLoad[0] |= 0x10;
        if (ui->checkBox_2->isChecked())
            payLoad[0] |= 0x01;
        if(payLoad[0])
            readyToSend = true;
        else
            QMessageBox::information(this, "MOTOR", "Seleccione MOTOR a GIRAR");

        payLoad[1] = 0x00;
        if (ui->radioButton_4->isChecked())
            payLoad[1] |= 0x10;
        if (ui->radioButton_5->isChecked())
            payLoad[1] |= 0x01;
        if (payLoad[1])
            readyToSend = true;
        else
            QMessageBox::information(this, "MOTOR", "Seleccione SENTIDO a GIRAR");

        //ui->spinBox->setRange(1, 90);
        dataPayload.ui32 = (ui->doubleSpinBox->value()*1000);
        payLoad[2] = dataPayload.ui8[0];
        payLoad[3] = dataPayload.ui8[1];

        ID = MOTOR_ACTION;
        length= 4;
        break;

    case SERVO_ACTION:
        dataPayload.i8[0] = QInputDialog::getInt(this, "SERVO","ANGULO", 0, -127, 500, 1, &ok);
        payLoad[0] = dataPayload.ui8[0];

        if(ok){
            ui->plainTextEdit->appendPlainText("OK");
            ui->plainTextEdit->appendPlainText(QString().number(dataPayload.i32));
            readyToSend = true;
        }else{
            ui->plainTextEdit->appendPlainText("CANCEL");
            readyToSend = false;
        }

        ID = SERVO_ACTION;
        length = 1;
        break;

    case ULTRA_SONIC:
//        ID = ULTRA_SONIC;
//        length = 0;
//        readyToSend = true;
//        if (isEnable.ultrasonic){
//            isEnable.ultrasonic=0;
//            ui->label_dist->setEnabled(false);
//            ui->lcdNumber_3->setEnabled(false);
//        }else{
//            isEnable.ultrasonic=1;
//            ui->label_dist->setEnabled(true);
//            ui->lcdNumber_3->setEnabled(true);
//        }
        break;

    case HORQUILLA:
//        ID = HORQUILLA;
//        length = 0;
//        readyToSend = true;

//        if (isEnable.horquilla){
//            isEnable.horquilla=0;
//            ui->label_veloc_izq->setEnabled(false);
//            ui->lcdNumber->setEnabled(false);
//            ui->label_veloc_der->setEnabled(false);
//            ui->lcdNumber_2->setEnabled(false);
//        }else{
//            isEnable.horquilla=1;
//            ui->label_veloc_izq->setEnabled(true);
//            ui->lcdNumber->setEnabled(true);
//            ui->label_veloc_der->setEnabled(true);
//            ui->lcdNumber_2->setEnabled(true);
//        }
        break;
    }

    if (readyToSend)
    {
        sendData();
    }
}

void MainWindow::sendData()
{
    uint8_t tx[12];
    QString str;

    if(QSerialPort1->isOpen()){
        tx[0] = 'U';    //HEADER
        tx[1] = 'N';
        tx[2] = 'E';
        tx[3] = 'R';
        tx[4] = 2 + length;  //NBYTES - Cantidad de bytes (2 + nPayload). El Alive no tiene payload
        tx[5] = ':';    //TOKEN
        tx[6] = ID;   //ID de Alive
        int i;
        if(length != 0){
            for (i=0; i<(length); i++) {
                tx[7+i] = payLoad[i];  //PAYLOAD
            }
        }
        tx[7+length] = 0;

        for(int i=0; i<7+length; i++)
            tx[7+length] ^= tx[i]; //XOR de todos los bytes transmitidos

        QSerialPort1->write((char *)tx, (8+length));
        strRx = "0x";
    }
    else
        QMessageBox::information(this, "PORT", "Abrir el PUERTO");

    str = "->0x";
    for (int i=0; i<8+length; i++) {
        str = str + QString("%1").arg(tx[i], 2, 16, QChar('0')).toUpper();
    }
    ui->plainTextEdit->appendPlainText(str);
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if (index == 01){
        ui->label_selec->setEnabled(true);
        ui->checkBox->setEnabled(true);
        ui->checkBox_2->setEnabled(true);

        ui->label_giro->setEnabled(true);
        ui->radioButton_4->setEnabled(true);
        ui->radioButton_5->setEnabled(true);
        ui->doubleSpinBox->setEnabled(true);
    }else{
        ui->label_selec->setEnabled(false);
        ui->checkBox->setEnabled(false);
        ui->checkBox_2->setEnabled(false);

        ui->label_giro->setEnabled(false);
        ui->radioButton_4->setEnabled(false);
        ui->radioButton_5->setEnabled(false);
        ui->doubleSpinBox->setEnabled(false);
    }
}

void MainWindow::on_actionEnable_Debigging_toggled(bool arg1)
{
    if (arg1)
        ui->plainTextEdit->setVisible(true);
    else
        ui->plainTextEdit->setVisible(false);
}

void MainWindow::on_actionIR_Sensors_toggled(bool arg1)
{
    if(arg1){
        isEnable.irsensor=true;
        ui->label_irizq->setEnabled(true);
        ui->label_irder->setEnabled(true);
        ui->label_irmed->setEnabled(true);
        ui->lcdNumber_4->setEnabled(true);
        ui->lcdNumber_5->setEnabled(true);
        ui->lcdNumber_6->setEnabled(true);
        ui->progressBar_4->setEnabled(true);
        ui->progressBar_5->setEnabled(true);
        ui->progressBar_6->setEnabled(true);
    }else {
        isEnable.irsensor=false;
        ui->label_irizq->setEnabled(false);
        ui->label_irder->setEnabled(false);
        ui->label_irmed->setEnabled(false);
        ui->lcdNumber_4->setEnabled(false);
        ui->lcdNumber_5->setEnabled(false);
        ui->lcdNumber_6->setEnabled(false);
        ui->progressBar_4->setEnabled(false);
        ui->progressBar_5->setEnabled(false);
        ui->progressBar_6->setEnabled(false);
    }
}

void MainWindow::on_actionDistancia_toggled(bool arg1)
{
    if (arg1){
        isEnable.ultrasonic=1;
        ui->label_dist->setEnabled(true);
        ui->lcdNumber_3->setEnabled(true);
        ui->progressBar_3->setEnabled(true);
    }else{
        isEnable.ultrasonic=0;
        ui->label_dist->setEnabled(false);
        ui->lcdNumber_3->setEnabled(false);
        ui->progressBar_3->setEnabled(false);
    }
}


void MainWindow::on_actionVelocidad_toggled(bool arg1)
{
    if (arg1){
        isEnable.horquilla=1;
        ui->label_veloc_izq->setEnabled(true);
        ui->lcdNumber->setEnabled(true);
        ui->label_veloc_der->setEnabled(true);
        ui->lcdNumber_2->setEnabled(true);
        ui->progressBar->setEnabled(true);
        ui->progressBar_2->setEnabled(true);
    }else{
        isEnable.horquilla=0;
        ui->label_veloc_izq->setEnabled(false);
        ui->lcdNumber->setEnabled(false);
        ui->label_veloc_der->setEnabled(false);
        ui->lcdNumber_2->setEnabled(false);
        ui->progressBar->setEnabled(false);
        ui->progressBar_2->setEnabled(false);
    }
}

