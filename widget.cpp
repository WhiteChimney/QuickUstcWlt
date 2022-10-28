#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

//    设置菜单目录
    trayMenu = new QMenu(this);

    trayMenu->addSeparator();
    actionExit = new QAction(tr("退出"),this);      trayMenu->addAction(actionExit);
    actionShowPref = new QAction(tr("首选项"),this); trayMenu->addAction(actionShowPref);
    actionGetCurrentStatus = new QAction(tr("获取当前网络状态"),this);
    trayMenu->addAction(actionGetCurrentStatus);
    connect(actionExit, &QAction::triggered, qApp, &QApplication::quit);
    connect(actionShowPref, &QAction::triggered, this, &Widget::show);
    connect(actionGetCurrentStatus, &QAction::triggered, this, &Widget::getCurretStatus);

    trayMenu->addSeparator();
    menuChangeTunnel = trayMenu->addMenu(tr("设置通道"));
    actionTunnel1 = new QAction(tr("通道1"),this); menuChangeTunnel->addAction(actionTunnel1);
    actionTunnel2 = new QAction(tr("通道2"),this); menuChangeTunnel->addAction(actionTunnel2);
    actionTunnel3 = new QAction(tr("通道3"),this); menuChangeTunnel->addAction(actionTunnel3);
    actionTunnel4 = new QAction(tr("通道4"),this); menuChangeTunnel->addAction(actionTunnel4);
    actionTunnel5 = new QAction(tr("通道5"),this); menuChangeTunnel->addAction(actionTunnel5);
    actionTunnel6 = new QAction(tr("通道6"),this); menuChangeTunnel->addAction(actionTunnel6);
    actionTunnel7 = new QAction(tr("通道7"),this); menuChangeTunnel->addAction(actionTunnel7);
    actionTunnel8 = new QAction(tr("通道8"),this); menuChangeTunnel->addAction(actionTunnel8);
    actionTunnel9 = new QAction(tr("通道9"),this); menuChangeTunnel->addAction(actionTunnel9);
    connect(actionTunnel1, &QAction::triggered, this, [&](){currentTunnel = 0;});
    connect(actionTunnel1, &QAction::triggered, this, &Widget::postRequest);
    connect(actionTunnel2, &QAction::triggered, this, [&](){currentTunnel = 1;});
    connect(actionTunnel2, &QAction::triggered, this, &Widget::postRequest);
    connect(actionTunnel3, &QAction::triggered, this, [&](){currentTunnel = 2;});
    connect(actionTunnel3, &QAction::triggered, this, &Widget::postRequest);
    connect(actionTunnel4, &QAction::triggered, this, [&](){currentTunnel = 3;});
    connect(actionTunnel4, &QAction::triggered, this, &Widget::postRequest);
    connect(actionTunnel5, &QAction::triggered, this, [&](){currentTunnel = 4;});
    connect(actionTunnel5, &QAction::triggered, this, &Widget::postRequest);
    connect(actionTunnel6, &QAction::triggered, this, [&](){currentTunnel = 5;});
    connect(actionTunnel6, &QAction::triggered, this, &Widget::postRequest);
    connect(actionTunnel7, &QAction::triggered, this, [&](){currentTunnel = 6;});
    connect(actionTunnel7, &QAction::triggered, this, &Widget::postRequest);
    connect(actionTunnel8, &QAction::triggered, this, [&](){currentTunnel = 7;});
    connect(actionTunnel8, &QAction::triggered, this, &Widget::postRequest);
    connect(actionTunnel9, &QAction::triggered, this, [&](){currentTunnel = 8;});
    connect(actionTunnel9, &QAction::triggered, this, &Widget::postRequest);

//    设置系统托盘
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon("/Users/noland/Documents/QtProjects/QuickUstcWlt/WLT_logo.png"));
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

//    设置网络
    naManager = new QNetworkAccessManager(this);

    this->fetchUiData();

}

Widget::~Widget()
{
    delete ui;
}

void Widget::fetchUiData()
{
    userName = ui->textUserName->text();
    userPassword = ui->textUserPassword->text();
    currentTunnel = ui->comboTunnel->currentIndex();
    timeExpire = ui->textTimeExpire->text();
}

void Widget::getCurretStatus()
{
    connect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealGetCurrentStatus);
    qDebug() << "current tunnel is " << currentTunnel;
    nRequest.setUrl(QUrl("http://wlt.ustc.edu.cn/cgi-bin/ip/?name=xhzhan&password=147913&cmd=login"));
    naManager->get(nRequest);
}

void Widget::dealGetCurrentStatus(QNetworkReply* nReply)
{
    if (nReply->error())
    {
        qDebug() << tr("网络出错了");
        qDebug() << nReply->errorString();
    }
    else
    {
        qDebug() << gb_code->toUnicode(nReply->readAll());
    }
    disconnect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealGetCurrentStatus);
}

void Widget::postRequest()
{
    connect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealGetCurrentStatus);
    qDebug() << "current tunnel is " << currentTunnel;
    nRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                       + QString("&password=") + userPassword
                       + QString("&cmd=set&type=") + QString::number(currentTunnel)
                       + QString("&exp=") + timeExpire));
    naManager->get(nRequest);
}


void Widget::on_buttonSet_clicked()
{
    this->fetchUiData();
}

