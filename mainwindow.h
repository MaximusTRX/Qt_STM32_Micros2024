#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <qpaintbox.h>
#include <QDateTime>
#include <QtSerialPort/QSerialPort>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include <QtWidgets>
#include <QSerialPortInfo>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /*!
     * \brief onQTimer1
     *
     * Recibe la señal timeout de QTimer
     */
    void onQTimer1();

    void onQSerialPort1Rx();

//    void paintEvent(QPaintEvent *event);

    void on_pushButton_3_clicked();

    void sendData();

    void decodeData();

    void on_encodeData_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_actionEnable_Debigging_toggled(bool arg1);

    void on_actionIR_Sensors_toggled(bool arg1);

    void on_actionDistancia_toggled();

    void on_actionVelocidad_toggled();

    void on_pushButton_Refresh_clicked();

private:
    Ui::MainWindow *ui;

    QSerialPort *QSerialPort1;

    QTimer *QTimer1;
    QPaintBox *QPaintBox1;
    QPixmap *backGroundClock;

    void drawBackGround(QPixmap *bkPixmap);

    QString strRx, strRxProcess;

    uint8_t bufRX[150], index, nbytes, cks, header, timeoutRx;
    uint8_t payLoad[4], ID, length;

    int counter;

    typedef struct _sSetEnable{
        bool horquilla=false;
        bool ultrasonic=false;
        bool irsensor=false;
    }_sSetEnable;
    _sSetEnable isEnable;

    typedef union {
        int32_t i32;
        int8_t i8[4];
        uint32_t ui32;
        uint16_t ui16[2];
        uint8_t ui8[4];
    }_udat;

    _udat myWord;

    typedef enum{
        ACK=0x0D,
        ALIVE=0xF0,
        FIRMWARE=0xF1,
        IR_SENSOR=0xA0,
        MOTOR_ACTION=0xA1,
        SERVO_ACTION=0xA2,
        ULTRA_SONIC=0xA3,
        HORQUILLA=0xA4,
        OTHERS
    }_eID;

    uint16_t valueIRIzq=0, valueIRDer=0, valueIRMed=0;

};
#endif // MAINWINDOW_H
