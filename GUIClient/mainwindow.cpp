#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QList<QString> operators=QList<QString>({"+","-","*","/"});
    ui->comboBox_operator->addItems(operators);
    QValidator *operand_validator = new QRegularExpressionValidator(QRegularExpression("^[0-9]*$"));
    ui->lineEdit_LeftOperand->setValidator(operand_validator);
    ui->lineEdit_RightOperand->setValidator(operand_validator);
    serial_refresh_timer = new QTimer(this);
    connect(serial_refresh_timer, &QTimer::timeout, this, &MainWindow::refreshPorts);
    serial_refresh_timer->start(100);
    ui->pushButton_Calculate->setDisabled(true);
    ui->pushButton_Calculate->setToolTip(QString("You need to Connect first"));//TODO Use language file
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshPorts()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo().availablePorts();
    QList<QString> portnames=QList<QString>(ports.length());
    for(int i=0; i<ports.length();i++){
        //qDebug()<<ports[i].portName();
        portnames[i] = ports[i].portName();
    }
    if(portnames_in_view != portnames){ //We only want to update the popup when sth. actually changed cause otherwise the user is unable to click anything during the refresh
        portnames_in_view = portnames;
        ui->comboBox_Serial->clear();
        ui->comboBox_Serial->addItems(portnames);
        ui->comboBox_Serial->hidePopup(); //This could be kind of annoying but is there to prevent strange behavior of the Popup when the items change
    }
}


void MainWindow::on_pushButton_Connect_clicked()
{
    port.setPortName(ui->comboBox_Serial->currentText());
    port.setBaudRate(115200);
    port.open(QIODevice::ReadWrite);
    ui->comboBox_Serial->setDisabled(true);
    ui->pushButton_Connect->setDisabled(true);
    serial_refresh_timer->stop();
    ui->pushButton_Calculate->setDisabled(false);
    ui->pushButton_Calculate->setToolTip(QString(""));
}


void MainWindow::on_pushButton_Calculate_clicked()
{
    QString message_str="AB"+ui->lineEdit_LeftOperand->text()+"C"+ui->comboBox_operator->currentText()+"D"+ui->lineEdit_RightOperand->text()+"E";
    QByteArray message = message_str.toUtf8();
    message.append(QString("X").toUtf8());
    message.append(QString::number(7139).toUtf8());
    message.append(QString("Y").toUtf8());
    qint32 checksum = calculate_checksum(message);
    message.append(QString::number(checksum).toUtf8());
    message.append(QString("Z\n").toUtf8());
    qDebug()<<message;
    port.write(message);
    port.waitForReadyRead(10000);
    qDebug()<<port.readLine();
}

quint8 MainWindow::calculate_checksum(QByteArray message)
{
    // int32 is chosen because arduino string to int function returns int32_t.
    // by using the same datatype it is ensured that the comparison will work even when an overflow occurs since
    // this overflow happens on both sides in the same way.
    quint8 toreturn = 0;
    for(int i=0;i<message.length();i++)
    {
        toreturn += message[i];
    }
    return toreturn;
}
