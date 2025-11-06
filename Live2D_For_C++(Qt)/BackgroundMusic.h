#pragma once
#include <QObject>
#include <QStringList>
#include <QMediaPlayer>
#include <QAudioOutput>

class BackgroundMusic : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundMusic(QObject* parent = nullptr);
    ~BackgroundMusic();

private:
    QStringList playlist;       // 音乐列表
    QMediaPlayer* player;
    QAudioOutput* audioOutput;

    void playRandom();          // 播放随机音乐
};
