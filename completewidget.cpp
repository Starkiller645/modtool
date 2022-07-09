#include "completewidget.h"
#include "ui_completewidget.h"

CompleteWidget::CompleteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CompleteWidget)
{
    ui->setupUi(this);
}

CompleteWidget::~CompleteWidget()
{
    delete ui;
}

void CompleteWidget::updateText(std::string path, int num) {
    std::string text = "Successfully installed " + std::to_string(num) + " mods to " + path;
    this->ui->installLabel->setText(text.c_str());
}

void CompleteWidget::updateCacheInfo(int downloaded, int cached, int old_removed) {
    std::string text = "Downloaded ";
    if(downloaded == 0) {
        text += "no mods ";
    } else if(downloaded == 1) {
        text += "1 mod ";
    } else {
        text += std::to_string(downloaded) + " mods ";
    }
    text += "and used ";
    if(cached == 0) {
        text += "no cached files ";
    } else if(cached == 1) {
        text += "1 cached file ";
    } else {
        text += std::to_string(cached) + " cached files ";
    }
    this->ui->infoLabel->setText(text.c_str());

    std::string deleted_text = "";
    if(old_removed == 1) {
        deleted_text = "Removed 1 old mod";
    } else if(old_removed > 1) {
        deleted_text = "Removed " + std::to_string(old_removed) + " old mods";
    }

    this->ui->deletedLabel->setText(deleted_text.c_str());
}
