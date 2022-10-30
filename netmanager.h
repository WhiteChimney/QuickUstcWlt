#ifndef NETMANAGER_H
#define NETMANAGER_H

#include <QObject>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QTextCodec>

class NetManager : public QObject
{
    Q_OBJECT

public:
    NetManager
    (QObject *parent = nullptr,
     QString userName = "name", QString userPassword = "1234",
     int tunnel = 0, QString expireTime = "0");

public:
    void updateData(QString userName, QString userPassword, int tunnel, QString expireTime);
    void checkNet();
    void logoutWlt();
    void setTunnel(int tunnel);
    int getCurrentTunnel(QString* answer);

protected slots:
    void dealLoginWlt(QNetworkReply*);

signals:
    int returnTunnel(int currentTunnel);

private:
    QNetworkAccessManager *loginManager, *logoutManager, *setManager;
    QNetworkRequest loginRequest, logoutRequest, setRequest;
    QTextCodec* gb_code = QTextCodec::codecForName("gb2312");
    QTextCodec* utf8_code = QTextCodec::codecForName("UTF-8");

private:
    QString userName;
    QString userPassword;
    int tunnel;
    QString expireTime;
};

#endif // NETMANAGER_H
