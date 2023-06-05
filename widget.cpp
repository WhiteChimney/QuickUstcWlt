#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
//    设置 UI
    ui->setupUi(this);
    this->setupUI();

//    设置任务
    naManager = new NetManager(this);
    connect(naManager, &NetManager::returnTunnel, this, &Widget::getCurrentTunnel);
    scheduledCheckNetTimer = new QTimer(this);
    connect(scheduledCheckNetTimer, &QTimer::timeout, this, &Widget::dealScheduledCheckNetTimerTimeout);
    connect(this, &Widget::scheduledCheckNetTunnelReturned, this, &Widget::dealScheduledCheckNetTunnelReturned);
    currentDate = new QDate();
    currentTime = new QTime();

//    检查是否存在配置文件
    QFileInfo iniInfo(iniName);
    if (iniInfo.isFile())
        loadFromIni();   // 存在配置则加载
    else
    {
        this->show();
        on_buttonHelp_clicked();
        saveToIni();     // 否则保存当前配置
    }

//    设置托盘菜单目录
    this->setupTrayMenu();

//    设置系统托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/images/WLT_logo.png"));
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Widget::dealTrayIconActivated);
    trayIcon->show();

    naManager->updateData(userName, password, defaultTunnel, expireTime);

//    远程
    if (ui->checkBoxAllowRemote->isChecked())
        tcpServerState = startTcpServer();
    else
        tcpServerState = false;

//    启动计划任务
    if (enableAutoLogin)
        on_buttonSet_clicked();
    else
        if (enableScheduledCheckNet)
        {
            dealScheduledCheckNetTimerTimeout();
            scheduledCheckNetTimer->start(1000*scheduledCheckNetTime.toInt()+0.17);
        }
}

Widget::~Widget()
{
    saveToIni();
    closeTcpService();
    delete ui;
}

void Widget::setupUI()
{
    this->setWindowTitle(tr("QuickUstcWlt V")
                       + tr(PROJECT_VERSION_0) + tr(".")
                       + tr(PROJECT_VERSION_1) + tr(".")
                       + tr(PROJECT_VERSION_2));
    ui->comboDefaultTunnel->view()->setMinimumWidth(350);
    ui->comboboxScheduledLoginStyle->view()->setMinimumWidth(200);
    for (int i = 0; i < 7; i++)
    {
        QCheckBox *checkBoxWeek = new QCheckBox(this);
        vCheckboxWeek.append(checkBoxWeek);
        ui->vLayoutWeek->insertWidget(i,checkBoxWeek);
    }
    vCheckboxWeek.at(0)->setText(tr("周一"));
    vCheckboxWeek.at(1)->setText(tr("周二"));
    vCheckboxWeek.at(2)->setText(tr("周三"));
    vCheckboxWeek.at(3)->setText(tr("周四"));
    vCheckboxWeek.at(4)->setText(tr("周五"));
    vCheckboxWeek.at(5)->setText(tr("周六"));
    vCheckboxWeek.at(6)->setText(tr("周日"));
    ui->timeStartTask->setTime(ui->timeStartTask->minimumTime());
    ui->timeEndTask->setTime(ui->timeEndTask->maximumTime());
    ui->tabWidget->setCurrentIndex(0);

    msgboxLoginFailed = new QMessageBox(this);
    msgboxLoginFailed->setIcon(QMessageBox::Warning);
    msgboxLoginFailed->setText(tr("登陆失败"));
    msgboxLoginFailed->setStandardButtons(QMessageBox::Help | QMessageBox::Yes);
    msgboxLoginFailed->button(QMessageBox::Help)->setText(tr("详细信息"));
    msgboxLoginFailed->button(QMessageBox::Yes)->setText(tr("确认"));
    msgboxLoginFailed->setDefaultButton(QMessageBox::Yes);

    msgboxAbout = new QMessageBox(this);
    msgboxAbout->setIcon(QMessageBox::Information);
    msgboxAbout->setText(tr("关于\n\n版本：V ")
                       + tr(PROJECT_VERSION_0) + tr(".")
                       + tr(PROJECT_VERSION_1) + tr(".")
                       + tr(PROJECT_VERSION_2) + tr("\n"));
    QString helpMessage = "";
    helpMessage += tr("这是一个简易的网络通登陆及状态查询软件\n");
    helpMessage += tr("出口状态可以直接通过「系统托盘图标」看到\n\n");
    helpMessage += tr("第一次使用时会自动弹出「首选项」界面\n");
    helpMessage += tr("「登陆信息」标签页设置网络通默认使用的登陆信息\n");
    helpMessage += tr("点击确认可以保存配置信息\n\n");
    helpMessage += tr("「计划任务」标签页可以设置定时网络通登陆与查询\n");
    helpMessage += tr("*注意如果间隔时间太短会触发网络通限制\n");
    helpMessage += tr("*官方指定不能在 2 分钟内进行超过 20 次操作\n");
    helpMessage += tr("*偶尔会出现的登陆失败情形可以通过选项进行忽略\n\n");
    helpMessage += tr("「其他」标签页可以设置启动登陆与开机自启\n\n");
    helpMessage += tr("「远程登陆」用于指定 IP 登陆网络通\n");
    helpMessage += tr("需要被控端勾选了允许远程登陆的选项\n");
    helpMessage += tr("且提前登陆了有效的网络通账号\n\n");
    helpMessage += tr("关于系统托盘图标\n");
    helpMessage += tr("「左键」可弹出首选项界面\n");
    helpMessage += tr("「中键」可直接查询网络通状态\n");
    helpMessage += tr("「右键」可进行更多操作\n");
    helpMessage += tr("「macOS 系统」因系统限制只能左键打开菜单\n");
    helpMessage += tr("菜单中「一键上网」指使用默认设置登陆网络通\n");
    QUrl myUrl = QUrl("https://github.com/WhiteChimney/QuickUstcWlt/releases");
    msgboxAbout->setInformativeText(helpMessage);
    msgboxAbout->setStandardButtons(QMessageBox::Help | QMessageBox::Yes);
    msgboxAbout->button(QMessageBox::Yes)->setText(tr("OK"));
    msgboxAbout->button(QMessageBox::Help)->setText(tr("检查更新"));
    msgboxAbout->button(QMessageBox::Help)->disconnect();
    connect(msgboxAbout->button(QMessageBox::Help), &QAbstractButton::clicked,
            this, [=](){QDesktopServices::openUrl(myUrl);});
    msgboxAbout->setDefaultButton(QMessageBox::Yes);
    msgboxAbout->setModal(false);
}

void Widget::fetchUiData()
{
    userName = ui->textUserName->text();
    password = ui->textPassword->text();
    defaultTunnel = ui->comboDefaultTunnel->currentIndex();
    expireTimeIndex = ui->comboExpireTime->currentIndex();
    switch (expireTimeIndex) {
    case 0:
        expireTime = "0";
        break;
    case 1:
        expireTime = "3600";
        break;
    case 2:
        expireTime = "14400";
        break;
    case 3:
        expireTime = "39600";
        break;
    case 4:
        expireTime = "50400";
        break;
    default:
        break;
    }

    enableScheduledCheckNet = ui->checkboxEnableScheduledCheckNet->isChecked();
    scheduledCheckNetTime = ui->textScheduledCheckNetTime->text();
    enableScheduledLogin = ui->checkboxEnableScheduledLogin->isChecked();
    scheduledLoginStyle = ui->comboboxScheduledLoginStyle->currentIndex();
    enableIgnoreWarning = ui->checkBoxIgnoreWarning->isChecked();
    enableAutoLogin = ui->checkboxEnableAutoLogIn->isChecked();
    enableRunAtStartup = ui->checkboxEnableRunAtStartup->isChecked();
    pushUiData();
    naManager->updateData(userName, password, defaultTunnel, expireTime);
}

void Widget::pushUiData()
{
    ui->textUserName->setText(userName);
    ui->textPassword->setText(password);
    ui->comboDefaultTunnel->setCurrentIndex(defaultTunnel);
    ui->comboExpireTime->setCurrentIndex(expireTimeIndex);

    ui->checkboxEnableScheduledCheckNet->setChecked(enableScheduledCheckNet);
    ui->textScheduledCheckNetTime->setText(scheduledCheckNetTime);
    ui->textScheduledCheckNetTime->setEnabled(enableScheduledCheckNet);

    ui->checkboxEnableScheduledLogin->setChecked(enableScheduledLogin);
    ui->comboboxScheduledLoginStyle->setCurrentIndex(scheduledLoginStyle);
    ui->checkBoxIgnoreWarning->setChecked(enableIgnoreWarning);

    ui->checkboxEnableAutoLogIn->setChecked(enableAutoLogin);
    ui->checkboxEnableRunAtStartup->setChecked(enableRunAtStartup);
}

void Widget::saveToIni()
{
    fetchUiData();

    QSettings *iniSettings = new QSettings(iniName, QSettings::IniFormat);
    iniSettings->setValue("userName",userName);
    iniSettings->setValue("password",this->passwordEncryption(password.toLocal8Bit().data(),key));
    iniSettings->setValue("defaultTunnel",defaultTunnel);
    iniSettings->setValue("expireTimeIndex",expireTimeIndex);

    iniSettings->setValue("enableScheduledCheckNet",enableScheduledCheckNet);
    iniSettings->setValue("scheduledCheckNetTime",scheduledCheckNetTime);
    iniSettings->setValue("enableScheduledLogin",enableScheduledLogin);
    iniSettings->setValue("scheduledLoginStyle",scheduledLoginStyle);
    iniSettings->setValue("enableIgnoreWarning",enableIgnoreWarning);
    iniSettings->setValue("enableTimeRange",ui->checkBoxScheduledTimeRange->isChecked());
    iniSettings->setValue("enableMonday",vCheckboxWeek.at(0)->isChecked());
    iniSettings->setValue("enableTuesday",vCheckboxWeek.at(1)->isChecked());
    iniSettings->setValue("enableWednesday",vCheckboxWeek.at(2)->isChecked());
    iniSettings->setValue("enableThursday",vCheckboxWeek.at(3)->isChecked());
    iniSettings->setValue("enableFriday",vCheckboxWeek.at(4)->isChecked());
    iniSettings->setValue("enableSaturday",vCheckboxWeek.at(5)->isChecked());
    iniSettings->setValue("enableSunday",vCheckboxWeek.at(6)->isChecked());
    iniSettings->setValue("timeStartTask",ui->timeStartTask->time());
    iniSettings->setValue("timeEndTask",ui->timeEndTask->time());

    iniSettings->setValue("enableAutoLogin",enableAutoLogin);
    iniSettings->setValue("enableRunAtStartup",enableRunAtStartup);

    iniSettings->setValue("allowRemote",ui->checkBoxAllowRemote->isChecked());
    iniSettings->setValue("localPort",ui->textLocalPort->text().toUInt());
    iniSettings->setValue("remoteIP",ui->textRemoteIP->text());
    iniSettings->setValue("remotePort",ui->textRemotePort->text().toUInt());

    delete iniSettings;
}

void Widget::loadFromIni()
{
    QSettings *iniSettings = new QSettings(iniName, QSettings::IniFormat);
    userName = iniSettings->value("userName").toString();
    password = QString(this->passwordDecryption(iniSettings->value("password").toByteArray(),key));
    defaultTunnel = iniSettings->value("defaultTunnel").toInt();
    expireTimeIndex = iniSettings->value("expireTimeIndex").toInt();
    enableScheduledCheckNet = iniSettings->value("enableScheduledCheckNet").toBool();
    scheduledCheckNetTime = iniSettings->value("scheduledCheckNetTime").toString();
    enableScheduledLogin = iniSettings->value("enableScheduledLogin").toBool();
    scheduledLoginStyle = iniSettings->value("scheduledLoginStyle").toInt();
    enableIgnoreWarning = iniSettings->value("enableIgnoreWarning").toBool();
    enableAutoLogin = iniSettings->value("enableAutoLogin").toBool();
    enableRunAtStartup = iniSettings->value("enableRunAtStartup").toBool();

    ui->checkBoxScheduledTimeRange->setChecked(iniSettings->value("enableTimeRange").toBool());
    vCheckboxWeek.at(0)->setChecked(iniSettings->value("enableMonday").toBool());
    vCheckboxWeek.at(1)->setChecked(iniSettings->value("enableTuesday").toBool());
    vCheckboxWeek.at(2)->setChecked(iniSettings->value("enableWednesday").toBool());
    vCheckboxWeek.at(3)->setChecked(iniSettings->value("enableThursday").toBool());
    vCheckboxWeek.at(4)->setChecked(iniSettings->value("enableFriday").toBool());
    vCheckboxWeek.at(5)->setChecked(iniSettings->value("enableSaturday").toBool());
    vCheckboxWeek.at(6)->setChecked(iniSettings->value("enableSunday").toBool());

    ui->timeStartTask->setTime(iniSettings->value("timeStartTask").toTime());
    ui->timeEndTask->setTime(iniSettings->value("timeEndTask").toTime());

    ui->checkBoxAllowRemote->setChecked(iniSettings->value("allowRemote").toBool());
    ui->textLocalPort->setText(iniSettings->value("localPort").toString());
    ui->textRemoteIP->setText(iniSettings->value("remoteIP").toString());
    ui->textRemotePort->setText(iniSettings->value("remotePort").toString());

    delete iniSettings;

    pushUiData();
}

void Widget::getCurrentTunnel(int m_currentTunnel)
{
    if (m_currentTunnel >= 0)                                               // 登陆成功
    {
        currentTunnel = m_currentTunnel;
        if (m_currentTunnel < 9)                                            // 网络通权限正常
        {
            networkAccess = true;
            trayIcon->setIcon(QIcon(QString(":/images/WLT_logo_") + QString::number(currentTunnel) + QString(".png")));
            this->setCheckedTunnel(currentTunnel);
        }
        else                                                               // 网络通权限权限为校内
        {
            networkAccess = false;
            trayIcon->setIcon(QIcon(QString(":/images/WLT_logo_none.png")));
            this->setCheckedTunnel(currentTunnel-1958);
        }
        if (enableScheduledCheckNet and enableScheduledLogin)
            emit scheduledCheckNetTunnelReturned(currentTunnel);
    }
    else if (!enableIgnoreWarning)                      // 登陆失败
    {
        networkAccess = false;
        scheduledCheckNetTimer->stop();
        ui->checkboxEnableScheduledCheckNet->setChecked(false);
        ui->checkboxEnableScheduledLogin->setChecked(false);
        currentTunnel = defaultTunnel;

        if (msgboxLoginFailed->exec() == QMessageBox::Help)
            naManager->displayAnswer();

        this->show();
    }
    emit tunnelUpdated();
}

void Widget::on_buttonSet_clicked()
{
    this->saveToIni();

    if (ui->tabWidget->currentIndex() == 0)
        naManager->setTunnel(defaultTunnel);
    else if (ui->tabWidget->currentIndex() == 1)
    {
        on_textScheduledCheckNetTime_editingFinished();

        if (enableScheduledCheckNet)
            scheduledCheckNetTimer->start(1000*scheduledCheckNetTime.toInt()+0.17);
        else
            scheduledCheckNetTimer->stop();

        enableIgnoreWarning = ui->checkBoxIgnoreWarning->isChecked();
    }

    // 短暂的动画
    ui->buttonSet->setText(tr("确定 √"));
    QTimer* tempTimer = new QTimer(this);
    connect(tempTimer, &QTimer::timeout, this, [=](){
        ui->buttonSet->setText(tr("确定"));
        tempTimer->stop();
        delete tempTimer;
    });
    tempTimer->start(500);
}

void Widget::on_buttonClose_clicked()
{
    this->hide();
}

void Widget::on_buttonHelp_clicked()
{
    msgboxAbout->show();
}

QByteArray Widget::passwordEncryption(QByteArray password, int key)
{
    for (int i = 0; i < password.size(); i++)
        password[i] += key;
    return password;
}

QByteArray Widget::passwordDecryption(QByteArray password, int key)
{
    for (int i = 0; i < password.size(); i++)
        password[i] -= key;
    return password;
}

