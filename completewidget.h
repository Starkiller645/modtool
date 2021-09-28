#ifndef COMPLETEWIDGET_H
#define COMPLETEWIDGET_H

#include <QWidget>

namespace Ui {
class CompleteWidget;
}

class CompleteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CompleteWidget(QWidget *parent = nullptr);
    ~CompleteWidget();

public slots:
    void updateText(std::string, int);

private:
    Ui::CompleteWidget *ui;
};

#endif // COMPLETEWIDGET_H
