#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QMessageBox>
#include <QListView>
#include <QThread>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QTextCodec>

#include <QStandardPaths>
#include <QFileInfo>
#include <QSettings>
#include <QDir>

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
    QAction *actionLogOut;
    QMenu *menuChangeTunnel;
    QVector<QAction*> vActionTunnel;
    QAction *actionQuickLogIn;

    QNetworkAccessManager *loginManager, *logoutManager, *setManager;
    QNetworkRequest loginRequest, logoutRequest, setRequest;
    QTextCodec* gb_code = QTextCodec::codecForName("gb2312");
    QTextCodec* utf8_code = QTextCodec::codecForName("UTF-8");

    QString userName, userPassword;
    int currentTunnel = -1, defaultTunnel;
    bool access = false; // 权限
    int timeExpireIndex;
    QString timeExpire;

    bool enableAutoLogin, enableRunAtStartup, enableTimedLogIn;
    QString timedLogIn;
    QTimer* logInTimer;

    bool enableAutoCheckNet;
    QString timedCheckNet;
    QTimer* timerCheckNet;

private:
    QString iniPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString iniName = iniPath + "/Preferences.ini";
    QFile *fSingleCount = new QFile();
    QTextStream fStream;
public:
    void saveToIni();
    // 保存当前配置到配置文件
    void loadFromIni();
    // 从配置文件加载配置
    int key = 1958;
    QByteArray simpleEncryption(QByteArray password, int key);
    QByteArray simpleDecryption(QByteArray password, int key);

public:
    void setupTrayMenu();
    void dealTrayIconActivated(QSystemTrayIcon::ActivationReason);
    void setCheckedTunnel(int checkedTunnel);
    void fetchUiData(), pushUiData();
    void logInWlt();        void dealLogInWlt(QNetworkReply*);
    void logOutWlt();
    void setTunnel(int tunnel); void dealSetTunnel(QNetworkReply*);
    void getCurrentStatus(QString* answer);
    void setRunAtStartUp(const QString &appPath);
    void unsetRunAtStartUp(const QString &appPath);

private slots:
    void on_buttonSet_clicked();
    void on_buttonClose_clicked();
    void on_checkboxEnableTimedLogIn_stateChanged(int);
    void on_checkboxTimedCheckNet_stateChanged(int);
    void on_comboTunnel_activated(int index);
    void on_buttonHelp_clicked();
};
#endif // WIDGET_H
