#include "widget.h"
#include "./ui_widget.h"

// 远程命令定义规则
// 格式：username&password&command
// 其中 command 为非负整数
// 0 用于查询通道，1-9 用于直接设定对应通道
// 其他的大于等于 10 的数字用于设置其他功能
// 第一位表示设置面板中的第几个 tab
// 如 1 表示登陆信息页，2 表示计划任务页
// 后面的位数自行定义功能
#define TCP_CHECKTUNNEL 0
#define TCP_STARTSCHEDULE 211
#define TCP_STOPSCHEDULE 212
#define TCP_STARTSCHEDULELOGIN 221

void Widget::on_checkBoxAllowRemote_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
        tcpServerState = startTcpServer();
    else
        stopTcpServer();
}

bool Widget::startTcpServer()
{
    if (tcpServerState)
        return true;

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(
                QHostAddress::AnyIPv4,
                ui->textLocalPort->text().toUInt()))
    {
        tcpServer->deleteLater();
        ui->labelServerInfo->setText(tr("错误：") + tcpServer->errorString());
        return false;
    }
    connect(tcpServer,&QTcpServer::newConnection,this,&Widget::dealServerNewConnection);
    ui->labelServerInfo->setText(tr("已开启，等待连接"));
    return true;
}

void Widget::stopTcpServer()
{
    if (tcpServerState)
    {
        disconnect(tcpServer,&QTcpServer::newConnection,this,&Widget::dealServerNewConnection);
        tcpServer->close();
        tcpServer->deleteLater();
        ui->labelServerInfo->setText(tr("未开启"));
        tcpServerState = false;
        ui->checkBoxAllowRemote->setChecked(false);
    }
}

void Widget::dealServerNewConnection()
{
    tcpSocketServer = tcpServer->nextPendingConnection();
    connect(tcpSocketServer,&QTcpSocket::readyRead,this,&Widget::dealServerRecvCmd);
    connect(tcpSocketServer,&QTcpSocket::disconnected,this,&Widget::dealServerDisconnected);
    ui->labelServerInfo->setText(tr("已连接"));
    serverSendMsg(tr("已连接"));
}

void Widget::dealServerDisconnected()
{
    if (tcpServerState)
        ui->labelServerInfo->setText(tr("已开启，等待连接"));
    else
        ui->labelServerInfo->setText(tr("未开启"));
}

void Widget::serverSendMsg(QString str)
{
    tcpSocketServer->write(str.toUtf8());
}

void Widget::dealServerRecvCmd()
{
    QString msg = tcpSocketServer->readAll();
    QString remoteUsername = msg.split(QLatin1Char('&')).first();
    QString remotePassword = msg.split(QLatin1Char('&')).at(1);
    int usernameMatch = QString::compare(remoteUsername,userName);
    int passwordMatch = QString::compare(remotePassword,password);
    if (usernameMatch != 0 or passwordMatch != 0)
    {
        serverSendMsg(tr("账户信息不匹配"));
        return;
    }

    int cmd = msg.split(QLatin1Char('&')).last().toInt();
    if (cmd < 0)
    {
        serverSendMsg(tr("不存在的指令！"));
        return;
    }
    if (cmd < 10)
    {
        if (cmd == TCP_CHECKTUNNEL)
        {
            disconnect(this,&Widget::tunnelUpdated,this,&Widget::dealRemoteCheckNet);
            connect(this,&Widget::tunnelUpdated,this,&Widget::dealRemoteCheckNet);
            naManager->checkNet();
        }
        else
        {
            naManager->setTunnel(cmd-1);
            serverSendMsg(tr("通道 ") + QString::number(cmd) + tr("，国际"));
        }
    }
    else
    {
        int currentTabIndex = cmd;
        while (currentTabIndex >= 10)
            currentTabIndex = currentTabIndex / 10;
        if (currentTabIndex < 5)
            ui->tabWidget->setCurrentIndex(currentTabIndex-1);
        switch (cmd) {
        case TCP_STARTSCHEDULE:
            ui->checkboxEnableScheduledCheckNet->setChecked(true);
            serverSendMsg(tr("已打开定时查询功能"));
            break;
        case TCP_STOPSCHEDULE:
            ui->checkboxEnableScheduledLogin->setChecked(false);
            ui->checkboxEnableScheduledCheckNet->setChecked(false);
            serverSendMsg(tr("已关闭定时查询功能"));
            break;
        case TCP_STARTSCHEDULELOGIN:
            ui->checkboxEnableScheduledCheckNet->setChecked(true);
            ui->checkboxEnableScheduledLogin->setChecked(true);
            serverSendMsg(tr("已打开定时查询并登陆功能"));
            break;
        default:
            serverSendMsg(tr("不存在的指令！"));
            break;
        }
        this->saveToIni();
    }
}

void Widget::dealRemoteCheckNet()
{
    QString msg;
    if (networkAccess)
    {
        msg = tr("通道 ") + QString::number(currentTunnel+1);
        msg += tr("，国际");
    }
    else
    {
        msg = tr("通道 ") + QString::number(currentTunnel-1958+1);
        msg += tr("，校内");
    }
    serverSendMsg(msg);
}

void Widget::on_textLocalPort_textChanged(const QString &arg1)
{
    ui->checkBoxAllowRemote->setChecked(false);
}

void Widget::on_textRemotePort_textChanged(const QString &arg1)
{
    if (tcpClientConnectState)
        on_buttonStopConnect_clicked();
}

void Widget::on_buttonStartConnect_clicked()
{
    if (tcpClientConnectState)
        return;

    tcpSocketClient = new QTcpSocket(this);
    connect(tcpSocketClient,&QTcpSocket::connected,this,[=](){tcpClientConnectState = true;});
    connect(tcpSocketClient,&QTcpSocket::readyRead,this,&Widget::dealClientRecvMsg);
    connect(tcpSocketClient,&QTcpSocket::disconnected,this,&Widget::on_buttonStopConnect_clicked);
    tcpSocketClient->connectToHost(
                ui->textRemoteIP->text(),
                ui->textRemotePort->text().toUInt());
}

void Widget::on_buttonStopConnect_clicked()
{
    if (tcpClientConnectState)
    {
        disconnect(tcpSocketClient,&QTcpSocket::readyRead,this,&Widget::dealClientRecvMsg);
        tcpSocketClient->close();
        tcpSocketClient->deleteLater();
        ui->labelServerMsg->setText(tr("尚未连接"));
        tcpClientConnectState = false;
    }
}

void Widget::dealClientRecvMsg()
{
    tcpClientConnectState = true;
    ui->labelServerMsg->setText(tcpSocketClient->readAll());
}

void Widget::clientSendMsg(QString str)
{
    if (tcpClientConnectState)
        tcpSocketClient->write(str.toUtf8());
}

void Widget::on_buttonRemoteLogin_clicked()
{
    clientSendMsg(userName + "&" + password + "&" +
                  QString::number(ui->comboRemoteTunnel->currentIndex()+1));
}

void Widget::on_buttonCheckRemoteNet_clicked()
{
    clientSendMsg(userName + "&" + password + "&" +
                  QString::number(TCP_CHECKTUNNEL));
}

void Widget::on_buttonTcpStartScheduleCheck_clicked()
{
    clientSendMsg(userName + "&" + password + "&" +
                  QString::number(TCP_STARTSCHEDULE));
}

void Widget::on_buttonTcpStopScheduleCheck_clicked()
{
    clientSendMsg(userName + "&" + password + "&" +
                  QString::number(TCP_STOPSCHEDULE));
}

void Widget::on_buttonTcpStartScheduleLogin_clicked()
{
    clientSendMsg(userName + "&" + password + "&" +
                  QString::number(TCP_STARTSCHEDULELOGIN));
}

void Widget::closeTcpService()
{
    if (tcpClientConnectState)
    {
        delete tcpSocketClient;
        delete tcpSocketServer;
    }
}
