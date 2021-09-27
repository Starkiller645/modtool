#include "mainwindow.h"
#include "backend.h"
#include "./ui_mainwindow.h"
#include "ui_startpage.h"
#include "ui_downloadpage.h"
#include <QObject>
#include <QPushButton>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    backend = new Backend();
    connect(ui->startPage->ui->startButton, &QPushButton::clicked, this->backend, &Backend::javaStart);
    connect(backend, &Backend::backendError, [this](std::string message){QMessageBox::warning(this, "ModTool", message.c_str());});
}

MainWindow::~MainWindow()
{
    delete ui;
}

