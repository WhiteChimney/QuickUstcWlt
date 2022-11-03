#include "netmanager.h"

NetManager::NetManager
        (QObject *parent,
         QString userName, QString userPassword,
         int tunnel, QString expireTime)
        : QObject(parent)
{
    loginManager = new QNetworkAccessManager(this);
    connect(loginManager, &QNetworkAccessManager::finished, this, &NetManager::dealLoginWlt);
    logoutManager = new QNetworkAccessManager(this);
    connect(logoutManager, &QNetworkAccessManager::finished, this, [=](){this->checkNet();});
    setManager = new QNetworkAccessManager(this);
    connect(setManager, &QNetworkAccessManager::finished, this, &NetManager::dealLoginWlt);

    fAnswer = new QFile();
    fAnswer->setFileName(tmpFileName);
    fStream.setDevice(fAnswer);
}

NetManager::~NetManager()
{
    delete fAnswer;
}

void NetManager::updateData
        (QString m_userName, QString m_userPassword, int m_tunnel, QString m_expireTime)
{
    userName = m_userName;
    userPassword = m_userPassword;
    tunnel = m_tunnel;
    expireTime = m_expireTime;
}

void NetManager::checkNet()
{
    loginRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                                                + QString("&password=") + userPassword
                                                + QString("&cmd=login")));
    loginManager->get(loginRequest);
}

void NetManager::logoutWlt()
{
    logoutRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                              + QString("&password=") + userPassword
                              + QString("&cmd=logout")));
    logoutManager->get(logoutRequest);
}

void NetManager::setTunnel(int tunnel)
{
    setRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                       + QString("&password=") + userPassword
                       + QString("&cmd=set&type=") + QString::number(tunnel)
                       + QString("&exp=") + expireTime));
    setManager->get(setRequest);
}

void NetManager::dealLoginWlt(QNetworkReply* nReply)
{
    answer = gb_code->toUnicode(nReply->readAll());
    tunnel = this->getCurrentTunnel(&answer);
    emit returnTunnel(tunnel);
}

void NetManager::displayAnswer()
{
    fAnswer->open(QIODevice::WriteOnly | QIODevice::Truncate);
    fStream << answer.replace("gb2312","utf-8");
    fAnswer->close();
    qDebug() << QDesktopServices::openUrl(QUrl::fromLocalFile(tmpFileName));
}

int NetManager::getCurrentTunnel(QString* answer)
{
    int currentTunnel = answer->mid(answer->indexOf(QString("状态:<br>\n出口:"))+12,1).toInt()-1;
    if (currentTunnel < 0 or currentTunnel > 8)
    {
        if (answer->mid(answer->indexOf(QString("警告: 最近2分钟内的操作次数超过20,"))+20,5).indexOf("请稍候再试"))
            return -1;
        else
            return -1958;
    }
    else
        if (answer->mid(answer->indexOf(QString("<br>\n权限"))+9,2).indexOf("校内"))
            return currentTunnel;
        else
            return currentTunnel + 1958;
}
