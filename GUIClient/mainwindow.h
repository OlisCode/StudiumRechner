#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

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

    void on_pushButton_refreshPorts_clicked();

    void on_pushButton_Connect_clicked();

    void on_pushButton_Calculate_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort port;
    quint8 calculate_checksum(QByteArray message);
};
#endif // MAINWINDOW_H
