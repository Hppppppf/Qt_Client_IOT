//mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWidgets/QMessageBox>
#include <windows.h>

//连接服务器参数
#define HOST "host地址"
#define PORT 1883
#define USERNAME "用户名"
#define PASSWORD "密码"
#define CLIENTID "ClientID"
#define SUBSCRIBE_TOPIC "Topic名称"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
      //把图片加载到按钮上
      light_switch = true;
      ui->lightButton->setIcon(QPixmap(":/new/image/light_on.png"));
      //设置图片的大小
      ui->lightButton->setIconSize(QSize(50,50));
      //设置按钮为隐藏
      ui->lightButton->setFlat(true);
      //设置按钮的大小
      ui->lightButton->setFixedSize(50,50);
      ui->lightButton->resize(50,50);
      //隐藏边框，初始化拖拽变量
      this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
      isDrag = false;

      //连接服务器
      m_client = new QMqttClient(this);
      connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
          const QString content =  message;
          qDebug()<<content.toStdString().data()<<endl;
          Display_Data(content);
      });
      connect(m_client, &QMqttClient::stateChanged, this, &MainWindow::updateLogStateChange);
      connect(m_client, &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);
      connect(m_client, &QMqttClient::pingResponseReceived, this, [this]() {
          const QString content = QDateTime::currentDateTime().toString()
                      + QLatin1String(" PingResponse")
                      + QLatin1Char('\n');
      });

      Connect();

      //等待连接完成再订阅
      QTime t;
      t.start();
      while(t.elapsed()<500)
        QCoreApplication::processEvents();

      Subscribe();

}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::Connect()
{
    m_client->setHostname(HOST);
    m_client->setPort(PORT);
    m_client->setUsername(USERNAME);
    m_client->setPassword(PASSWORD);
    m_client->setClientId(CLIENTID);
    m_client->setKeepAlive(300);
    m_client->setCleanSession(true);
    m_client->connectToHost();
    if (m_client->state() == QMqttClient::Disconnected)
    {
        //连接失败
        ui->label_20->setText("未连接");
    }else
    {
        //连接成功
        ui->label_20->setText("已连接");
        ui->label_20->setText("等待订阅");
    }
    m_client_2 = new QMqttClient(this);
    //连接发送服务器
    m_client_2->setHostname("host地址");
    m_client_2->setPort(PORT);
    m_client_2->setUsername("用户名");
    m_client_2->setPassword("密码");
    m_client_2->setClientId(" ClientID");
    m_client_2->setKeepAlive(300);
    m_client_2->setCleanSession(true);
    m_client_2->connectToHost();
}
void MainWindow::Subscribe()
{
    //订阅所需Topic,默认QOS0
    quint8 qos = 0;
    QString sub = SUBSCRIBE_TOPIC;
    qDebug()<<qos<<endl<<sub<<endl;
    auto subscription =m_client->subscribe(sub,qos);
    if (!subscription)
    {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe."));
        //return;
    }
    else
    {
        ui->label_20->setText("订阅成功");
    }
}
//点击关闭按钮
void MainWindow::on_pushButton_clicked()
{
    m_client_2->disconnectFromHost();
    m_client->disconnectFromHost();
    this->close();
}
//拖拽操作
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        isDrag=true;
        //获得鼠标的初始位置
        mouseStartPoint=event->globalPos();
        //获得窗口的初始位置
        windowTopLeftPoint=this->frameGeometry().topLeft();
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isDrag)
    {
        //获得鼠标移动的距离
        QPoint distance=event->globalPos()-mouseStartPoint;
        //改变窗口的位置
        this->move(windowTopLeftPoint+distance);
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        isDrag=false;
    }
}
//点击开关灯按钮
void MainWindow::on_lightButton_clicked()
{
    QString pub = "/a1n0rK9DIAy/Controller/user/get";
    quint8 qos = 0;
    QString ON = "{\"lightswitch\":1}";
    QString OFF = "{\"lightswitch\":0}";
    if (light_switch)
    {
        ui->lightButton->setIcon(QPixmap(":/new/image/light_off.png"));
        light_switch = false;
        if (m_client_2->publish(pub,OFF.toUtf8(),qos) == -1)
            QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
    }
    else
    {
        ui->lightButton->setIcon(QPixmap(":/new/image/light_on.png"));
        light_switch = true;
        if (m_client_2->publish(pub,ON.toUtf8(),qos) == -1)
            QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
    }
}
//更新参数
void MainWindow::updateData(QString content)
{
    //qDebug()<<content.mid(content.indexOf("lux")+35,content.indexOf("},\"PM25")-(content.indexOf("lux")+35));
    if (content.mid(content.indexOf("LightSwitch")+43,content.indexOf("},\"PM25")-(content.indexOf("LightSwitch")+43)) == "1")
    {light_switch = true;}else{light_switch = false;}
    temperature = (content.mid(content.indexOf("RoomTemp")+40,content.indexOf("},\"lux")-(content.indexOf("RoomTemp")+40))).toFloat();
    humidity = (content.mid(content.indexOf("RoomHumidity")+44,content.indexOf("}}}")-(content.indexOf("RoomHumidity")+44))).toFloat();
    PM25 = (content.mid(content.indexOf("PM25")+36,content.indexOf("},\"co2")-(content.indexOf("PM25")+36))).toFloat();
    CO2 = (content.mid(content.indexOf("co2")+35,content.indexOf("},\"RoomTemp")-(content.indexOf("co2")+35))).toInt();
    light = (content.mid(content.indexOf("lux")+35,content.indexOf("},\"RoomHumidity")-(content.indexOf("lux")+35))).toInt();
    qDebug()<<endl<<light;
}
void MainWindow::Display_Data(QString content)
{
    updateData(content);
    //刷新显示
    ui->label_RoomTemp->setText(QString::number(temperature));
    ui->label_RoomHumidity->setText(QString::number(humidity));
    ui->label_lux->setText(QString::number(light));
    ui->label_PM25->setText(QString::number(PM25));
    ui->label_CO2->setText(QString::number(CO2));
    ui->label_TIME->setText(QTime::currentTime().toString());
    //灯的开关
    if (light_switch)
    {
        ui->lightButton->setIcon(QPixmap(":/new/image/light_on.png"));
    }
    else
    {
        ui->lightButton->setIcon(QPixmap(":/new/image/light_off.png"));
    }
    //处理色条显示
    int color[3];
    ChangeColor(temperature,20,color);
    QString styleSheet = QString("background-color: rgb(%1, %2, %3);").arg(color[0]).arg(color[1]).arg(color[2]);
    ui->line_RoomTemp->setStyleSheet(styleSheet);

    ChangeColor(humidity,45,color);
    styleSheet = QString("background-color: rgb(%1, %2, %3);").arg(color[0]).arg(color[1]).arg(color[2]);
    ui->line_RoomHumidity->setStyleSheet(styleSheet);

    ChangeColor(light,1000,color);
    styleSheet = QString("background-color: rgb(%1, %2, %3);").arg(color[0]).arg(color[1]).arg(color[2]);
    ui->line_lux->setStyleSheet(styleSheet);

    ChangeColor(PM25,25,color);
    styleSheet = QString("background-color: rgb(%1, %2, %3);").arg(color[0]).arg(color[1]).arg(color[2]);
    ui->line_PM25->setStyleSheet(styleSheet);

    ChangeColor(CO2,500,color);
    styleSheet = QString("background-color: rgb(%1, %2, %3);").arg(color[0]).arg(color[1]).arg(color[2]);
    ui->line_CO2->setStyleSheet(styleSheet);
}
//计算状态显示颜色
void MainWindow::ChangeColor(float data,float standard,int (&color)[3])
{
    qDebug()<<data-standard<<"/"<<standard<<"=";
    color[0] = 30;
    color[1] = 200;
    color[2] = 30;
    float state = (data-standard)/standard;
    qDebug()<<state<<endl;
    if (state>0)
    {
        if (state>0.5)
        {
            if (state >= 1)
            {
                color[0]=255;
                color[1]=0;
                color[2]=0;
            }else
            {
                color[0] = 200;
                color[1] = 30+120*state;
            }
        }else
        {
            color[0] += 120*state;
        }
    }else
    {
        state *= (-1);
        if (state>0.5)
        {
            if (state >= 1)
            {
                color[0]=0;
                color[1]=0;
                color[2]=255;
            }else
            {
                color[2] = 200;
                color[1] = 30+120*state;
            }
        }else
        {
            color[2] += 120*state;
        }
    }
    qDebug()<<"color:"<<color[0]<<","<<color[1]<<","<<color[2]<<endl;
}
void MainWindow::updateLogStateChange()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client->state())
                    + QLatin1Char('\n');
    qDebug()<<content<<endl;
}
void MainWindow::brokerDisconnected()
{
    qDebug()<<"DisConnect"<<endl;
}

