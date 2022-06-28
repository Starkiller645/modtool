#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/assets/logo.ico"));
    MainWindow w;
    w.show();
    return a.exec();
}
