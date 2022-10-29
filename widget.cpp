#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("首选项"));
    ui->comboTunnel->view()->setMinimumWidth(8*ui->comboTunnel->itemText(0).size());

//    设置菜单目录
    this->setupTrayMenu();

//    设置系统托盘
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/images/WLT_logo.png"));
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Widget::dealTrayIconActivated);
    trayIcon->show();

//    设置网络
    loginManager = new QNetworkAccessManager(this);
    connect(loginManager, &QNetworkAccessManager::finished, this, &Widget::dealLogInWlt);
    logoutManager = new QNetworkAccessManager(this);
    connect(logoutManager, &QNetworkAccessManager::finished, this, &Widget::logInWlt);
    setManager = new QNetworkAccessManager(this);
    connect(setManager, &QNetworkAccessManager::finished, this, &Widget::dealSetTunnel);

//    设置时钟
    logInTimer = new QTimer(this);
    connect(logInTimer, &QTimer::timeout, this, [=](){this->setTunnel(defaultTunnel);});
    timerCheckNet = new QTimer(this);
    connect(timerCheckNet, &QTimer::timeout, this, &Widget::logInWlt);

//    检查是否存在配置文件（ini）
    QFileInfo iniInfo(iniName);
    if (iniInfo.isFile())
        loadFromIni();   // 存在配置则加载
    else
    {
        this->show();
        on_buttonHelp_clicked();
        saveToIni();     // 否则保存当前配置
    }

    if (enableAutoLogin)
        this->setTunnel(defaultTunnel);
    else
        this->logInWlt();

}

Widget::~Widget()
{
    delete ui;
}

void Widget::setupTrayMenu()
{
    trayMenu = new QMenu(this);

    trayMenu->addSeparator();
    actionQuickLogIn = new QAction(tr("一键上网"),this);
    trayMenu->addAction(actionQuickLogIn);
    connect(actionQuickLogIn, &QAction::triggered, this, &Widget::setTunnel);
    menuChangeTunnel = trayMenu->addMenu(tr("登陆其他出口"));
    QAction* actionTunnel = new QAction(tr("1 教育网出口"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("2 电信网出口"),this);   vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("3 联通网出口"),this);   vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("4 电信网出口 2"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("5 联通网出口 2"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("6 电信网出口 3"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("7 联通网出口 3"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("8 教育网出口 2"),this); vActionTunnel.append(actionTunnel);
    actionTunnel = new QAction(tr("9 移动网出口"),this);   vActionTunnel.append(actionTunnel);
    for (int i = 0; i < 9; i++)
    {
        actionTunnel = vActionTunnel.at(i);
        menuChangeTunnel->addAction(actionTunnel);
        actionTunnel->setCheckable(true);
        connect(actionTunnel, &QAction::triggered, this, [=](){
            currentTunnel = i;
            this->setTunnel(i);
        });
    }

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

void Widget::setCheckedTunnel(int checkedTunnel)
{
    for (int j = 0; j < 9; j++)
    {
        if (j == checkedTunnel)
            vActionTunnel.at(j)->setChecked(true);
        else
            vActionTunnel.at(j)->setChecked(false);
    }

}

void Widget::dealTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:      // windows 左键单击
        this->show();
        break;
    case QSystemTrayIcon::Context:      // windows 右键单击
        break;
    case QSystemTrayIcon::DoubleClick:  // windows 左键双击
        break;
    case QSystemTrayIcon::MiddleClick:  // windows 中键单击
        this->logInWlt();
        break;
    default:
        break;
    }
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

    timedCheckNet = ui->textTimedCheckNet->text();
    if (timedCheckNet.toInt() < 3)
        timedCheckNet = QString("3");

    timedLogIn = ui->textTimedLogIn->text();
    if (timedLogIn.toInt() < 10)
        timedLogIn = QString("10");
    enableAutoCheckNet = ui->checkboxTimedCheckNet->isChecked();

    pushUiData();
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
    ui->checkboxTimedCheckNet->setChecked(enableAutoCheckNet);
    ui->textTimedCheckNet->setText(timedCheckNet);
    ui->textTimedCheckNet->setEnabled(enableAutoCheckNet);
}

void Widget::logInWlt()
{
    fetchUiData();
    loginRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                                                + QString("&password=") + userPassword
                                                + QString("&cmd=login")));
    loginManager->get(loginRequest);
}

void Widget::dealLogInWlt(QNetworkReply* nReply)
{
    QString answer = gb_code->toUnicode(nReply->readAll());
    this->getCurrentStatus(&answer);
}

void Widget::getCurrentStatus(QString* answer)
{
    currentTunnel = answer->mid(answer->indexOf(QString("状态:<br>\n出口:"))+12,1).toInt()-1;
    if (currentTunnel < 0 or currentTunnel > 9)
    {
        if (timerCheckNet->isActive())
            timerCheckNet->stop();
        if (logInTimer->isActive())
            logInTimer->stop();
        ui->checkboxTimedCheckNet->setChecked(false);
        ui->checkboxEnableTimedLogIn->setChecked(false);
        currentTunnel = defaultTunnel;
        QMessageBox::warning(this,
                               QString("登陆失败"),
                               QString("请检查用户名与密码"),
                               QMessageBox::Ok);
        this->show();
    }
    else
    {
        if (answer->mid(answer->indexOf(QString("<br>\n权限"))+9,2).indexOf("校内"))
        {
            access = true;
            trayIcon->setIcon(QIcon(QString(":/images/WLT_logo_") + QString::number(currentTunnel) + QString(".png")));
            this->setCheckedTunnel(currentTunnel);
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
    logoutRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?cmd=logout")));
    logoutManager->get(logoutRequest);
}


void Widget::setTunnel(int tunnel)
{
    setRequest.setUrl(QUrl(QString("http://wlt.ustc.edu.cn/cgi-bin/ip?name=" + userName)
                       + QString("&password=") + userPassword
                       + QString("&cmd=set&type=") + QString::number(tunnel)
                       + QString("&exp=") + timeExpire));
    setManager->get(setRequest);
}

void Widget::dealSetTunnel(QNetworkReply* nReply)
{
    QString answer = gb_code->toUnicode(nReply->readAll());
    this->getCurrentStatus(&answer);
}

void Widget::on_buttonSet_clicked()
{
    this->saveToIni();
    this->setTunnel(defaultTunnel);

    if (enableAutoCheckNet)
        timerCheckNet->start(1000*timedCheckNet.toInt()+0.17);
    else
        timerCheckNet->stop();

    if (enableTimedLogIn)
        logInTimer->start(1000*timedLogIn.toInt()+0.59);
    else
        logInTimer->stop();

    if (enableRunAtStartup)
        this->setRunAtStartUp(qApp->applicationFilePath());
    else
        this->unsetRunAtStartUp(qApp->applicationFilePath());

    ui->buttonSet->setText(tr("确定 √"));
    QTimer* tempTimer = new QTimer(this);
    connect(tempTimer, &QTimer::timeout, this, [=](){
        ui->buttonSet->setText(tr("确定"));
        tempTimer->stop();
        delete tempTimer;
    });
    tempTimer->start(250);
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
    iniSettings->setValue("enableAutoCheckNet",enableAutoCheckNet);
    iniSettings->setValue("timedCheckNet",timedCheckNet);
    delete iniSettings;
}

void Widget::loadFromIni()
{
    QSettings *iniSettings = new QSettings(iniName, QSettings::IniFormat);
    userName = iniSettings->value("userName").toString();
    userPassword = QString(this->simpleDecryption(iniSettings->value("userPassword").toByteArray(),key));
    defaultTunnel = iniSettings->value("defaultTunnel").toInt();
    timeExpireIndex = iniSettings->value("timeExpireIndex").toInt();
    enableAutoLogin = iniSettings->value("enableAutoLogin").toBool();
    enableRunAtStartup = iniSettings->value("enableRunAtStartup").toBool();
    enableTimedLogIn = iniSettings->value("enableTimedLogIn").toBool();
    timedLogIn = iniSettings->value("timedLogIn").toString();
    enableAutoCheckNet = iniSettings->value("enableAutoCheckNet").toBool();
    timedCheckNet = iniSettings->value("timedCheckNet").toString();
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
        settings.remove(name);
}

QByteArray Widget::simpleEncryption(QByteArray password, int key)
{
    for (int i = 0; i < password.size(); i++)
        password[i] += key;
    return password;
}

QByteArray Widget::simpleDecryption(QByteArray password, int key)
{
    for (int i = 0; i < password.size(); i++)
        password[i] -= key;
    return password;
}

void Widget::on_checkboxTimedCheckNet_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
        ui->textTimedCheckNet->setEnabled(true);
    else
        ui->textTimedCheckNet->setEnabled(false);
}


void Widget::on_comboTunnel_activated(int index)
{
    ui->comboTunnel->view()->setMinimumWidth(1000);
}


void Widget::on_buttonHelp_clicked()
{
    QString helpMessage = "";
    helpMessage += tr("这是一个简易的网络通登陆及状态查询软件\n");
    helpMessage += tr("出口状态可以直接通过「系统托盘图标」看到\n\n");
    helpMessage += tr("第一次使用时会自动弹出首选项界面\n");
    helpMessage += tr("在该界面设置完「用户名」与「密码」后点击确定即可保存信息\n\n");
    helpMessage += tr("「定时查询状态」指每隔指定时间就查询一下网络通出口及权限状态\n");
    helpMessage += tr("「定时登陆」指每隔指定时间就使用默认设置登陆一下网络通\n");
    helpMessage += tr("*注意如果间隔时间太短会触发网络通限制\n");
    helpMessage += tr("*即不能在 2 分钟内进行超过 20 次操作\n\n");
    helpMessage += tr("托盘图标上「左键」可弹出首选项界面\n");
    helpMessage += tr("「中键」可直接查询网络通状态\n");
    helpMessage += tr("「右键」可进行更多操作\n");
    helpMessage += tr("其中「一键上网」指使用默认设置登陆网络通\n");
    QMessageBox::about(this,tr("操作说明"),helpMessage);
}

