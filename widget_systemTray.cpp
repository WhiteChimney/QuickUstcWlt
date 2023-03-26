#include "widget.h"
#include "./ui_widget.h"

void Widget::setupTrayMenu()
{
    trayMenu = new QMenu(this);

    trayMenu->addSeparator();

    actionQuickLogIn = new QAction(tr("一键上网"),this);
    trayMenu->addAction(actionQuickLogIn);
    connect(actionQuickLogIn, &QAction::triggered, naManager, [=](){naManager->setTunnel(defaultTunnel);});

    menuChangeTunnel = new QMenu(this);
    menuChangeTunnel = trayMenu->addMenu(tr("登陆其他出口"));
    QAction *actionTunnel = new QAction(tr("1 教育网出口"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("2 电信网出口"),this);   vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("3 联通网出口"),this);   vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("4 电信网出口 2"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("5 联通网出口 2"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("6 电信网出口 3"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("7 联通网出口 3"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("8 教育网出口 2"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("9 移动网出口"),this);   vActionTunnel.append(actionTunnel);
    for (int i = 0; i < 9; i++)
    {
        actionTunnel = vActionTunnel.at(i);
        menuChangeTunnel->addAction(actionTunnel);
        actionTunnel->setCheckable(true);
        connect(actionTunnel, &QAction::triggered, this, [=](){naManager->setTunnel(i);});
    }

    trayMenu->addSeparator();

    actionGetCurrentStatus = new QAction(tr("获取当前网络状态"),this);
    trayMenu->addAction(actionGetCurrentStatus);
    connect(actionGetCurrentStatus, &QAction::triggered, naManager, [=](){naManager->checkNet();});

    actionLogOut = new QAction(tr("退出登陆网络通"),this);
    trayMenu->addAction(actionLogOut);
    connect(actionLogOut, &QAction::triggered, naManager, [=](){naManager->logoutWlt();});

    trayMenu->addSeparator();

    actionShowPref = new QAction(tr("首选项"),this); trayMenu->addAction(actionShowPref);
    connect(actionShowPref, &QAction::triggered, this, &Widget::show);

    actionExit = new QAction(tr("退出"),this);      trayMenu->addAction(actionExit);
    connect(actionExit, &QAction::triggered, qApp, &QApplication::quit);
}

void Widget::dealTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:      // 左键单击
        if (QSysInfo::productType() != "macos")   // macos 下单击无效
            this->show();
        break;
    case QSystemTrayIcon::Context:      // 右键单击
        break;
    case QSystemTrayIcon::DoubleClick:  // 左键双击
        break;
    case QSystemTrayIcon::MiddleClick:  // 中键单击
        naManager->checkNet();
        break;
    default:
        break;
    }
}
