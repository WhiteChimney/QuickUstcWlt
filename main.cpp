#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    if (QSysInfo::productType() == "windows")
        a.setWindowIcon(QIcon(":/images/WLT_logo.ico"));
    else
        if (QSysInfo::kernelType() == "macos")
             a.setWindowIcon(QIcon(":/images/WLT_logo.icns"));
    Widget w;
    return a.exec();
}
