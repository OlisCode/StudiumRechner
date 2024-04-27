#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QTimer>
#include <QRandomGenerator>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Add a filter to the operands so we can only enter numbers
    QValidator *operand_validator = new QRegularExpressionValidator(QRegularExpression("^[0-9]*$"));
    // Create a timer to automaticly refreshes the serialports listed in the combobox
    serial_refresh_timer = new QTimer(this);
    connect(serial_refresh_timer, &QTimer::timeout, this, &MainWindow::refreshPorts);
    serial_refresh_timer->start(100);
    // Prepare an initial state for the aplication
    ui->pushButton_Calculate->setDisabled(true);
    ui->pushButton_Calculate->setToolTip(QCoreApplication::translate("MainWindow", "You need to Connect first"));
    ui->pushButton_add->click(); // simple way to prevent a situation where there is no opeartor selected
    ui->label_Status->setText(QCoreApplication::translate("MainWindow", "Disconnected"));
    // Remove unwanted elements from the tabindex to simplify usage using keyboard
    ui->pushButton_add->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_divide->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_subtract->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_multiply->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_Calculate->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_Connect->setFocusPolicy(Qt::NoFocus);
    ui->comboBox_Serial->setFocusPolicy(Qt::NoFocus);
    // Prepare the lineedits
    ui->lineEdit_LeftOperand->setValidator(operand_validator);
    ui->lineEdit_RightOperand->setValidator(operand_validator);
    // To enable detection of pressing an operator while in the lineedit QShortcut did not work, therefore an eventfilter is used.
    ui->lineEdit_LeftOperand->installEventFilter(this);
    ui->lineEdit_RightOperand->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    // Check if the event is an operator and then redirect it to the coresponding button
    // if not we just pass it along
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Plus:
            ui->pushButton_add->click();
            break;
        case Qt::Key_Minus:
            ui->pushButton_subtract->click();
            break;
        case Qt::Key_Asterisk:
            ui->pushButton_multiply->click();
            break;
        case Qt::Key_Slash:
            ui->pushButton_divide->click();
            break;
        default:
            return false;
        }
        // Tab to the next operand after the user pressed a operator
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
        QApplication::sendEvent(this, &key);
        return true;
    }
    return false;
}

MainWindow::~MainWindow()
{
    // TODO Disconnect from port
    delete ui;
}

void MainWindow::refreshPorts()
{
    // Update the list of available ports
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
    // Connect to the port or if connected disconnect
    if(port.portName()==""){
        serialconnect();
    }else{
        serialdisconnect();
    }
}

void MainWindow::serialconnect()
{
    // Connect to the port
    if(port.portName()==""){
        port.setPortName(ui->comboBox_Serial->currentText());
        port.setBaudRate(115200);
        port.open(QIODevice::ReadWrite);
        ui->comboBox_Serial->setDisabled(true);
        ui->pushButton_Calculate->setDisabled(false);
        ui->pushButton_Calculate->setToolTip(QString(""));
        ui->label_Status->setText(QCoreApplication::translate("MainWindow", "Connected"));
        ui->pushButton_Connect->setText(QCoreApplication::translate("MainWindow", "Disconnect"));
        serial_refresh_timer->stop();   // Since we are connected there is no point in refreshing the list of available ports
    }
}

void MainWindow::serialdisconnect()
{
    // Disconnect from the port
    port.close();
    port.setPortName("");
    ui->comboBox_Serial->setDisabled(false);
    ui->pushButton_Calculate->setDisabled(true);
    ui->pushButton_Calculate->setToolTip(QCoreApplication::translate("MainWindow", "You need to Connect first"));
    ui->label_Status->setText(QCoreApplication::translate("MainWindow", "Disconnected"));
    ui->pushButton_Connect->setText(QCoreApplication::translate("MainWindow", "Connect"));
    serial_refresh_timer->start(100);   // Start the timer back up to refresh the list of ports
}


void MainWindow::on_pushButton_Calculate_clicked()
{
    // Assemble the task, send it and then disassemble the result
    // Assemble the task
    ui ->resultlabel->setText(QString("")); // Clear the result in the ui to prevent a false display if the rest fails
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
    // Send the taskl up to 5 times if there is a failure in the communication
    for(quint8 i=0;i<5;i++)
    {
        response="";
        port.write(message);    // Send the task
        port.waitForReadyRead(100);
        response=port.readLine();
        qDebug() << "Response:" <<response;
        qint32 index_bb = response.indexOf("BB");  // If the fist 2 Characters are AB the package should not be answered
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
    // by using the same datatype on both the arduino and the client it is ensured that the comparison will work even when an overflow occurs since
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

