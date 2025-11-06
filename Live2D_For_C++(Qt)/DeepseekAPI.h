#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QStringList>

class QNetworkReply;
class DeepseekSendRequest;

class DeepseekAPI  : public QObject
{
	Q_OBJECT


public:
	static QStringList& GetAPIKeyList();
	static QStringList& GetDeepseekSettingList();

	static void GetModelName(const QString& modelName);
	QString& setModelName();
	QString& setChatModel();

	auto BuildDeepseekJson();
	auto BuildDeepseekJson(QString& deepseekSettingUser, QString& userInput);
	auto BuildDeepseekJsonMemory(QString& userInputUser);


	void GetDeepseekSetting(const QString& deepseekSetting);
	void SendRequestLogin(const QString& APIKey);
	void GetAPIkey(const QString& APIKey);
	void SendRequestChat(QString& userInput);
	void SendRequestMemory(QString& userInput);


public:
	DeepseekAPI(QObject *parent);
	~DeepseekAPI();

signals:
	void SignalResponse(QString& deepseekResponse);

signals:
	void requestFailed(const QString& errorMsg);

private slots:
	void HandleReply(QNetworkReply* reply);

private:
	QString APIKey_m;
	QString deepseekSetting_m;

	static QStringList apiKeyList_m;  // 静态共享成员
	static QStringList deepseekSettingList_m;  

	static QString modelName_m;
	static QString chatModel_m;

	QString lastUserInput;
	QList<QPair<QString, QString>> chatHistory; // 保存多轮对话，<user, assistant>

	QNetworkAccessManager* managerLogin;
	QNetworkAccessManager* managerUser;



	void ProcessResponse(const QByteArray& responseData);
};

