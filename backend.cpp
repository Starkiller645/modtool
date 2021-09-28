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
#ifdef WIN_32
    #include <Windows.h>
#endif

Backend::Backend(QObject *parent) : QObject(parent)
{

}

void Backend::javaStart() {
    qDebug() << "Checking for/downloading Java";
    const QStringList args = {"--version"};
    QProcess java;
    java.start("java", args);
    java.waitForReadyRead();
    QString output(java.readAllStandardOutput());
    QString err(java.readAllStandardError());
    qDebug() << output;
    qDebug() << err;
    std::cout << output.toStdString() << std::endl;
    std::cout << err.toStdString() << std::endl;
    //std::vector<std::string> test_vector = {"https://files.vexatos.com/Computronics/Computronics-1.12.2-1.6.6.jar", "https://media.forgecdn.net/files/2828/357/OpenComputers-MC1.12.2-1.7.5.192.jar", "https://media.forgecdn.net/files/2974/106/ImmersiveEngineering-0.12-98.jar"};
    if(output.trimmed().isEmpty()) {
        std::string url;
        std::string filename;
#ifdef __unix__
        emit backendError("You are on Linux but do not seem to have java installed. Please install Java through your package manager and rerun the program.");
#elif defined(WIN32) || defined(_WIN32)
        emit backendInfo("Java was not found on your system. We will now download the Java installer. Please follow the instructions to install Java");
        url = "https://sdlc-esd.oracle.com/ESD6/JSCDL/jdk/8u301-b09/d3c52aa6bfa54d3ca74e617f18309292/jre-8u301-windows-i586.exe?GroupName=JSC&FilePath=/ESD6/JSCDL/jdk/8u301-b09/d3c52aa6bfa54d3ca74e617f18309292/jre-8u301-windows-i586.exe&BHost=javadl.sun.com&File=jre-8u301-windows-i586.exe&AuthParam=1632757471_350cd084cf0f41795280719fc7f6339e&ext=.exe";
        filename = "jre-8u301-windows-i586.exe";
        this->java_filename = filename;
        this->downloadFile(url, filename);
        disconnect(this, &Backend::downloadComplete, nullptr, nullptr);
        connect(this, &Backend::downloadComplete, [this](){this->javaInstall();});
#elif __APPLE_
    #if TARGET_OS_MAC
        emit backendError("We do not officially support MacOS, because *someone* didn't want me 'fiddling with their Mac'. Please install Java yourself and rerun");
#endif
#endif
    } else {
        std::cout << "Java was found. Continuing install..." << std::endl;
        this->forgeStart();
    }
}

void Backend::javaInstall() {
    QProcess jinstall;
    QStringList args = {};
    jinstall.start(this->java_filename.c_str(), args);
    connect(&jinstall, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(forgeStart(int, QProcess::ExitStatus)));
}

void Backend::forgeStart(int, QProcess::ExitStatus) {
    return forgeStart();
}

void Backend::forgeStart() {
    std::string mc_dir_path;
#ifdef __unix__
    std::string envHome = std::getenv("HOME");
    mc_dir_path = envHome + "/.minecraft/versions/";
#elif defined(WIN32) || defined(_WIN32)
    std::string appData = std::getenv("APPDATA");
    mc_dir_path = appData + "/.minecraft/versions";
#elif __APPLE__ && TARGET_OS_MAC
    std::string envHome = std::getenv("HOME");
    mc_dir_path = envHome + "/Library/Application Support/minecraft";
#endif
    QDir mc_dir(mc_dir_path.c_str());
    bool haveForge = false;
    QStringList versions = mc_dir.entryList(QStringList() << "*", QDir::Dirs);
    foreach(QString filename, versions) {
        std::cout << filename.toStdString() << std::endl;
        if(filename.contains("1.12.2-forge")) {
            std::cout << "Found Forge install" << std::endl;
            haveForge = true;
            break;
        }
    }

    if(haveForge) {
        this->manifestStart();
    } else {
        emit backendInfo("A valid Forge install was not found. We are now downloading the Forge installer. Please follow the instructions to install Forge.");
        QUrl forge_dl_url("https://maven.minecraftforge.net/net/minecraftforge/forge/1.12.2-14.23.5.2855/forge-1.12.2-14.23.5.2855-installer.jar");
        this->forge_filename = forge_dl_url.fileName().toStdString();
        this->downloadFile(this->forge_filename, forge_dl_url.toString().toStdString());
        disconnect(this, &Backend::downloadComplete, nullptr, nullptr);
        connect(this, &Backend::downloadComplete, [this](){this->forgeInstall();});
    }
}

void Backend::forgeInstall() {
    QProcess *finstall = new QProcess;
    QStringList args = {"-jar", QString(this->forge_filename.c_str())};
    finstall->start("java", args);
    connect(finstall, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus){this->modsStart();});
}

void Backend::downloaded(QNetworkReply *res) {
    std::cout << "Writing file" << std::endl;
    QByteArray data = res->readAll();
    if(this->downloadMultipleFiles) {
        QUrl file(this->url_list[this->iter].c_str());
        QFile filehandle(file.fileName());
        filehandle.open(QIODevice::WriteOnly);
        filehandle.write(data);
        filehandle.close();
    } else {
        QFile filehandle(this->download_filename.c_str());
        filehandle.open(QIODevice::WriteOnly);
        filehandle.write(data);
        filehandle.close();
    }
    this->iter += 1;
    std::cout << "Download complete" << std::endl;
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
    int percentage = (received / total) * 100;
    std::cout << "Downloading: [" << std::to_string(percentage) << "%]\n";
}

void Backend::downloadFile(std::vector<std::string> url_list, int iter) {
    this->downloadMultipleFiles = true;
    this->url_list = url_list;
    this->iter = iter;
    std::cout << "Progress: " << std::to_string(iter) << "/" << std::to_string(url_list.size()) << std::endl;
    if(this->iter >= this->url_list.size()) {
        emit downloadComplete();
        std::cout << "Done!" << std::endl;
        return;
    }
    QUrl url(this->url_list[this->iter].c_str());
    QString filename = url.fileName();
    auto *manager = new QNetworkAccessManager();
    std::cout << "Downloading: " << filename.toStdString() << std::endl << "From: " << url.toString().toStdString() << std::endl << std::endl;
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
            std::cout << "No errors found";
            break;
        default:
            std::cout << "An error occurred during file download";
    }
}

void Backend::installMods() {
    std::cout << "Installing mods" << std::endl;
    std::string mod_dir_path;
    #ifdef __unix__
    std::string envHome = std::getenv("HOME");
    mod_dir_path = envHome + "/.minecraft/mods/";
#elif defined(WIN32) || defined(_WIN32)
    std::string appData = std::getenv("APPDATA");
    mod_dir_path = appData + "/.minecraft/mods/";
#elif __APPLE__ && TARGET_OS_MAC
    std::string envHome = std::getenv("HOME");
    mod_dir_path = envHome + "/Library/Application Support/mods/";
#endif
    QDir currentDir("./");
    QStringList mods = currentDir.entryList(QStringList() << "*.jar", QDir::Files);
    foreach(QString mod_file, mods) {
        QString new_location = QString(mod_dir_path.c_str()) + mod_file;
        currentDir.rename(mod_file, currentDir.toNativeSeparators(new_location));
    }
    emit switchPage(2);
    emit modInfo(mod_dir_path, this->mods_name_list.size());
}

void Backend::modsStart() {
    emit switchPage(1);
    emit setupDownload(this->mods_name_list.size());
    this->downloadingMods = true;
    this->downloadFile(this->mods_url_list, 0);
    disconnect(this, &Backend::downloadComplete, nullptr, nullptr);
    connect(this, &Backend::downloadComplete, this, &Backend::installMods);
    
}

void Backend::parseManifest() {
    std::cout << "Parsing manifest" << std::endl;
    QFile manifest_file("manifest.txt");
    manifest_file.open(QIODevice::ReadOnly);
    QByteArray manifest_data = manifest_file.readAll();
    QString manifest = QString(manifest_data);
    QStringList manifest_list;
    manifest_list = manifest.split("\n");
    std::vector<std::string> mods_url_list;
    std::vector<std::string> name_list;
    foreach(QString line, manifest_list) {
        QStringList vars = line.split("|", Qt::SkipEmptyParts);
        if(vars.length() > 1) {
            name_list.push_back(vars[0].toStdString());
            mods_url_list.push_back(vars[1].toStdString());
        }
    }
    foreach(std::string mod, mods_name_list) {
        std::cout << mod << std::endl;
    }
    this->mods_url_list = mods_url_list;
    this->mods_name_list = name_list;
    this->modsStart();
}

void Backend::manifestStart() {
    std::cout << "Downloading manifest" << std::endl;
    this->downloadFile("manifest.txt", "https://jacobtye.dev/manifest");
    disconnect(this, &Backend::downloadComplete, nullptr, nullptr);
    connect(this, &Backend::downloadComplete, this, &Backend::parseManifest);
}
