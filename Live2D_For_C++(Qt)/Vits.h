#pragma once

#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QEventLoop>
#include <QDebug>
#include <QStringList>
#include <QMediaPlayer>
#include <QAudioOutput>


class Vits  : public QObject
{
	Q_OBJECT

public:
	static QStringList& GetResposeJPList();
	static QStringList& GetIP();

	void GetResonseJPString(const QString& JPString);
	void GetIPList(const QString& IP);
	void SendRequestSound();
	void HandleReply(QNetworkReply* reply);

public:
	Vits(QObject *parent);
	~Vits();

signals:
	void SoundGenerated();  // 音频生成完成信号
	void SoundRequestFailed();    // 请求失败

private:
	QString responseJP_m;
	QString vitsIP_m;

	static QStringList responseJPList_m;
	static QStringList ipList_m;

	bool isRequestingSound = false;

	QNetworkAccessManager* managerSound;

};

