#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>

class DeepseekAPI;
class QLineEdit;  
class QTextEdit;
class TranslateJP;
class Vits;

class GLCore : public QOpenGLWidget
{
public:
	GLCore(QWidget* parent = nullptr);
	~GLCore();

public:
	void resizeEvent(QResizeEvent* event);
	void UserSendRequest();

protected:
	bool eventFilter(QObject* obj, QEvent* event) override;
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void StartLipSync();

private:
	bool responseDisplayed = false;
	bool ATRI_Memory = false; //如果为true，则执行记忆页函数
	bool ATRI_Sound = false; //如果为true则开启唇音同步
	QTextEdit* userInputEdit;
	QPushButton* roleAudioBtn;
	QPushButton* clearMemoryBtn;
	DeepseekAPI* apiChat;
	TranslateJP* translateJP;
	Vits* vits;

private:
	bool isPressed = false;
};