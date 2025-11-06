#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QStringList>
#include "Vits.h"
#include "DeepseekSendRequest.h"

class Vits;

class TranslateJP  : public QObject
{
	Q_OBJECT

public:
	static QStringList& GetResponseChineseList();
	static QStringList& GetAPIKeyList();

	static void GetModelName(const QString& modelName);
	QString& setModelName();
	QString& setChatModel();

	void GetResponseChineseString(const QString& chineseString);
	void GetAPIkey(const QString& APIKey);
	void SendRequestTranslate();
	void ProcessResponse(const QByteArray& responseData);

	auto BuildTranslateJson();

signals:
	void SignalResponseJPString(QString responseChinese);

signals:
	void SignalJPString();

private slots:
	void HandleReply(QNetworkReply* reply);

public:
	TranslateJP(QObject *parent);
	~TranslateJP();

private:

	QString responseChinese_m;
	QString APIKey_m;

	static QString modelName_m;
	static QString chatModel_m;

	static QStringList responseChineseList_m;
	static QStringList apiKeyList_m; //从开头获取的API Key

	Vits* vits;

	QNetworkAccessManager* managerTranslate;

};

