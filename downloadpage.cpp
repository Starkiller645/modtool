#include "downloadpage.h"
#include "ui_downloadpage.h"

DownloadPage::DownloadPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadPage)
{
    ui->setupUi(this);
    this->current_iter = 0;
}

DownloadPage::~DownloadPage()
{
    delete ui;
}

void DownloadPage::setup(int total) {
    this->total_iter = total;
    this->ui->progressBar->setMinimum(0);
    this->ui->progressBar->setMaximum(this->total_iter);
}

void DownloadPage::setName(std::string name) {
    this->current_iter += 1;
    std::string text;
    text += std::to_string(this->current_iter);
    text += "/";
    text += std::to_string(this->total_iter);
    text += ": ";
    text += name;
    this->ui->infoLabel->setText(QString(text.c_str()));
    this->ui->progressBar->setValue(this->current_iter);
}