#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSerialPort>

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_refreshPorts_clicked()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo().availablePorts();
    QList<QString> portnames=QList<QString>(ports.length());
    for(int i=0; i<ports.length();i++){
        qDebug()<<ports[i].portName();
        portnames[i] = ports[i].portName();
    }
    ui->comboBox_Serial->clear();
    ui->comboBox_Serial->addItems(portnames);
}


void MainWindow::on_pushButton_Connect_clicked()
{
    port.setPortName(ui->comboBox_Serial->currentText());
    port.setBaudRate(9600);
    port.open(QIODevice::ReadWrite);
    ui->comboBox_Serial->setDisabled(true);
    ui->pushButton_refreshPorts->setDisabled(true);
    ui->pushButton_Connect->setDisabled(true);
}


void MainWindow::on_pushButton_Calculate_clicked()
{
    QString message_str="A"+ui->lineEdit_LeftOperand->text()+"B"+ui->comboBox_operator->currentText()+"C"+ui->lineEdit_LeftOperand->text()+"D";
    QByteArray message = message_str.toUtf8();
    qint32 checksum = calculate_checksum(message);
    message.append(QString("Y").toUtf8());
    message.append(QString::number(checksum).toUtf8());
    message.append(QString("Z\n").toUtf8());
    qDebug()<<message;
    port.write(message);
    port.waitForReadyRead(10000);
    qDebug()<<port.readLine();
}

qint32 MainWindow::calculate_checksum(QByteArray message)
{
    // int32 is chosen because arduino string to int function returns int32_t.
    // by using the same datatype it is ensured that the comparison will work even when an overflow occurs since
    // this overflow happens on both sides in the same way.
    qint32 toreturn = 0;
    for(int i=0;i<message.length();i++)
    {
        toreturn += message[i];
    }
    return toreturn;
}
