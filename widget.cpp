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
    connect(scheduledCheckNetTimer, &QTimer::timeout, naManager, [=](){
        if (ui->checkBoxScheduledTimeRange->isChecked())
        {
            if (vCheckboxWeek.at(currentDate->currentDate().dayOfWeek()-1)->isChecked()
                    and currentTime->currentTime() > ui->timeStartTask->time()
                    and currentTime->currentTime() < ui->timeEndTask->time())
                naManager->checkNet();
        }
    });
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

//    启动计划任务
    if (enableAutoLogin)
        on_buttonSet_clicked();

    naManager->updateData(userName, password, defaultTunnel, expireTime);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::setupUI()
{
    this->setWindowTitle(tr("首选项"));
    ui->comboDefaultTunnel->view()->setMinimumWidth(370);
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
}


void Widget::setupTrayMenu()
{
    trayMenu = new QMenu(this);

    trayMenu->addSeparator();

    actionQuickLogIn = new QAction(tr("一键上网"),this);
    trayMenu->addAction(actionQuickLogIn);
    connect(actionQuickLogIn, &QAction::triggered, naManager, [=](){naManager->setTunnel(defaultTunnel);});

    menuChangeTunnel = new QMenu(this);
    menuChangeTunnel = trayMenu->addMenu(tr("登陆其他出口"));
    QAction *actionTunnel = new QAction(tr("1 教育网出口"),this); vActionTunnel.append(actionTunnel);
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
        connect(actionTunnel, &QAction::triggered, this, [=](){naManager->setTunnel(i);});
    }

    trayMenu->addSeparator();

    actionGetCurrentStatus = new QAction(tr("获取当前网络状态"),this);
    trayMenu->addAction(actionGetCurrentStatus);
    connect(actionGetCurrentStatus, &QAction::triggered, naManager, [=](){naManager->checkNet();});

    actionLogOut = new QAction(tr("退出登陆网络通"),this);
    trayMenu->addAction(actionLogOut);
    connect(actionLogOut, &QAction::triggered, naManager, [=](){naManager->logoutWlt();});

    trayMenu->addSeparator();

    actionShowPref = new QAction(tr("首选项"),this); trayMenu->addAction(actionShowPref);
    connect(actionShowPref, &QAction::triggered, this, &Widget::show);

    actionExit = new QAction(tr("退出"),this);      trayMenu->addAction(actionExit);
    connect(actionExit, &QAction::triggered, qApp, &QApplication::quit);
}

void Widget::dealTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:      // 左键单击
        if (QSysInfo::productType() != "macos")   // macos 下单击无效
            this->show();
        break;
    case QSystemTrayIcon::Context:      // 右键单击
        break;
    case QSystemTrayIcon::DoubleClick:  // 左键双击
        break;
    case QSystemTrayIcon::MiddleClick:  // 中键单击
        naManager->checkNet();
        break;
    default:
        break;
    }
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
    enableAutoLogin = ui->checkboxEnableAutoLogIn->isChecked();
    enableRunAtStartup = ui->checkboxEnableRunAtStartup->isChecked();
    pushUiData();
    naManager->updateData(userName, password, defaultTunnel, expireTime);
}

void Widget::on_textScheduledCheckNetTime_editingFinished()
{
    if (ui->textScheduledCheckNetTime->text().toInt() < 3)
        ui->textScheduledCheckNetTime->setText(QString("3"));
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
    iniSettings->setValue("enableAutoLogin",enableAutoLogin);
    iniSettings->setValue("enableRunAtStartup",enableRunAtStartup);
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
            trayIcon->setIcon(QIcon(QString(":/images/WLT_logo_") + QString::number(currentTunnel) + QString(".png")));
            this->setCheckedTunnel(currentTunnel);
        }
        else                                                               // 网络通权限权限为校内
        {
            trayIcon->setIcon(QIcon(QString(":/images/WLT_logo_none.png")));
            this->setCheckedTunnel(currentTunnel-1958);
        }
        if (enableScheduledCheckNet and enableScheduledLogin)
            emit scheduledCheckNetTunnelReturned(currentTunnel);
    }
    else                                                                   // 登陆失败
    {
        scheduledCheckNetTimer->stop();
        ui->checkboxEnableScheduledCheckNet->setChecked(false);
        ui->checkboxEnableScheduledLogin->setChecked(false);
        currentTunnel = defaultTunnel;

        if (msgboxLoginFailed->exec() == QMessageBox::Help)
            naManager->displayAnswer();

        this->show();
    }
}

void Widget::dealScheduledCheckNetTunnelReturned(int m_currentTunnel)
{
    if (scheduledLoginStyle == 2 and m_currentTunnel != defaultTunnel)  // 第 3 种模式，非默认通道即登陆
        naManager->setTunnel(defaultTunnel);
    else if (m_currentTunnel > 8)                                       // 否则校内时才操作
    {
        if (scheduledLoginStyle == 0)                                   // 登陆默认通道
            naManager->setTunnel(defaultTunnel);
        else
            naManager->setTunnel(m_currentTunnel-1958);                 // 登陆当前通道
    }
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

void Widget::on_buttonSet_clicked()
{
    on_textScheduledCheckNetTime_editingFinished();

    this->saveToIni();

    if (enableScheduledCheckNet)
        scheduledCheckNetTimer->start(1000*scheduledCheckNetTime.toInt()+0.17);
    else
        scheduledCheckNetTimer->stop();

    if (enableRunAtStartup)
        this->setRunAtStartup(true);
    else
        this->setRunAtStartup(false);

    ui->buttonSet->setText(tr("确定 √"));
    QTimer* tempTimer = new QTimer(this);
    connect(tempTimer, &QTimer::timeout, this, [=](){
        ui->buttonSet->setText(tr("确定"));
        tempTimer->stop();
        delete tempTimer;
    });
    tempTimer->start(250);

    naManager->setTunnel(defaultTunnel);
}

void Widget::on_buttonClose_clicked()
{
    this->hide();
}

void Widget::on_checkboxEnableScheduledCheckNet_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
    {
        ui->textScheduledCheckNetTime->setEnabled(true);
        ui->checkboxEnableScheduledLogin->setEnabled(true);
        ui->checkBoxScheduledTimeRange->setEnabled(true);
    }
    else
    {
        ui->textScheduledCheckNetTime->setEnabled(false);
        ui->checkboxEnableScheduledLogin->setEnabled(false);
        ui->checkboxEnableScheduledLogin->setChecked(false);
        ui->checkBoxScheduledTimeRange->setEnabled(false);
        ui->checkBoxScheduledTimeRange->setChecked(false);
    }
}

void Widget::on_checkboxEnableScheduledLogin_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
    {
        ui->comboboxScheduledLoginStyle->setEnabled(true);
    }
    else
    {
        ui->comboboxScheduledLoginStyle->setEnabled(false);
    }
}


void Widget::on_checkBoxScheduledTimeRange_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
    {
        ui->groupboxTimeRange->setEnabled(true);
    }
    else
    {
        ui->groupboxTimeRange->setEnabled(false);
    }
}


void Widget::on_timeStartTask_userTimeChanged(const QTime &time)
{
    if (ui->timeEndTask->time() < time)
    {
        ui->timeEndTask->setTime(time);
        ui->timeEndTask->setTimeRange(time,ui->timeEndTask->maximumTime());
    }
}


void Widget::on_buttonHelp_clicked()
{
    QString helpMessage = "";
    helpMessage += tr("这是一个简易的网络通登陆及状态查询软件\n");
    helpMessage += tr("出口状态可以直接通过「系统托盘图标」看到\n\n");
    helpMessage += tr("第一次使用时会自动弹出「首选项」界面\n");
    helpMessage += tr("在该界面设置完「用户名」与「密码」后点击确定即可保存信息\n\n");
    helpMessage += tr("「定时查询状态（秒）」指每隔指定时间就查询一下网络通出口及权限状态\n");
    helpMessage += tr("「查询后登陆」指在查询结束后使用指定模式登陆网络通\n");
    helpMessage += tr("*注意如果间隔时间太短会触发网络通限制\n");
    helpMessage += tr("*官方指定不能在 2 分钟内进行超过 20 次操作\n\n");
    helpMessage += tr("托盘图标上「左键」可弹出首选项界面\n");
    helpMessage += tr("「中键」可直接查询网络通状态\n");
    helpMessage += tr("「右键」可进行更多操作\n");
    helpMessage += tr("「macOS 系统」因系统限制只能左键打开菜单\n");
    helpMessage += tr("「一键上网」指使用默认设置登陆网络通\n");
    QMessageBox::about(this,tr("操作说明"),helpMessage);
}

//设置程序自启动 appPath程序路径
void Widget::setRunAtStartup(bool setEnable)
{
    if (QSysInfo::productType() == "windows")
    {
        //注册表路径需要使用双反斜杠
        QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                           QSettings::NativeFormat);

        //以程序名称作为注册表中的键
        //根据键获取对应的值（程序路径）
        const QString appPath = qApp->applicationFilePath();
        QFileInfo fInfo(appPath);
        QString name = fInfo.baseName();
        QString path = settings.value(name).toString();

        //如果注册表中的路径和当前程序路径不一样，
        //则表示没有设置自启动或自启动程序已经更换了路径
        //toNativeSeparators的意思是将"/"替换为"\"
        QString newPath = QDir::toNativeSeparators(appPath);
        if (path != newPath)
        {
            if (setEnable)
                settings.setValue(name, newPath);
        }
        else
        {
            if (!setEnable)
                settings.remove(name);
        }
    }
    else if (QSysInfo::productType() == "macos")
    {
        QDir dir = QDir ( QCoreApplication::applicationDirPath() );
        dir.cdUp();
        dir.cdUp();
        QString macOSXAppBundlePath = dir.absolutePath();
        // absolutePath will contain a "/" at the end,
        // but we want the clean path to the .app bundle
        if ( macOSXAppBundlePath.length() > 0 && macOSXAppBundlePath.right(1) == "/" ) {
            macOSXAppBundlePath.chop(1);
        }

        QFileInfo fileInfo(macOSXAppBundlePath);
        QString macOSXAppBundleName = fileInfo.baseName();

        if (setEnable)
        {
            QStringList args;

            args << "-e tell application \"System Events\" to delete login item\""
                + macOSXAppBundleName + "\"";
            QProcess::execute("osascript", args);

            args << "-e tell application \"System Events\" to make login item at end with properties {path:\""
                    + macOSXAppBundlePath + "\", hidden:false}";

            QProcess::execute("osascript", args);
        }
        else
        {
            QStringList args;
            args << "-e tell application \"System Events\" to delete login item\""
                + macOSXAppBundleName + "\"";

            QProcess::execute("osascript", args);
        }
    }

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


