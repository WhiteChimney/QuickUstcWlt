#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    a.setWindowIcon(QIcon(":/images/WLT_logo.ico"));
    Widget w;
    return a.exec();
}
