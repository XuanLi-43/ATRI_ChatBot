#include "DeepseekAPI.h"
#include "DeepseekSendRequest.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QLabel>
#include <QStringList>
#include <regex>

QStringList DeepseekAPI::apiKeyList_m;
QStringList DeepseekAPI::deepseekSettingList_m;

QString DeepseekAPI::modelName_m;
QString DeepseekAPI::chatModel_m;

DeepseekAPI::DeepseekAPI(QObject *parent)
	: QObject(parent)
{
	managerLogin = new QNetworkAccessManager(this);
	managerUser = new QNetworkAccessManager(this);

	connect(managerLogin, &QNetworkAccessManager::finished, this, &DeepseekAPI::HandleReply);
	connect(managerUser, &QNetworkAccessManager::finished, this, &DeepseekAPI::HandleReply);
}

DeepseekAPI::~DeepseekAPI()
{}


//获取模型api地址，deepseek和通义千问
void DeepseekAPI::GetModelName(const QString& modelName) {
	if (modelName == "Deepseek") {
		const QString apiUrlDeepseek = "https://api.deepseek.com/v1/chat/completions";
		const QString chatModelDeepseek = "deepseek-chat";
		modelName_m = apiUrlDeepseek;
		chatModel_m = chatModelDeepseek;
	}
	else {
		const QString apiUrlQwen = "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions";
		const QString chatModelQwen = "qwen-plus";
		modelName_m = apiUrlQwen;
		chatModel_m = chatModelQwen;
	}
}

QString& DeepseekAPI::setModelName() {
	qDebug() << "模型名称：" << modelName_m;
	return modelName_m;
}

QString& DeepseekAPI::setChatModel() {
	qDebug() << "聊天模型" << chatModel_m;
	return chatModel_m;
}


//构建deepseek要求的json格式, 登录页
auto DeepseekAPI::BuildDeepseekJson() {
	QJsonObject jsonDataLogin;
	jsonDataLogin["model"] = chatModel_m;

	QJsonArray messageArrayLogin;

	//发送响应，接收响应
	QJsonObject systemMessageLogin;
	systemMessageLogin["role"] = "user";
	systemMessageLogin["content"] = "你好";
	messageArrayLogin.append(systemMessageLogin);

	jsonDataLogin["messages"] = messageArrayLogin;

	QJsonDocument jsonDocLogin(jsonDataLogin);
	QByteArray deepseekJsonDataLogin = jsonDocLogin.toJson(QJsonDocument::Compact);

	qDebug() << "Deepseek Json数据:" << deepseekJsonDataLogin;

	return deepseekJsonDataLogin;
}

//获取角色设置
void DeepseekAPI::GetDeepseekSetting(const QString& deepseekSetting) {
	this->deepseekSetting_m = deepseekSetting;
	deepseekSettingList_m.append(deepseekSetting);
	qDebug() << "收到的 deepseekSetting:" << deepseekSetting;
}

QStringList& DeepseekAPI::GetDeepseekSettingList() {
	return deepseekSettingList_m;
}

//构建符号deepseek要求的json格式, 聊天页
auto DeepseekAPI::BuildDeepseekJson(QString& deepseekSettingUser, QString& userInputUser) {
	QJsonObject jsonDataUser;
	jsonDataUser["model"] = chatModel_m;

	QJsonArray messageArrayUser;

	// system
	QJsonObject systemMessageUser;
	systemMessageUser["role"] = "system";
	systemMessageUser["content"] = deepseekSettingUser;
	messageArrayUser.append(systemMessageUser);
	qDebug() << "收到的 deepseekSettingUser:" << deepseekSettingUser;

	// user
	QJsonObject userMessageUser;
	userMessageUser["role"] = "user";
	userMessageUser["content"] = userInputUser;
	messageArrayUser.append(userMessageUser);

	jsonDataUser["messages"] = messageArrayUser;

	QJsonDocument jsonDocUser(jsonDataUser);
	QByteArray deepseekJsonDataUser = jsonDocUser.toJson(QJsonDocument::Compact);

	qDebug() << "Deepseek Json数据:" << deepseekJsonDataUser;

	return deepseekJsonDataUser;
}


//构建符号deepseek要求的json格式, 记忆页, 当为GLCore.cpp的ATRI_Memory true时，执行该函数
auto DeepseekAPI::BuildDeepseekJsonMemory(QString& userInputUser) {
	QJsonObject jsonData;
	jsonData["model"] = chatModel_m;

	QJsonArray messages;

	// system
	QJsonObject systemMessage;
	systemMessage["role"] = "system";
	deepseekSettingList_m = GetDeepseekSettingList();
	QString deepseekSettingUserMemory = deepseekSettingList_m.first();
	systemMessage["content"] = deepseekSettingUserMemory;
	messages.append(systemMessage);

	//加入历史上下文
	for (const auto& pair : chatHistory) {
		QJsonObject userMsg;
		userMsg["role"] = "user";
		userMsg["content"] = pair.first;
		messages.append(userMsg);

		QJsonObject assistantMsg;
		assistantMsg["role"] = "assistant";
		assistantMsg["content"] = pair.second;
		messages.append(assistantMsg);
	}

	// 当前输入
	QJsonObject currentUserMsg;
	currentUserMsg["role"] = "user";
	currentUserMsg["content"] = userInputUser;
	messages.append(currentUserMsg);

	jsonData["messages"] = messages;

	QJsonDocument jsonDoc(jsonData);
	QByteArray deepseekJsonData = jsonDoc.toJson(QJsonDocument::Compact);

	qDebug() << "Deepseek Memory Json数据:" << deepseekJsonData;

	return deepseekJsonData;
}

//发送请求, 登录页
void DeepseekAPI::SendRequestLogin(const QString& APIKey) {
	const QString apiUrl = modelName_m;

	QNetworkRequest request;
	request.setUrl(QUrl(apiUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", ("Bearer " + APIKey).toUtf8());

	auto deepseekJsonDataLogin = DeepseekAPI::BuildDeepseekJson();

	managerLogin->post(request, deepseekJsonDataLogin);
}

//获取全局api key
void DeepseekAPI::GetAPIkey(const QString& APIKey) {
	this->APIKey_m = APIKey;
	apiKeyList_m.append(APIKey);
}

QStringList& DeepseekAPI::GetAPIKeyList() {
	return apiKeyList_m;
}

//发送请求, 聊天页
void DeepseekAPI::SendRequestChat(QString& userInput) {
	const QString apiUrl = modelName_m;

	apiKeyList_m = GetAPIKeyList();
	QString APIKey = apiKeyList_m.first();

	deepseekSettingList_m = GetDeepseekSettingList();
	QString deepseekSetting = deepseekSettingList_m.first();
	qDebug() << "SendRequestChat收到的 deepseekSetting:" << deepseekSetting;

	QNetworkRequest request;
	request.setUrl(QUrl(apiUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", ("Bearer " + APIKey).toUtf8());

	auto deepseekJsonDataUser = DeepseekAPI::BuildDeepseekJson(deepseekSetting, userInput);

	managerUser->post(request, deepseekJsonDataUser);
	lastUserInput = userInput; // 保存当前输入
}

//发送请求, 记忆页
void DeepseekAPI::SendRequestMemory(QString& userInput) {
	const QString apiUrl = modelName_m;

	apiKeyList_m = GetAPIKeyList();
	QString APIKey = apiKeyList_m.first();

	deepseekSettingList_m = GetDeepseekSettingList();
	QString deepseekSetting = deepseekSettingList_m.first();
	qDebug() << "SendRequestChat收到的 deepseekSetting:" << deepseekSetting;

	QNetworkRequest request;
	request.setUrl(QUrl(apiUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", ("Bearer " + APIKey).toUtf8());

	auto deepseekJsonDataUserMemory = DeepseekAPI::BuildDeepseekJsonMemory(userInput);

	managerUser->post(request, deepseekJsonDataUserMemory);
	lastUserInput = userInput; // 保存当前输入
}

//槽函数，处理请求完成返回的响应
void DeepseekAPI::HandleReply(QNetworkReply* reply) {
	//检查连接状态
	if (reply->error() != QNetworkReply::NoError) {
		QString errorMsg = reply->errorString();
		qWarning() << "错误: " << errorMsg;
		emit requestFailed(errorMsg);
		reply->deleteLater();
		return;
	}

	//响应获取成功
	QByteArray responseData = reply->readAll();
	qDebug() << "响应数据: " << responseData;
	
	//提取信息
	ProcessResponse(responseData);
	reply->deleteLater();
}

//解析返回的数据
void DeepseekAPI::ProcessResponse(const QByteArray& responseData) {
	QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

	if (!jsonResponse.isObject()) {
		qWarning() << "错误: 响应数据格式错误";
		return;
	}

	QJsonObject jsonObject = jsonResponse.object();
	if (jsonObject.contains("choices") && jsonObject["choices"].isArray()) {
		QJsonArray choices = jsonObject["choices"].toArray();

		if (!choices.isEmpty()) {
			QString deepseekResponse = choices[0]
				.toObject()
				.value("message")
				.toObject()
				.value("content")
				.toString();

			qDebug() << "debug deepseek的回复:" << deepseekResponse;

			//保存上下文
			if (!lastUserInput.isEmpty()) {
				chatHistory.append(qMakePair(lastUserInput, deepseekResponse));
				// 限制记忆长度，防止 JSON 太大
				if (chatHistory.size() > 10)
					chatHistory.removeFirst();
			}

			emit SignalResponse(deepseekResponse);
			return;
		}
	}

	qWarning() << "错误: 响应数据中没有有效的choices";
}
