#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QtNetwork/QNetworkReply>
#include <QProcess>
/*#ifdef WIN_32
    #include <Windows.h>
#elif __unix__

#endif*/

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

public slots:
    void modsStart();
    void forgeStart();
    void forgeStart(int, QProcess::ExitStatus);
    void manifestStart();
    void javaStart();
    void downloaded(QNetworkReply*);
    void logProgress(qint64, qint64);
    void javaInstall();
signals:
    void switchPage(int);
    void manifestComplete();
    void modsComplete();
    void forgeComplete();
    void javaComplete();
    void downloadComplete();
    void nextFile(std::vector<std::string>, int);
    void backendError(std::string);
    void backendInfo(std::string);
    void setupDownload(int);
    void downloadingMod(std::string);
private:
    void downloadFile(std::string, std::string);
    void downloadFile(std::vector<std::string>, int);
    void forgeInstall();
    void parseManifest();
    std::vector<std::string> url_list;
    std::string java_filename;
    std::string forge_filename;
    int iter;
    bool downloadMultipleFiles = false;
    bool downloadingMods = false;
    std::vector<std::string> mods_url_list;
    std::vector<std::string> mods_name_list;
    std::string download_filename;
};

#endif // BACKEND_H
