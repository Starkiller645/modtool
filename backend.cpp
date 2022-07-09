#include "backend.h"
#include <QtCore>
#include <QProcess>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QMessageBox>
#include <QUrl>
#include <iostream>
#include <QDir>
#include <iomanip>
#include <sstream>
#ifdef WIN_32
    #include <Windows.h>
#endif

double round(double d)
{
    double value = (int)(d * 100 + .5);
    return (double)value / 100;
}

Backend::Backend(QObject *parent) : QObject(parent)
{

}

void Backend::init() {
#ifdef __unix__
    std::string envHome = std::getenv("HOME");
    this->xdg_resources_dir = envHome + "/.local/share";
    this->cache_dir = this->xdg_resources_dir + "/modtool";
    this->mc_dir = envHome + "/.minecraft";
    this->mc_mods_dir = this->mc_dir + "/mods";
    this->mc_versions_dir = this->mc_dir + "/versions";
#elif defined(WIN32) || defined(_WIN32)
    std::string appData = std::getenv("APPDATA");
    this->xdg_resources_dir = std::getenv("APPDATA");
    this->cache_dir = this->xdg_resources_dir + "\\ModTool";
    this->mc_dir = this->xdg_resources_dir + "\\.minecraft";
    this->mc_mods_dir = this->mc_dir + "\\mods";
    this->mc_versions_dir = this->mc_dir + "\\versions"
#elif __APPLE__ && TARGET_OS_MAC
    std::string envHome = std::getenv("HOME");
    this->xdg_resources_dir = envHome + "/Library/Application Support";
    this->cache_dir = this->xdg_resources_dir + "/dev.tallie.modtool";
    this->mc_dir = envHome + "/minecraft";
    this->mc_mods_dir = this->mc_dir + "/mods";
    this->mc_versions_dir = this->mc_dir + "/versions";
#endif
    QDir cache_dir;
    cache_dir.mkdir(this->cache_dir.c_str());
    cache_dir.mkdir(this->mc_mods_dir.c_str());

    this->cache = new QDir(this->cache_dir.c_str());

    this->javaStart();
}

void Backend::javaStart() {
    std::cout << "       [\033[33mJava\033[0m] Checking for Java...\n";
    QStringList args;
#if !defined(WIN32) && !defined(_WIN32)
    args << "--version";
#else
    args << "-version";
#endif
    QProcess java;
    java.start("java", args);
    java.waitForFinished();
    QString output(java.readAllStandardOutput());
    QString err(java.readAllStandardError());
    if(output.trimmed().isEmpty() && err.trimmed().isEmpty()) {
        std::string url;
        std::string filename;
        std::cout << "       [\033[33mJava\033[0m] Could not find Java" << std::endl;
#ifdef __unix__
        emit backendError("Java was not found on your system. Please install Java through your package manager and rerun the program.");
#elif defined(WIN32) || defined(_WIN32)
        emit backendError("Java was not found on your system. Please install java first from https://www.java.com/en/download/ and rerun the program");
#elif __APPLE_
    #if TARGET_OS_MAC
        emit backendError("We do not officially support MacOS, because *someone* didn't want me 'fiddling with their Mac'. Please install Java yourself and rerun");
#endif
#endif
    } else {
        std::cout << "       [\033[33mJava\033[0m] Found a valid Java install, continuing...\n";
        this->forgeStart();
    }
}

void Backend::javaInstall() {
    std::cout << "Installing Java" << std::endl;
    QProcess jinstall;
    QStringList args = {};
    jinstall.start(".\\jre-8u301-windows-x64.exe", args);
    jinstall.waitForFinished();
    QString stdErr(jinstall.readAllStandardError());
    QString stdOut(jinstall.readAllStandardOutput());
    connect(&jinstall, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(forgeStart(int, QProcess::ExitStatus)));
}

void Backend::forgeStart(int, QProcess::ExitStatus) {
    return forgeStart();
}

void Backend::forgeStart() {
    std::cout << "      [\033[96mForge\033[0m] Checking for Forge...\n";
    QDir cache_dir;
    cache_dir.mkdir(this->cache_dir.c_str());
    QDir mc_versions_dir(this->mc_versions_dir.c_str());
    bool haveForge = false;
    QStringList versions = mc_versions_dir.entryList(QStringList() << "*", QDir::Dirs);
    foreach(QString filename, versions) {
        if(filename.contains("1.12.2-forge")) {
            std::cout << "      [\033[96mForge\033[0m] Found Forge install: \033[2m" << filename.toStdString() << "\n";
            haveForge = true;
            break;
        }
    }

    if(haveForge) {
        this->manifestStart();
    } else {
        emit backendInfo("A valid Forge install was not found. We are now downloading the Forge installer. Please follow the instructions to install Forge.");
        QUrl forge_dl_url("https://maven.minecraftforge.net/net/minecraftforge/forge/1.12.2-14.23.5.2859/forge-1.12.2-14.23.5.2859-installer.jar");
        this->forge_filename = forge_dl_url.fileName().toStdString();
        this->downloadFile(this->forge_filename, forge_dl_url.toString().toStdString());
        disconnect(this, &Backend::downloadComplete, nullptr, nullptr);
        connect(this, &Backend::downloadComplete, [this](){this->forgeInstall();});
    }
}

void Backend::forgeInstall() {
     QProcess *finstall = new QProcess;
 #ifdef __unix__
     std::string path;
     path = this->cache_dir + "/" + this->forge_filename;
     QStringList args = {"-jar", QString(path.c_str())};
 #elif defined(WIN32) || defined(_WIN32)
     std::string path;
     path = this->cache_dir + "\\" + this->forge_filename;
     std::cout << path << std::endl;
     QStringList args = {"-jar", QString(path.c_str())};
 #endif
    finstall->start("java", args);
    connect(finstall, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus){this->manifestStart();});
}

void Backend::downloaded(QNetworkReply *res) {
    QByteArray data = res->readAll();
    if(this->downloadMultipleFiles) {
        QUrl file(this->url_list[this->iter].c_str());
#ifdef __unix__
        std::string path;
        path = this->cache_dir + "/" + file.fileName().toStdString();
        QFile filehandle(path.c_str());
#elif defined(WIN32) || defined(_WIN32)
        std::string path;
        path = this->cache_dir + "\\" + file.fileName().toStdString();
        QFile filehandle(path.c_str());
#endif
         filehandle.open(QIODevice::WriteOnly);
        filehandle.write(data);
        filehandle.close();
    } else {
        QUrl file(this->download_filename.c_str());
#ifdef __unix__
        std::string path;
        path = this->cache_dir + "/" + file.fileName().toStdString();
        QFile filehandle(path.c_str());
#elif defined(WIN32) || defined(_WIN32)
        std::string path;
        path = this->cache_dir + "\\" + file.fileName().toStdString();
        QFile filehandle(path.c_str());
#endif
        filehandle.open(QIODevice::WriteOnly);
        filehandle.write(data);
        filehandle.close();
    }
    this->iter += 1;
    this->downloaded_mods += 1;

    std::string fname = QUrl(this->download_filename.c_str()).fileName().toStdString();        ;
    std::string orig_fname = fname;
    if(fname.length() > 30) {
        fname = fname.substr(0, 27) + "...";
    }
    while(fname.length() < 30) {
        fname += " ";
    }

    std::cout << "\e[F\e[K\e[F\e[K\e[F\e[K\e[F\e[K";
    std::cout << " [\033[92mDownloaded\033[0m] " << orig_fname << "\033[0m\n";
    std::cout << "╭────────────────────────────────────────────────────────────╮" << "\n";
    std::cout << "│Download complete: " << fname << "           │\n";
    std::cout << "│[DOWNLOAD FINISHED] \033[94m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m 100%│\n";
    std::cout << "╰────────────────────────────────────────────────────────────╯\n";
    //std::cout << "\n";
    if(!this->downloadMultipleFiles) emit downloadComplete();
}

void Backend::downloadFile(std::string name, std::string url_str) {
    this->downloadMultipleFiles = false;
    this->download_filename = name;
    QUrl url(url_str.c_str());
    auto *manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, &Backend::downloaded);
    QNetworkRequest rq(url);
    QNetworkReply *reply = manager->get(rq);
    connect(reply, &QNetworkReply::downloadProgress, this, &Backend::logProgress);
}

void Backend::logProgress(qint64 received, qint64 total) {
    if(total != 0) {
        std::cout << "\e[F\e[K\e[F\e[K\e[F\e[K\e[F\e[K";
        std::cout << "╭────────────────────────────────────────────────────────────╮" << "\n";
        //This is 60 horizontal line chars plus a corner at each end, so 60 chars of space available

        int percentage = ((double)received / (double)total) * 100;
        float mb_total = (float)total / 1000000.0;
        float mb_current = (float)received / 1000000.0;

        std::string fname = QUrl(this->download_filename.c_str()).fileName().toStdString();        ;
        if(fname.length() > 30) {
            fname = fname.substr(0, 27) + "...";
        }
        while(fname.length() < 30) {
            fname += " ";
        }

        std::stringstream ss_1;
        ss_1 << std::fixed << std::setprecision(2) << mb_total;
        std::stringstream ss_2;
        ss_2 << std::fixed << std::setprecision(2) << mb_current;

        std::string mb_total_string = ss_1.str();
        std::string mb_current_string = ss_2.str();

        std::cout << "│";
        std::cout << "Downloading: ";
        std::cout << fname;
        // Should always be 30 chars + 13 for "Downloading" so 43 total
        std::cout << "                 ";
        std::cout << "│\n";

        std::cout << "│";
        std::cout << "[" << mb_current_string << "MB/" << mb_total_string << "MB]";

        int download_pad = 19 - (mb_current_string.length() + mb_total_string.length() + 7);

        for(int i = 0; i < download_pad; i++) {
            std::cout << " ";
        }

        std::cout << " ";
        // 19 chars + 2 for "  " so 21 total, + " xx%" is 25 leaving 35 for the progress bar

        int cells_complete = ((double)percentage / 100.0) * 35.0;
        int cells_inprogress = 35 - cells_complete;

        for(int i = 0; i < cells_complete; i++) {
            std::cout << "\033[94m━\033[0m";
        }
        for(int i = 0; i < cells_inprogress; i++) {
            std::cout << "\033[2m━\033[0m";
        }

        std::string percentage_string = std::to_string(percentage);
        if(percentage < 10) percentage_string = " " + percentage_string;
        if(percentage < 100) percentage_string = " " + percentage_string;

        std::cout << " " << percentage_string << "%";

        std::cout << "│\n";
        std::cout << "╰────────────────────────────────────────────────────────────╯\n";

        emit modDownloadProgress(percentage, round(mb_current), round(mb_total));
    } else {
        std::cout << "Downloading: [...]\n";
        emit modDownloadProgress(0, 0.0, 0.0);
    }
}

void Backend::downloadFile(std::vector<std::string> url_list, int iter) {
    this->downloadMultipleFiles = true;
    this->url_list = url_list;
    this->iter = iter;
    if(this->iter >= this->url_list.size()) {
        emit downloadComplete();
        return;
    }
    QUrl url(this->url_list[this->iter].c_str());
    this->download_filename = url.toString().toStdString();
    QString filename = url.fileName();

    if(this->cacheCheck(filename.toStdString())) {
        this->iter += 1;
        this->cached_mods += 1;

        std::string fname = QUrl(this->download_filename.c_str()).fileName().toStdString();        ;
        std::string orig_fname = fname;
        if(fname.length() > 30) {
            fname = fname.substr(0, 27) + "...";
        }
        while(fname.length() < 30) {
            fname += " ";
        }

        std::cout << "\e[F\e[K\e[F\e[K\e[F\e[K\e[F\e[K";
        std::cout << "  [\033[92mCache Hit\033[0m] \033[2m" << orig_fname << "\033[0m\n";
        std::cout << "╭────────────────────────────────────────────────────────────╮" << "\n";
        std::cout << "│Cache Hit for: " << fname << "               │\n";
        std::cout << "│[CACHE HIT ON FILE] \033[92m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m 100%│\n";
        std::cout << "╰────────────────────────────────────────────────────────────╯\n";
        this->downloadFile(this->url_list, this->iter);
    } else {
        auto *manager = new QNetworkAccessManager();
        connect(manager, &QNetworkAccessManager::finished, this, &Backend::downloaded);
        if(this->downloadingMods) {
            try {
                emit downloadingMod(this->mods_name_list[iter]);
            } catch(std::logic_error) {
                ;
            }
        }
        QNetworkRequest rq(url);
        QNetworkReply *reply = manager->get(rq);
        connect(reply, &QNetworkReply::downloadProgress, this, &Backend::logProgress);
        connect(manager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *res){this->downloadFile(this->url_list, this->iter);});
        switch (reply->error()) {
            case QNetworkReply::NoError:
                break;
            default:
                std::cout << "An error occurred during file download";
        }
    }
}

bool Backend::cacheCheck(std::string filename) {
    QStringList cached_files = this->cache->entryList(QStringList() << "*.jar", QDir::Files);
    if(cached_files.contains(filename.c_str(), Qt::CaseInsensitive)) return true;
    return false;
}

int Backend::flushOldMods() {
    int removal_count = 0;
    QDir modsDir(this->cache_dir.c_str());
    QStringList mods = modsDir.entryList(QStringList() << "*.jar", QDir::Files);
    for(int i = 0; i < this->mods_url_list.size(); i++) {
        QString filename = QUrl(this->mods_url_list[i].c_str()).fileName();
    }

    foreach(QString mod_name, mods) {
        bool found_mod = false;
        for(int i = 0; i < this->mods_url_list.size(); i++) {
            QString filename = QUrl(this->mods_url_list[i].c_str()).fileName();
            if(filename == mod_name) found_mod = true;
        }
        if(!found_mod) {
            std::string file_path = this->cache_dir.c_str();
#if defined(WIN32) || defined(_WIN32)
            file_path += "\\";
#else
            file_path += "/";
#endif
            file_path += mod_name.toStdString();
            QFile mod_to_remove(file_path.c_str());
            mod_to_remove.remove();
            removal_count += 1;
            std::cout << "    [\033[91mRemoved\033[0m] " + file_path << std::endl;
        }
    }
    return removal_count;
}

void Backend::installMods() {
    std::cout << "\e[F\e[K\e[F\e[K\e[F\e[K\e[F\e[K";

    int mods_deleted = this->flushOldMods();

    QDir currentDir(this->cache_dir.c_str());
    QDir modsDir(this->mc_mods_dir.c_str());
    modsDir.mkdir(this->mc_mods_dir.c_str());
    QStringList mods = currentDir.entryList(QStringList() << "*.jar", QDir::Files);
    QStringList oldMods = modsDir.entryList(QStringList() << "*.jar", QDir::Files);

    foreach(QString old_mod, oldMods) {
        modsDir.remove(old_mod);
    }

    foreach(QString mod_file, mods) {
#if defined(__unix__) || defined(__APPLE__)
        std::string path = this->mc_mods_dir + "/";
        std::string cache_path = this->cache_dir + "/";
#elif defined(WIN32) || defined(_WIN32)
        std::string path = this->mc_mods_dir + "\\";
        std::string cache_path = this->cache_dir + "\\";
#endif
        QString new_location = QString(path.c_str()) + mod_file;
        std::string mod_path = cache_path + mod_file.toStdString();
        std::cout << " [\033[95mInstalling\033[0m] " << mod_file.toStdString() << "\033[0m\n";
        if (!QFile::copy(mod_path.c_str(), currentDir.toNativeSeparators(new_location))) std::cout << "Copy failed\n";
    }
    emit switchPage(2);
    emit modInfo(this->mc_mods_dir, this->mods_name_list.size());
    emit cacheInfo(this->downloaded_mods, this->cached_mods, mods_deleted);
}

void Backend::modsStart() {
    emit switchPage(1);
    emit setupDownload(this->mods_name_list.size());
    this->downloaded_mods = 0;
    this->cached_mods = 0;
    this->downloadingMods = true;
    disconnect(this, &Backend::downloadComplete, nullptr, nullptr);
    connect(this, &Backend::downloadComplete, this, &Backend::installMods);
    this->downloadFile(this->mods_url_list, 0);
}

void Backend::parseManifest() {
#ifdef __unix__
    std::string path;
    path = this->cache_dir + "/manifest.modtool.txt";
    QFile manifest_file(path.c_str());
#elif defined(WIN32) || defined(_WIN32)
    std::string path;
    path = this->cache_dir + "\\manifest.modtool.txt";
    QFile manifest_file(path.c_str());
#endif
    manifest_file.open(QIODevice::ReadOnly);
    QByteArray manifest_data = manifest_file.readAll();
    QString manifest = QString(manifest_data);
    QStringList manifest_list;
    manifest_list = manifest.split("\n");
    std::vector<std::string> mods_url_list;
    std::vector<std::string> name_list;
    foreach(QString line, manifest_list) {
        QStringList vars = line.split("|");
        if(vars.length() > 1) {
            name_list.push_back(vars[0].toStdString());
            mods_url_list.push_back(vars[1].toStdString());
        }
    }
    this->mods_url_list = mods_url_list;
    this->mods_name_list = name_list;
    this->modsStart();
}

void Backend::manifestStart() {
    std::cout << "\n\n\n\n";
    this->downloadFile("manifest.modtool.txt", "https://tallie.dev/modtool/manifest");
    disconnect(this, &Backend::downloadComplete, nullptr, nullptr);
    connect(this, &Backend::downloadComplete, this, &Backend::parseManifest);
}
