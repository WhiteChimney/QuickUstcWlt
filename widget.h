#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QSystemTrayIcon>
#include <QMenu>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QTextCodec>

#include <QDebug>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;

private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *actionShowPref;
    QAction *actionExit;
    QAction *actionGetCurrentStatus;
    QMenu *menuChangeTunnel;
    QAction *actionTunnel1,
            *actionTunnel2,
            *actionTunnel3,
            *actionTunnel4,
            *actionTunnel5,
            *actionTunnel6,
            *actionTunnel7,
            *actionTunnel8,
            *actionTunnel9;

    QNetworkAccessManager* naManager;
    QNetworkRequest nRequest;
    QTextCodec* gb_code = QTextCodec::codecForName("gb2312");

    QString userName, userPassword;
    int currentTunnel = 0;
    QString timeExpire;

public:
    void fetchUiData();
    void getCurretStatus();
    void dealGetCurrentStatus(QNetworkReply*);
    void postRequest();

private slots:
    void on_buttonSet_clicked();
    void on_buttonClose_clicked();
};
#endif // WIDGET_H
