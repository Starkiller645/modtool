#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QtNetwork/QNetworkReply>
#include <QProcess>
/*#ifdef WIN_32
    #include <Windows.h>
#elif __unix__

#endif*/

struct Mod {
    std::string name;
    std::string filename;
    std::string url;
};

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
    void installMods();
    void init();
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
    void modInfo(std::string, int);
    void modDownloadProgress(int, float, float);
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
    std::vector<Mod> mods_list;
    std::string download_filename;

    std::string mc_dir;
    std::string cache_dir;
    std::string xdg_resources_dir;
    std::string mc_mods_dir;
    std::string mc_versions_dir;

    QDir *cache;
};

#endif // BACKEND_H
