#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("首选项"));

//    设置菜单目录
    this->setupTrayMenu();

//    设置系统托盘
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/images/WLT_logo.png"));
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

//    设置网络
    naManager = new QNetworkAccessManager(this);

//    设置时钟
    logInTimer = new QTimer(this);
    connect(logInTimer, &QTimer::timeout, this, &Widget::quickSetTunnel);

//    检查是否存在配置文件（ini）
    QFileInfo iniInfo(iniName);
    if (iniInfo.isFile())
        loadFromIni();   // 存在配置则加载
    else
    {
        this->show();
        saveToIni();     // 否则保存当前配置
    }

    if (enableAutoLogin)
        this->quickSetTunnel();

}

Widget::~Widget()
{
    delete ui;
}

void Widget::setupTrayMenu()
{
    trayMenu = new QMenu(this);

    trayMenu->addSeparator();
    actionQuickLogIn = new QAction(tr("快捷登陆"),this);
    trayMenu->addAction(actionQuickLogIn);
    connect(actionQuickLogIn, &QAction::triggered, this, &Widget::quickSetTunnel);
    menuChangeTunnel = trayMenu->addMenu(tr("切换通道"));
    actionTunnel1 = new QAction(tr("1教育网出口(国际,仅用教育网访问,适合看文献)"),this); menuChangeTunnel->addAction(actionTunnel1);
    actionTunnel2 = new QAction(tr("2电信网出口(国际,到教育网走教育网)"),this); menuChangeTunnel->addAction(actionTunnel2);
    actionTunnel3 = new QAction(tr("3联通网出口(国际,到教育网走教育网)"),this); menuChangeTunnel->addAction(actionTunnel3);
    actionTunnel4 = new QAction(tr("4电信网出口2(国际,到教育网免费地址走教育网)"),this); menuChangeTunnel->addAction(actionTunnel4);
    actionTunnel5 = new QAction(tr("5联通网出口2(国际,到教育网免费地址走教育网)"),this); menuChangeTunnel->addAction(actionTunnel5);
    actionTunnel6 = new QAction(tr("6电信网出口3(国际,默认电信,其他分流)"),this); menuChangeTunnel->addAction(actionTunnel6);
    actionTunnel7 = new QAction(tr("7联通网出口3(国际,默认联通,其他分流)"),this); menuChangeTunnel->addAction(actionTunnel7);
    actionTunnel8 = new QAction(tr("8教育网出口2(国际,默认教育网,其他分流)"),this); menuChangeTunnel->addAction(actionTunnel8);
    actionTunnel9 = new QAction(tr("9移动网出口(国际,无P2P或带宽限制)"),this); menuChangeTunnel->addAction(actionTunnel9);
    connect(actionTunnel1, &QAction::triggered, this, [&](){currentTunnel = 0;});
    connect(actionTunnel1, &QAction::triggered, this, &Widget::setTunnel);
    connect(actionTunnel2, &QAction::triggered, this, [&](){currentTunnel = 1;});
    connect(actionTunnel2, &QAction::triggered, this, &Widget::setTunnel);
    connect(actionTunnel3, &QAction::triggered, this, [&](){currentTunnel = 2;});
    connect(actionTunnel3, &QAction::triggered, this, &Widget::setTunnel);
    connect(actionTunnel4, &QAction::triggered, this, [&](){currentTunnel = 3;});
    connect(actionTunnel4, &QAction::triggered, this, &Widget::setTunnel);
    connect(actionTunnel5, &QAction::triggered, this, [&](){currentTunnel = 4;});
    connect(actionTunnel5, &QAction::triggered, this, &Widget::setTunnel);
    connect(actionTunnel6, &QAction::triggered, this, [&](){currentTunnel = 5;});
    connect(actionTunnel6, &QAction::triggered, this, &Widget::setTunnel);
    connect(actionTunnel7, &QAction::triggered, this, [&](){currentTunnel = 6;});
    connect(actionTunnel7, &QAction::triggered, this, &Widget::setTunnel);
    connect(actionTunnel8, &QAction::triggered, this, [&](){currentTunnel = 7;});
    connect(actionTunnel8, &QAction::triggered, this, &Widget::setTunnel);
    connect(actionTunnel9, &QAction::triggered, this, [&](){currentTunnel = 8;});
    connect(actionTunnel9, &QAction::triggered, this, &Widget::setTunnel);

    trayMenu->addSeparator();
    actionGetCurrentStatus = new QAction(tr("获取当前网络状态"),this);
    trayMenu->addAction(actionGetCurrentStatus);
    connect(actionGetCurrentStatus, &QAction::triggered, this, &Widget::logInWlt);
    actionLogOut = new QAction(tr("退出登陆网络通"),this);
    trayMenu->addAction(actionLogOut);
    connect(actionLogOut, &QAction::triggered, this, &Widget::logOutWlt);

    trayMenu->addSeparator();
    actionShowPref = new QAction(tr("首选项"),this); trayMenu->addAction(actionShowPref);
    actionExit = new QAction(tr("退出"),this);      trayMenu->addAction(actionExit);
    connect(actionShowPref, &QAction::triggered, this, &Widget::show);
    connect(actionExit, &QAction::triggered, qApp, &QApplication::quit);
}

void Widget::fetchUiData()
{
    userName = ui->textUserName->text();
    userPassword = ui->textUserPassword->text();
    defaultTunnel = ui->comboTunnel->currentIndex();
    timeExpireIndex = ui->comboExpireTime->currentIndex();
    switch (timeExpireIndex) {
    case 0:
        timeExpire = "0";
        break;
    case 1:
        timeExpire = "3600";
        break;
    case 2:
        timeExpire = "14400";
        break;
    case 3:
        timeExpire = "39600";
        break;
    case 4:
        timeExpire = "50400";
        break;
    default:
        break;
    }
    enableAutoLogin = ui->checkboxEnableAutoLogIn->isChecked();
    enableRunAtStartup = ui->checkboxEnableRunAtStartup->isChecked();
    enableTimedLogIn = ui->checkboxEnableTimedLogIn->isChecked();
    timedLogIn = ui->textTimedLogIn->text();
}

void Widget::pushUiData()
{
    ui->textUserName->setText(userName);
    ui->textUserPassword->setText(userPassword);
    ui->comboTunnel->setCurrentIndex(defaultTunnel);
    ui->comboExpireTime->setCurrentIndex(timeExpireIndex);
    ui->checkboxEnableAutoLogIn->setChecked(enableAutoLogin);
    ui->checkboxEnableRunAtStartup->setChecked(enableRunAtStartup);
    ui->checkboxEnableTimedLogIn->setChecked(enableTimedLogIn);
    ui->textTimedLogIn->setText(timedLogIn);
    ui->textTimedLogIn->setEnabled(enableTimedLogIn);
}

void Widget::logInWlt()
{
    connect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealLogInWlt);
    nRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                                                + QString("&password=") + userPassword
                                                + QString("&cmd=login")));
    naManager->get(nRequest);
}

void Widget::dealLogInWlt(QNetworkReply* nReply)
{
    QString answer = gb_code->toUnicode(nReply->readAll());
    this->getCurrentStatus(&answer);
    disconnect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealLogInWlt);
}

void Widget::getCurrentStatus(QString* answer)
{
    currentTunnel = answer->mid(answer->indexOf(QString("状态:<br>\n出口:"))+12,1).toInt()-1;
    if (currentTunnel < 0 or currentTunnel > 9)
    {
        QMessageBox::warning(this,
                               QString("登陆失败"),
                               QString("请检查用户名与密码"),
                               QMessageBox::Ok);
        currentTunnel = defaultTunnel;
    }
    else
    {
        if (answer->mid(answer->indexOf(QString("<br>\n权限"))+9,2).indexOf("校内"))
        {
            access = true;
                trayIcon->setIcon(QIcon(QString(":/images/WLT_logo_") + QString::number(currentTunnel) + QString(".png")));
        }
        else
        {
            access = false;
            trayIcon->setIcon(QIcon(QString(":/images/WLT_logo_none.png")));
        }
    }
}

void Widget::logOutWlt()
{
    connect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealLogOutWlt);
    nRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?cmd=logout")));
    naManager->get(nRequest);
}

void Widget::dealLogOutWlt(QNetworkReply* nReply)
{
    this->logInWlt();
    disconnect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealLogOutWlt);
}

void Widget::setTunnel()
{
    connect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealSetTunnel);
    nRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                       + QString("&password=") + userPassword
                       + QString("&cmd=set&type=") + QString::number(currentTunnel)
                       + QString("&exp=") + timeExpire));
    naManager->get(nRequest);
}

void Widget::dealSetTunnel(QNetworkReply* nReply)
{
    QString answer = gb_code->toUnicode(nReply->readAll());
    this->getCurrentStatus(&answer);
    disconnect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealSetTunnel);
}
void Widget::quickSetTunnel()
{
    fetchUiData();
    connect(naManager, &QNetworkAccessManager::finished, this, &Widget::dealSetTunnel);
    nRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                       + QString("&password=") + userPassword
                       + QString("&cmd=set&type=") + QString::number(defaultTunnel)
                       + QString("&exp=") + timeExpire));
    naManager->get(nRequest);
}

void Widget::on_buttonSet_clicked()
{
    this->saveToIni();
    this->quickSetTunnel();

    if (enableTimedLogIn)
        logInTimer->start(1000*timedLogIn.toUInt());
    else
        logInTimer->stop();

    if (enableRunAtStartup)
    {
        qDebug() << "setting...";
        this->setRunAtStartUp(qApp->applicationFilePath());
    }
    else
        this->unsetRunAtStartUp(qApp->applicationFilePath());
}

void Widget::on_buttonClose_clicked()
{
    this->on_buttonSet_clicked();
    this->hide();
}

void Widget::saveToIni()
{
    fetchUiData();

    QSettings *iniSettings = new QSettings(iniName, QSettings::IniFormat);
    iniSettings->setValue("userName",userName);
    iniSettings->setValue("userPassword",this->simpleEncryption(userPassword.toLocal8Bit().data(),key));
    iniSettings->setValue("defaultTunnel",defaultTunnel);
    iniSettings->setValue("timeExpireIndex",timeExpireIndex);
    iniSettings->setValue("enableAutoLogin",enableAutoLogin);
    iniSettings->setValue("enableRunAtStartup",enableRunAtStartup);
    iniSettings->setValue("enableTimedLogIn",enableTimedLogIn);
    iniSettings->setValue("timedLogIn",timedLogIn);
    delete iniSettings;
}

void Widget::loadFromIni()
{
    QSettings *iniSettings = new QSettings(iniName, QSettings::IniFormat);
    userName = iniSettings->value("userName").toString();
    userPassword = QString(this->simpleDecryption(iniSettings->value("userPassword").toByteArray(),key));
    qDebug() << userPassword;
    defaultTunnel = iniSettings->value("defaultTunnel").toInt();
    timeExpireIndex = iniSettings->value("timeExpireIndex").toInt();
    enableAutoLogin = iniSettings->value("enableAutoLogin").toBool();
    enableRunAtStartup = iniSettings->value("enableRunAtStartup").toBool();
    enableTimedLogIn = iniSettings->value("enableTimedLogIn").toBool();
    timedLogIn = iniSettings->value("timedLogIn").toString();
    delete iniSettings;

    pushUiData();
}

void Widget::on_checkboxEnableTimedLogIn_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
        ui->textTimedLogIn->setEnabled(true);
    else
        ui->textTimedLogIn->setEnabled(false);
}

//设置程序自启动 appPath程序路径
void Widget::setRunAtStartUp(const QString &appPath)
{
    //注册表路径需要使用双反斜杠，如果是32位系统，要使用QSettings::Registry32Format
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::Registry64Format);

    //以程序名称作为注册表中的键
    //根据键获取对应的值（程序路径）
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();
    QString path = settings.value(name).toString();

    //如果注册表中的路径和当前程序路径不一样，
    //则表示没有设置自启动或自启动程序已经更换了路径
    //toNativeSeparators的意思是将"/"替换为"\"
    QString newPath = QDir::toNativeSeparators(appPath);
    qDebug() << "path = " << path;
    qDebug() << "new path = " << newPath;
    if (path != newPath)
        settings.setValue(name, newPath);
}

void Widget::unsetRunAtStartUp(const QString &appPath)
{
    //注册表路径需要使用双反斜杠，如果是32位系统，要使用QSettings::Registry32Format
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::Registry64Format);

    //以程序名称作为注册表中的键
    //根据键获取对应的值（程序路径）
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();
    QString path = settings.value(name).toString();

    //如果注册表中的路径和当前程序路径不一样，
    //则表示没有设置自启动或自启动程序已经更换了路径
    //toNativeSeparators的意思是将"/"替换为"\"
    QString newPath = QDir::toNativeSeparators(appPath);
    if (path == newPath)
    {
        settings.remove(name);
    }
}

QByteArray Widget::simpleEncryption(QByteArray password, int key)
{
    for (int i = 0; i < password.size(); i++)
    {
        password[i] += key;
    }
    return password;
}

QByteArray Widget::simpleDecryption(QByteArray password, int key)
{
    for (int i = 0; i < password.size(); i++)
    {
        password[i] -= key;
    }
    return password;
}
