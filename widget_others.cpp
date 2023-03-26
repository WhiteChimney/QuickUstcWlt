#include "widget.h"
#include "./ui_widget.h"


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

            args << "-e tell application \"System Events\" to delete (every login item whose name is \""
                + macOSXAppBundleName + "\")";
            QProcess::execute("osascript", args);

            args << "-e tell application \"System Events\" to make login item at end with properties {path:\""
                    + macOSXAppBundlePath + "\", hidden:false}";

            QProcess::execute("osascript", args);
        }
        else
        {
            QStringList args;
            args << "-e tell application \"System Events\" to delete (every login item whose name is \""
                + macOSXAppBundleName + "\")";

            QProcess::execute("osascript", args);
        }
    }
}

void Widget::on_buttonIniPath_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(iniPath));
}
