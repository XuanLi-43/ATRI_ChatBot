#include "TranslateJP.h"
#include "GLCore.h"
#include "DeepseekAPI.h"
#include "DeepseekSendRequest.h"
#include "Vits.h"

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>
#include <QStringList>
#include <regex>

//接收中文字符串
//转换为json格式
//发送json格式数据到服务器
//接收服务器返回的json格式数据,  翻译成日语的
//解析json数据
//返回翻译后的日语字符串

QStringList TranslateJP::responseChineseList_m;
QStringList TranslateJP::apiKeyList_m;
QString TranslateJP::modelName_m;
QString TranslateJP::chatModel_m;

TranslateJP::TranslateJP(QObject *parent)
	: QObject(parent)
{
	managerTranslate = new QNetworkAccessManager(this);
	connect(managerTranslate, &QNetworkAccessManager::finished, this, &TranslateJP::HandleReply);

}

TranslateJP::~TranslateJP()
{}

//获取模型api地址，deepseek和通义千问
void TranslateJP::GetModelName(const QString& modelName) {
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

QString& TranslateJP::setModelName() {
	qDebug() << "模型名称：" << modelName_m;
	return modelName_m;
}

QString& TranslateJP::setChatModel() {
	qDebug() << "聊天模型" << chatModel_m;
	return chatModel_m;
}

//接收中文字符串
void TranslateJP::GetResponseChineseString(const QString& chineseString) {
	//添加翻译的需求
	QString needAddString = chineseString;
	QString stringNeed = "。翻译成日语。";
	QString combineString = stringNeed + needAddString;
	this->responseChinese_m = combineString;
	combineString.remove(QRegularExpression(R"(([\*][^*]+[\*])|([\(（][^\)）]*[\)）]))"));
	responseChineseList_m.clear();
	responseChineseList_m.append(combineString);
	qDebug() << "发送的中文字符串为：" << combineString;
}

QStringList& TranslateJP::GetResponseChineseList() {
	return responseChineseList_m;
}

//构建要求的json格式
auto TranslateJP::BuildTranslateJson() {
	QJsonObject jsonDataTranslate;
	jsonDataTranslate["model"] = chatModel_m;
	qDebug() << "选择的模型：" << chatModel_m;

	QJsonArray messageArrayTranslate;

	//发送响应的中文字符串
	QJsonObject messageTranslate;
	messageTranslate["role"] = "user";
	responseChineseList_m = GetResponseChineseList();
	QString chineseString = responseChineseList_m.first();
	messageTranslate["content"] = chineseString;
	messageArrayTranslate.append(messageTranslate);
	jsonDataTranslate["messages"] = messageArrayTranslate;

	QJsonDocument jsonDocTranslate(jsonDataTranslate);
	QByteArray ChineseJsonData = jsonDocTranslate.toJson(QJsonDocument::Compact);
	qDebug() << "发送的json数据为：" << ChineseJsonData;
	return ChineseJsonData;
}

//获取全局api key
void TranslateJP::GetAPIkey(const QString& APIKey) {
	this->APIKey_m = APIKey;
	apiKeyList_m.append(APIKey);
}

QStringList& TranslateJP::GetAPIKeyList() {
	return apiKeyList_m;
}

//发送请求
void TranslateJP::SendRequestTranslate() {

	//选择模型调整
	const QString apiUrl = modelName_m;
	qDebug() << "选择的api地址：" << apiUrl;

	apiKeyList_m = GetAPIKeyList();
	QString APIKey = apiKeyList_m.first();

	QNetworkRequest request;
	request.setUrl(QUrl(apiUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("Authorization", ("Bearer " + APIKey).toUtf8());

	auto ChineseJsonData = TranslateJP::BuildTranslateJson();
	managerTranslate->post(request, ChineseJsonData);
}

//槽函数，处理请求完成返回的响应
void TranslateJP::HandleReply(QNetworkReply* reply) {
	//检查连接状态
	if (reply->error() != QNetworkReply::NoError) {
		qWarning() << "错误: " << reply->errorString();
		reply->deleteLater();
		return;
	}

	//响应获取成功
	QByteArray responseData = reply->readAll();
	qDebug() << "响应数据: " << responseData;

	ProcessResponse(responseData);
}

void TranslateJP::ProcessResponse(const QByteArray& responseData) {
	QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

	if (!jsonResponse.isObject()) {
		qWarning() << "错误: 响应数据格式错误";
		return;
	}

	QJsonObject jsonObject = jsonResponse.object();
	if (jsonObject.contains("choices") && jsonObject["choices"].isArray()) {
		QJsonArray choices = jsonObject["choices"].toArray();

		if (!choices.isEmpty()) {
			QString JPResponse = choices[0]
				.toObject()
				.value("message")
				.toObject()
				.value("content")
				.toString();

			qDebug() << "日语翻译结果: " << JPResponse;
			emit SignalResponseJPString(JPResponse);

			//发送给vits.cpp
			vits = new Vits(this);
			vits->GetResonseJPString(JPResponse);
			//发送日语生成的信号
			emit SignalJPString();
			return;
		}
	}
	qWarning() << "错误: 响应数据中没有有效的choices";
}