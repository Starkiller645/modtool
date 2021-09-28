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