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

    void refreshPorts();

    void on_pushButton_Connect_clicked();

    void on_pushButton_Calculate_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_subtract_clicked();

    void on_pushButton_multiply_clicked();

    void on_pushButton_divide_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort port;
    quint8 calculate_checksum(QByteArray message);
    QTimer *serial_refresh_timer;
    QList<QString> portnames_in_view;
    QString selected_operator;
};
#endif // MAINWINDOW_H
