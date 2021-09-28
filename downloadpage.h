#ifndef DOWNLOADPAGE_H
#define DOWNLOADPAGE_H

#include <QWidget>

namespace Ui {
class DownloadPage;
}

class DownloadPage : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadPage(QWidget *parent = nullptr);
    ~DownloadPage();

public slots:
    void setName(std::string);
    void setup(int);

private:
    Ui::DownloadPage *ui;
    QString current_name;
    int current_iter;
    int total_iter;
};

#endif // DOWNLOADPAGE_H
