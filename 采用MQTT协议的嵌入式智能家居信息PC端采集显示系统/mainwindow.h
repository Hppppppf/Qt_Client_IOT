//mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QtMqtt/QMqttClient>
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
    void on_lightButton_clicked();
    void on_pushButton_clicked();
    //拖拽窗口
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    //连接服务器
    void Connect();
    void Subscribe();
    //显示参数
    void updateData(QString content);
    void Display_Data(QString content);
    void ChangeColor(float data,float standard,int (&color)[3]);
private:
    Ui::MainWindow *ui;
    //拖拽窗口
    bool isDrag;
    QPoint mouseStartPoint;
    QPoint windowTopLeftPoint;
    //智能家居环境参数
    bool light_switch;
    float temperature;
    float humidity;
    float PM25;
    int CO2;
    int light;
    //mqtt client
    QMqttClient * m_client;
    QMqttClient * m_client_2;
    void updateLogStateChange();
    void brokerDisconnected();
};
#endif // MAINWINDOW_H
