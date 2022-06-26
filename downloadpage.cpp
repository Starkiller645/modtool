#include "downloadpage.h"
#include "ui_downloadpage.h"

#include <iomanip>
#include <sstream>

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

void DownloadPage::modDownloadProgress(int prog, float mb, float total) {
    std::string format_str;
    std::string mb_str;
    std::string total_str;
    std::stringstream mb_ss;
    mb_ss << std::fixed << std::setprecision(2) << mb;
    mb_str = mb_ss.str();
    std::stringstream total_ss;
    total_ss << std::fixed << std::setprecision(2) << total;
    total_str = total_ss.str();
    format_str = "[";
    format_str += mb_str;
    format_str += "MB / ";
    format_str += total_str;
    format_str += "MB]";
    this->ui->progressLabel->setText(format_str.c_str());
    this->ui->progressBarSub->setValue(prog);
}
