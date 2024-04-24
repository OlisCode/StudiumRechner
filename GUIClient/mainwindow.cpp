#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QTimer>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QValidator *operand_validator = new QRegularExpressionValidator(QRegularExpression("^[0-9]*$"));
    ui->lineEdit_LeftOperand->setValidator(operand_validator);
    ui->lineEdit_RightOperand->setValidator(operand_validator);
    serial_refresh_timer = new QTimer(this);
    connect(serial_refresh_timer, &QTimer::timeout, this, &MainWindow::refreshPorts);
    serial_refresh_timer->start(100);
    ui->pushButton_Calculate->setDisabled(true);
    ui->pushButton_Calculate->setToolTip(QCoreApplication::translate("MainWindow", "You need to Connect first"));//TODO Use language file
    ui->pushButton_add->click(); // simple way to prevent a situation where there is no opeartor selected
    ui->label_Status->setText(QCoreApplication::translate("MainWindow", "Disconnected"));//TODO Use language file
    ui->pushButton_add->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_divide->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_subtract->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_multiply->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_Calculate->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_Connect->setFocusPolicy(Qt::NoFocus);
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
    if(port.portName()==""){
        serialconnect();
    }else{
        serialdisconnect();
    }
}

void MainWindow::serialconnect()
{
    if(port.portName()==""){
        port.setPortName(ui->comboBox_Serial->currentText());
        port.setBaudRate(115200);
        port.open(QIODevice::ReadWrite);
        ui->comboBox_Serial->setDisabled(true);
        serial_refresh_timer->stop();
        ui->pushButton_Calculate->setDisabled(false);
        ui->pushButton_Calculate->setToolTip(QString(""));
        ui->label_Status->setText(QCoreApplication::translate("MainWindow", "Connected"));//TODO Use language file
        ui->pushButton_Connect->setText(QCoreApplication::translate("MainWindow", "Disconnect"));//TODO Use language file
    }
}

void MainWindow::serialdisconnect()
{
    port.close();
    port.setPortName("");
    ui->comboBox_Serial->setDisabled(false);
    serial_refresh_timer->stop();
    ui->pushButton_Calculate->setDisabled(true);
    ui->pushButton_Calculate->setToolTip(QCoreApplication::translate("MainWindow", "You need to Connect first"));//TODO Use language file
    ui->label_Status->setText(QCoreApplication::translate("MainWindow", "Disconnected"));//TODO Use language file
    ui->pushButton_Connect->setText(QCoreApplication::translate("MainWindow", "Connect"));//TODO Use language file
    serial_refresh_timer->start(100);
}


void MainWindow::on_pushButton_Calculate_clicked()
{
    ui ->resultlabel->setText(QString(""));
    QString message_str="AB"+ui->lineEdit_LeftOperand->text()+"C"+selected_operator+"D"+ui->lineEdit_RightOperand->text()+"E";
    QByteArray message = message_str.toUtf8();
    message.append(QString("X").toUtf8());
    qint32 id=QRandomGenerator::global()->generate();
    message.append(QString::number(id).toUtf8());
    message.append(QString("Y").toUtf8());
    qint32 checksum = calculate_checksum(message);
    message.append(QString::number(checksum).toUtf8());
    message.append(QString("Z\n").toUtf8());
    qDebug() << "Request: " << message;
    QString response;
    for(quint8 i=0;i<5;i++)
    {
        response="";
        port.write(message);
        port.waitForReadyRead(100);
        response=port.readLine();
        qDebug() << "Response:" <<response;
        qint32 index_bb = response.indexOf("BB");  // If the fist 2 Characters are BB the package should not be answered
        qint32 index_c = response.indexOf('C');
        qint32 index_x = response.indexOf('X');
        qint32 index_y = response.indexOf('Y');
        qint32 index_z = response.indexOf('Z');
        if(index_bb >= 0 && index_c >= 0 && index_x >= 0 && index_x >= 0 && index_y >= 0 && index_z >= 0)
        {
            qDebug()<<"Response contains all Indexes";
            QString id_received = response.mid(index_x+1,index_y-index_x-1);
            qDebug() << "Id received: " << id_received;
            if(QString::number(id)==id_received)
            {
                qDebug() << "Received id matches requested";
                QString checksum_received = response.mid(index_y+1,index_z-index_y-1);
                qDebug() << "Checksum received: " << checksum_received;
                quint8 checksum_calculated = calculate_checksum(response.mid(index_bb,index_y+1-index_bb).toUtf8());
                qDebug() << "Checksum calculated: " << QString::number(checksum_calculated);
                if(QString::number(checksum_calculated)==checksum_received)
                {
                    qDebug() << "Received checksum matches calculated";
                    QString result =response.mid(index_bb+2,index_c-index_bb-2);
                    ui ->resultlabel->setText(result);
                    i=10;
                }
            }
        }

    }
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

void MainWindow::on_pushButton_add_clicked()
{
    selected_operator = QString("+");
    ui->pushButton_add->setChecked(true);
    ui->pushButton_subtract->setChecked(false);
    ui->pushButton_multiply->setChecked(false);
    ui->pushButton_divide->setChecked(false);
}


void MainWindow::on_pushButton_subtract_clicked()
{
    selected_operator = QString("-");
    ui->pushButton_add->setChecked(false);
    ui->pushButton_subtract->setChecked(true);
    ui->pushButton_multiply->setChecked(false);
    ui->pushButton_divide->setChecked(false);
}


void MainWindow::on_pushButton_multiply_clicked()
{
    selected_operator = QString("*");
    ui->pushButton_add->setChecked(false);
    ui->pushButton_subtract->setChecked(false);
    ui->pushButton_multiply->setChecked(true);
    ui->pushButton_divide->setChecked(false);
}


void MainWindow::on_pushButton_divide_clicked()
{
    selected_operator = QString("/");
    ui->pushButton_add->setChecked(false);
    ui->pushButton_subtract->setChecked(false);
    ui->pushButton_multiply->setChecked(false);
    ui->pushButton_divide->setChecked(true);
}


void MainWindow::on_lineEdit_LeftOperand_returnPressed()
{
    ui->pushButton_Calculate->click();
}


void MainWindow::on_lineEdit_RightOperand_returnPressed()
{
    ui->pushButton_Calculate->click();
}

