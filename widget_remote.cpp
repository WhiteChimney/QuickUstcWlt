#include "widget.h"
#include "./ui_widget.h"

#define TCP_CHECKTUNNEL 0
#define TCP_QUICKLOGIN 1

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
    connect(tcpSocketServer,&QTcpSocket::disconnected,this,[=](){
        ui->labelServerInfo->setText(tr("已开启，等待连接"));
    });
    ui->labelServerInfo->setText(tr("已连接"));
    this->serverSendMsg(tr("已连接"));
}

void Widget::serverSendMsg(QString str)
{
    tcpSocketServer->write(str.toUtf8());
}

void Widget::dealServerRecvCmd()
{
    int cmd = tcpSocketServer->readAll().toInt();
    switch (cmd) {
    case TCP_CHECKTUNNEL:
        naManager->checkNet();
        serverSendMsg(tr("当前通道为：") + QString::number(currentTunnel+1));
        break;
    case TCP_QUICKLOGIN:
        naManager->setTunnel(defaultTunnel);
        serverSendMsg(tr("当前通道为：") + QString::number(defaultTunnel+1));
        break;
    default:
        break;
    }
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

    connect(tcpSocketClient,&QTcpSocket::connected,this,[=](){tcpClientConnectState = true;});
    connect(tcpSocketClient,&QTcpSocket::readyRead,this,&Widget::dealClientRecvMsg);
    connect(tcpSocketClient,&QTcpSocket::disconnected,this,&Widget::on_buttonStopConnect_clicked);
    tcpSocketClient->connectToHost(
                ui->textRemoteIP->text(),
                ui->textRemotePort->text().toUInt());
}

void Widget::on_buttonStopConnect_clicked()
{
    disconnect(tcpSocketClient,&QTcpSocket::readyRead,this,&Widget::dealClientRecvMsg);
    tcpSocketClient->disconnectFromHost();
    ui->labelServerMsg->setText(tr("尚未连接"));
    tcpClientConnectState = false;
}

void Widget::dealClientRecvMsg()
{
    ui->labelServerMsg->setText(tcpSocketClient->readAll());
}

void Widget::clientSendMsg(QString str)
{
    if (tcpClientConnectState)
        tcpSocketClient->write(str.toUtf8());
}

void Widget::on_buttonRemoteLogin_clicked()
{
    clientSendMsg(QString::number(TCP_QUICKLOGIN));
}

void Widget::on_buttonCheckRemoteNet_clicked()
{
    clientSendMsg(QString::number(TCP_CHECKTUNNEL));
}
