#include "BackgroundMusic.h"
#include <QDir>
#include <QRandomGenerator>
#include <QDebug>
#include <QMediaPlayer>

BackgroundMusic::BackgroundMusic(QObject* parent)
    : QObject(parent)
{
    // 初始化播放器
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.4);

    // 读取本地音乐文件夹
    QDir musicDir("music");
    QStringList files = musicDir.entryList(QStringList() << "*.mp3" << "*.wav", QDir::Files);
    for (const QString& file : files)
        playlist << musicDir.filePath(file);

    if (playlist.isEmpty()) {
        qWarning() << "未找到任何音乐文件，请在程序目录下创建 music 文件夹并放入 mp3 或 wav 文件。";
        return;
    }

    // 当一首播放结束时，自动播放下一首
    connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            playRandom();  // 无限循环
        }
        });

    // 开始播放
    playRandom();
}

BackgroundMusic::~BackgroundMusic()
{
    player->stop();
    player->deleteLater();
    audioOutput->deleteLater();
}

void BackgroundMusic::playRandom()
{
    if (playlist.isEmpty()) return;

    int index = QRandomGenerator::global()->bounded(playlist.size());
    QString file = playlist.at(index);
    qDebug() << "正在播放背景音乐:" << file;

    player->setSource(QUrl::fromLocalFile(file));
    player->play();
}
