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
    QString message="A"+ui->lineEdit_LeftOperand->text()+"B"+ui->comboBox_operator->currentText()+"C"+ui->lineEdit_LeftOperand->text()+"D\n";
    port.write(message.toUtf8());
    port.waitForReadyRead(2000);
    qDebug()<<port.readLine();
}

