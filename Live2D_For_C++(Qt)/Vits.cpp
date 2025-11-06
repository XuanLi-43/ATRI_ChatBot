#include "Vits.h"
#include "TranslateJP.h"
#include "LAppModel.hpp"

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

// 接收日语的字符串
//调用vits api进行语音合成
// 保存合成后的语音文件
//将合成后的语音播放

QStringList Vits::responseJPList_m;
QStringList Vits::ipList_m;

Vits::Vits(QObject *parent)
	: QObject(parent)
{
	managerSound = new QNetworkAccessManager(this);
	connect(managerSound, &QNetworkAccessManager::finished, this, &Vits::HandleReply);
}

Vits::~Vits()
{}

//接收日文字符串
void Vits::GetResonseJPString(const QString& JPString) {
	this->responseJP_m = JPString;
	responseJPList_m.clear();
	responseJPList_m.append(JPString);
}

QStringList& Vits::GetResposeJPList() {
	return responseJPList_m;
}

//接收ip
void Vits::GetIPList(const QString& IP) {
	this->vitsIP_m = IP;	
	ipList_m.clear();
	ipList_m.append(IP);
}

QStringList& Vits::GetIP() {
	return ipList_m;
}

//发送请求，填入的ip地址和日文
void Vits::SendRequestSound() {
	if (isRequestingSound) {
		qDebug() << "已有请求正在进行，忽略重复发送";
		return;
	}

	QStringList& jpList = GetResposeJPList();
	QStringList& ipList = GetIP();

	if (jpList.isEmpty() || ipList.isEmpty()) {
		qWarning() << "VITS 参数未设置，跳过语音合成";
		return;
	}

	QString responseJP = jpList.first();
	QString vitsIP = ipList.first();

	QString encodedText = QUrl::toPercentEncoding(responseJP);
	QUrl vitsApiUrl(vitsIP + "/voice/vits?id=4&text=" + encodedText);

	QNetworkRequest request(vitsApiUrl);
	managerSound->get(request);

	isRequestingSound = true;  // 标记请求正在进行
	qDebug() << "已发送请求: " << vitsApiUrl.toString();
}

//槽函数，除了发送请求，还要处理返回的结果
void Vits::HandleReply(QNetworkReply* reply) {
	if (reply->error() != QNetworkReply::NoError) {
		qWarning() << "错误: " << reply->errorString();
		emit SoundRequestFailed();    // 请求失败
		reply->deleteLater();
		isRequestingSound = false;  // 重置状态
		return;
	}

	QByteArray audioData = reply->readAll();
	QString audioFileName = "output.wav";

	QFile file(audioFileName);
	if (file.open(QIODevice::WriteOnly)) {
		file.write(audioData);
		file.close();
		qDebug() << "音频已保存为" << audioFileName;
	}
	else {
		qWarning() << "无法创建文件";
		reply->deleteLater();
		isRequestingSound = false;
		return;
	}

	emit SoundGenerated();

	QMediaPlayer* player = new QMediaPlayer(this);
	QAudioOutput* audioOutput = new QAudioOutput(this);
	player->setAudioOutput(audioOutput);
	audioOutput->setVolume(1.0);
	player->setSource(QUrl::fromLocalFile(audioFileName));
	player->play();

	qDebug() << "正在播放音频...";

	connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
		if (status == QMediaPlayer::EndOfMedia || status == QMediaPlayer::InvalidMedia) {
			QFile::remove(audioFileName);
			qDebug() << "播放完成，音频文件已删除：" << audioFileName;
			player->deleteLater();
			audioOutput->deleteLater();
		}
		});

	reply->deleteLater();
	isRequestingSound = false;  // 重置状态，允许下一次请求
}
