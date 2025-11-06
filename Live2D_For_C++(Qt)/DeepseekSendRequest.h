#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QString>

#include "TranslateJP.h"

class QLabel;
class QLineEdit;
class QPushButton;
class DeepseekAPI;
class TranslateJP;

class DeepseekSendRequest  : public QDialog
{
	Q_OBJECT

public:
	DeepseekSendRequest(QDialog* parent = nullptr);
	~DeepseekSendRequest();

protected:
	auto ReadSetting();
	void paintEvent(QPaintEvent* event);


private slots:
	void onSendRequest();  // 按钮点击的槽函数

private:
	bool requestInProgress = false;  // 请求进行中标志

	QLineEdit* inputEdit;
	DeepseekAPI* apiLogin;
	TranslateJP* apiTranslate;

	QPushButton* deepseekModelBtn;
	QPushButton* qwenModelBtn;
};