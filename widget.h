#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QSysInfo>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QMessageBox>
#include <QListView>
#include <QProcess>
#include <QDesktopServices>

#include <QStandardPaths>
#include <QFileInfo>
#include <QSettings>
#include <QDir>

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

#include <QDebug>

#include "netmanager.h"

#include "projectInfo.h"

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
    void setupUI();

//    网络通首选项
private:
    QString userName, password;
    int defaultTunnel, currentTunnel;
    bool networkAccess = false;
    int expireTimeIndex; QString expireTime;
    bool enableScheduledCheckNet;
    QString scheduledCheckNetTime;
    QTimer *scheduledCheckNetTimer;
    bool enableScheduledLogin;
    int scheduledLoginStyle;
    bool enableIgnoreWarning;
    bool enableAutoLogin, enableRunAtStartup;
    QVector<QCheckBox*> vCheckboxWeek;
    QDate *currentDate;
    QTime *currentTime;
    NetManager *naManager;
    QMessageBox *msgboxLoginFailed;
    QMessageBox *msgboxAbout;

public:
    void setRunAtStartup(bool setEnable);
public slots:
    void getCurrentTunnel(int tunnel);
signals:
    void scheduledCheckNetTunnelReturned(int tunnel);
public slots:
    void dealScheduledCheckNetTunnelReturned(int tunnel);
    void dealScheduledCheckNetTimerTimeout();

//    系统托盘菜单项
private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *actionQuickLogIn;
    QMenu *menuChangeTunnel;
    QVector<QAction*> vActionTunnel;
    QAction *actionGetCurrentStatus;
    QAction *actionLogOut;
    QAction *actionShowPref;
    QAction *actionExit;

public:
    void setupTrayMenu();
    void setCheckedTunnel(int checkedTunnel);
public slots:
    void dealTrayIconActivated(QSystemTrayIcon::ActivationReason);

//    配置文件
private:
    QString iniPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString iniName = iniPath + "/Preferences.ini";
public:
    void fetchUiData(), pushUiData();
    void saveToIni();    // 保存当前配置到配置文件
    void loadFromIni();    // 从配置文件加载配置

//    简单的密码加密
private:
    int key = 1958;
public:
    QByteArray passwordEncryption(QByteArray password, int key);
    QByteArray passwordDecryption(QByteArray password, int key);

//    远程登陆
private:
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocketServer;
    QTcpSocket *tcpSocketClient;
    bool tcpServerState = false;
    bool tcpClientConnectState = false;

signals:
    void tunnelUpdated();

public slots:
    void dealServerNewConnection();
    void dealServerDisconnected();
    void dealServerRecvCmd();
    void dealClientRecvMsg();
    void dealRemoteCheckNet();

public:
    bool startTcpServer();
    void stopTcpServer();
    void serverSendMsg(QString);
    void clientSendMsg(QString);
    void closeTcpService();

//    其他操作
private slots:
    void on_buttonSet_clicked();
    void on_buttonClose_clicked();
    void on_buttonHelp_clicked();
    void on_checkboxEnableScheduledCheckNet_stateChanged(int checkState);
    void on_textScheduledCheckNetTime_editingFinished();
    void on_checkboxEnableScheduledLogin_stateChanged(int checkState);
    void on_checkBoxScheduledTimeRange_stateChanged(int checkState);
    void on_timeStartTask_userTimeChanged(const QTime &time);
    void on_buttonIniPath_clicked();
    void on_checkboxEnableRunAtStartup_stateChanged(int checkState);
    void on_checkBoxAllowRemote_stateChanged(int arg1);
    void on_buttonStartConnect_clicked();
    void on_buttonStopConnect_clicked();
    void on_buttonRemoteLogin_clicked();
    void on_buttonCheckRemoteNet_clicked();
    void on_textLocalPort_textChanged(const QString &arg1);
    void on_textRemotePort_textChanged(const QString &arg1);
    void on_buttonTcpStartScheduleCheck_clicked();
    void on_buttonTcpStopScheduleCheck_clicked();
    void on_buttonTcpStartScheduleLogin_clicked();
    void on_buttonExit_clicked();
};
#endif // WIDGET_H
