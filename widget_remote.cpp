#include "widget.h"
#include "./ui_widget.h"

#define TCP_CHECKTUNNEL 0

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
    QString remoteUsername = msg.chopped(1);
    int userMatch = QString::compare(remoteUsername,userName);
    if (userMatch != 0)
    {
        serverSendMsg(tr("账户信息不匹配"));
        return;
    }

    int cmd = msg.last(1).toInt();
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
    clientSendMsg(userName +
                  QString::number(ui->comboRemoteTunnel->currentIndex()+1));
}

void Widget::on_buttonCheckRemoteNet_clicked()
{
    clientSendMsg(userName +
                  QString::number(TCP_CHECKTUNNEL));
}

void Widget::closeTcpService()
{
    if (tcpClientConnectState)
    {
        delete tcpSocketClient;
        delete tcpSocketServer;
    }
}
