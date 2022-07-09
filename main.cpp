#include "mainwindow.h"

#include <iostream>
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/assets/logo.ico"));
#if defined(WIN32) || defined(_WIN32)
    QFile* style_sheet = new QFile(":/style/assets/win11_dark.qss");
    style_sheet->open(QIODevice::ReadOnly | QIODevice::Text);
    QString style_data = style_sheet->readAll();
    std::cout << style_data.toStdString() << std::endl;
    a.setStyleSheet(style_data);
#endif
    MainWindow w;
    w.show();
    return a.exec();
}
