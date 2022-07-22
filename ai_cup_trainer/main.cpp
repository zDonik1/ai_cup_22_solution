/**************************************************************************
 *
 *   @author Doniyorbek Tokhirov <tokhirovdoniyor@gmail.com>
 *   @date 11/07/2022
 *
 *************************************************************************/

#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QString>
#include <QCommandLineParser>
#include <QDebug>

using namespace std;

constexpr auto EPISODE_DIR = "episode_";

auto lastEpisode()
{
    QString episodeDir = EPISODE_DIR;
    const auto episodes = QDir::current().entryList({episodeDir + "*"}, QDir::Dirs, QDir::Name);
    return episodes.empty() ? 0 : episodes.last().sliced(episodeDir.size()).toInt();
}

int main(int argc, char *argv[])
{
    auto currentEpisode = lastEpisode();
    auto lastEpisodeDir = QString{};

    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("ai_cup_trainer");
    QCoreApplication::setApplicationVersion("1.0");

    const auto configOption = QCommandLineOption{{"c", "config"},
                                                 "Set a configuration file for server.",
                                                 "file",
                                                 "config.json"};

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "Uses ai cup game server and client to train Q-Network continuously");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({configOption});
    parser.process(a);

    const auto configFile = parser.value(configOption);
    if (!QFile::exists(configFile)) {
        qFatal("Config file \"%s\" not found.",
               configFile.toStdString().c_str());
    }

    QProcess serverProcess;
    QProcess clientProcess;
    clientProcess.setProcessChannelMode(QProcess::ForwardedChannels);

    const auto tryStartSimulation = [&] {
        if (serverProcess.state() == QProcess::NotRunning) {
            if (currentEpisode % 50 != 0 && !lastEpisodeDir.isEmpty()) {
                QDir{lastEpisodeDir}.removeRecursively();
            }
            qInfo() << "Starting episode" << ++currentEpisode;
            const auto episodeStr = QString::number(currentEpisode).rightJustified(6, '0');
            lastEpisodeDir = QString{EPISODE_DIR} + episodeStr;
            QDir{}.mkdir(lastEpisodeDir);
            serverProcess.start("./bootstrap_server.sh", {configFile, episodeStr});
        }
    };

    QObject::connect(&serverProcess, &QProcess::errorOccurred, &a, [&](auto error) {
        if (error == QProcess::FailedToStart) {
            qFatal("Couldn't start server executable");
        }
    });
    QObject::connect(&clientProcess, &QProcess::errorOccurred, &a, [](auto error) {
        if (error == QProcess::FailedToStart) {
            qFatal("Couldn't start client executable");
        }
    });

    QObject::connect(&serverProcess, &QProcess::finished, &a, tryStartSimulation);

    tryStartSimulation();
    clientProcess.start("./bootstrap_client.sh", {"31001"});
    return a.exec();
}
