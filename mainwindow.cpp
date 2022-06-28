#include "mainwindow.h"
#include "backend.h"
#include "./ui_mainwindow.h"
#include "ui_startpage.h"
#include "ui_downloadpage.h"
#include "ui_completewidget.h"
#include <QObject>
#include <QPushButton>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    backend = new Backend();
    this->setWindowTitle("Tallie's ModTool - v1.3");
    connect(ui->startPage->ui->startButton, &QPushButton::clicked, this->backend, &Backend::init);
    connect(backend, &Backend::backendError, [this](std::string message){QMessageBox::warning(this, "ModTool", message.c_str());});
    connect(backend, &Backend::backendInfo, [this](std::string message){QMessageBox::information(this, "ModTool", message.c_str());});
    connect(backend, &Backend::switchPage, this->ui->stackedWidget, &QStackedWidget::setCurrentIndex);
    connect(backend, &Backend::setupDownload, this->ui->downloadPage, &DownloadPage::setup);
    connect(backend, &Backend::downloadingMod, this->ui->downloadPage, &DownloadPage::setName);
    connect(backend, &Backend::modInfo, this->ui->completePage, &CompleteWidget::updateText);
    connect(backend, &Backend::modDownloadProgress, this->ui->downloadPage, &DownloadPage::modDownloadProgress);
    connect(backend, &Backend::cacheInfo, this->ui->completePage, &CompleteWidget::updateCacheInfo);
}

MainWindow::~MainWindow()
{
    delete ui;
}

