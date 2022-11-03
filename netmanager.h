#ifndef NETMANAGER_H
#define NETMANAGER_H

#include <QObject>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QTextCodec>
#include <QStandardPaths>
#include <QFile>
#include <QDesktopServices>

class NetManager : public QObject
{
    Q_OBJECT

public:
    NetManager
    (QObject *parent = nullptr,
     QString userName = "name", QString userPassword = "1234",
     int tunnel = 0, QString expireTime = "0");
    ~NetManager();

public:
    void updateData(QString userName, QString userPassword, int tunnel, QString expireTime);
    void checkNet();
    void logoutWlt();
    void setTunnel(int tunnel);
    int getCurrentTunnel(QString* answer);
    void displayAnswer();

protected slots:
    void dealLoginWlt(QNetworkReply*);

signals:
    void returnTunnel(int currentTunnel);

private:
    QNetworkAccessManager *loginManager, *logoutManager, *setManager;
    QNetworkRequest loginRequest, logoutRequest, setRequest;
    QTextCodec* gb_code = QTextCodec::codecForName("gb2312");
    QTextCodec* utf8_code = QTextCodec::codecForName("UTF-8");
    QString answer;
    QString tmpPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString tmpFileName = tmpPath + "/answer.html";
    QFile *fAnswer;
    QTextStream fStream;

private:
    QString userName;
    QString userPassword;
    int tunnel;
    QString expireTime;
};

#endif // NETMANAGER_H
