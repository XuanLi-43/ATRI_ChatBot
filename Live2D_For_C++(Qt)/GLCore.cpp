#include "LAppDelegate.hpp"
#include "LAppView.hpp"
#include "LAppPal.hpp"
#include "LAppLive2DManager.hpp"
#include <QTimer>
#include <QLabel>
#include <QLineEdit>
#include <QResizeEvent>
#include <QGraphicsOpacityEffect>
#include <QTextEdit>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QLayout>
#include <QThread>

#include "GLCore.h"
#include "DeepseekAPI.h"
#include "DeepseekSendRequest.h"
#include "TranslateJP.h"
#include "Vits.h"
#include "BackgroundMusic.h"
#include "LAppModel.hpp"
#include "TouchManager_Common.hpp"


GLCore::GLCore(QWidget* parent)
	: QOpenGLWidget(parent)
{
    setWindowTitle("ATRI-ChaBbot");

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    qDebug() << "OpenGL version:" << format.majorVersion() << "." << format.minorVersion();
    qDebug() << "Profile:" << format.profile();


    BackgroundMusic* bgMusic = new BackgroundMusic(this);

	QTimer* timer = new QTimer();
	connect(timer, &QTimer::timeout, this, [this]() {
		update();
		});
	timer->start((1.0 / 60.0) * 1000);

    this->setMouseTracking(true); 


    roleAudioBtn = new QPushButton(this);
    roleAudioBtn->setText("角色音频");
    clearMemoryBtn = new QPushButton(this);
    clearMemoryBtn->setText("清除记忆");

    // 应用与输入框统一的样式
    QString buttonStyle = R"(
    QPushButton {
        background-color: rgba(80, 80, 80, 140);
        color: white;
        border: 1px solid rgba(255, 255, 255, 50);
        border-radius: 16px;
        padding: 10px 20px;
        font-size: 16px;
    }
    QPushButton:hover {
        border: 1px solid rgba(255, 255, 255, 120);
    }
    QPushButton:pressed {
        background-color: rgba(100, 100, 100, 180);
    }
    )";
    roleAudioBtn->setStyleSheet(buttonStyle);
    clearMemoryBtn->setStyleSheet(buttonStyle);

    // 信号连接
    connect(roleAudioBtn, &QPushButton::clicked, this, [=]() {
        QWidget* audioWindow = new QWidget();
        audioWindow->setWindowTitle("角色音频设置");
        audioWindow->resize(420, 200);
        audioWindow->setAttribute(Qt::WA_DeleteOnClose);
        audioWindow->setWindowModality(Qt::ApplicationModal);

        QVBoxLayout* layout = new QVBoxLayout(audioWindow);

        // 输入框
        QLineEdit* vitsInput = new QLineEdit(audioWindow);
        vitsInput->setPlaceholderText("输入 VITS 的地址");
        vitsInput->setStyleSheet(R"(
        QLineEdit {
            background-color: rgba(80, 80, 80, 160);
            border: 1px solid rgba(255, 255, 255, 80);
            border-radius: 10px;
            padding: 6px 10px;
            font-size: 16px;
            color: white;
        }
        QLineEdit:focus {
            border: 1px solid rgba(255, 255, 255, 150);
        }
    )");
        layout->addWidget(vitsInput);

        // 按钮布局
        QHBoxLayout* btnLayout = new QHBoxLayout();
        QPushButton* enableBtn = new QPushButton("开启", audioWindow);
        QPushButton* disableBtn = new QPushButton("关闭", audioWindow);

        QString btnStyle = R"(
        QPushButton {
            background-color: rgba(80, 80, 80, 160);
            color: white;
            border: 1px solid rgba(255, 255, 255, 80);
            border-radius: 10px;
            padding: 6px 20px;
            font-size: 16px;
        }
        QPushButton:hover {
            border: 1px solid rgba(255, 255, 255, 150);
        }
        QPushButton:pressed {
            background-color: rgba(100, 100, 100, 180);
        }
    )";
        enableBtn->setStyleSheet(btnStyle);
        disableBtn->setStyleSheet(btnStyle);

        btnLayout->addWidget(enableBtn);
        btnLayout->addWidget(disableBtn);
        layout->addLayout(btnLayout);

        layout->addStretch();

        //按钮逻辑 
        connect(enableBtn, &QPushButton::clicked, this, [=]() {
            QString inputText = vitsInput->text().trimmed();
            if (inputText.isEmpty()) {
                QMessageBox::warning(audioWindow, "警告", "地址不能为空!");
                return;
            }

            ATRI_Sound = true;
            QMessageBox::information(audioWindow, "提示", "已发送地址");

            // 发送输入框的内容
            vits = new Vits(this);
            vits->GetIPList(inputText);
            qDebug() << "发送 VITS 地址：" << inputText;

            // 清空输入框
            vitsInput->clear();
            });

        connect(disableBtn, &QPushButton::clicked, this, [=]() {
            ATRI_Sound = false;
            QMessageBox::information(audioWindow, "提示", "已关闭语音模式");
            });

        audioWindow->show();
        });


    connect(clearMemoryBtn, &QPushButton::clicked, this, [=]() {
        qDebug() << "清除记忆被点击";
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this, "清除记忆",
            "您确定要清除记忆吗？\n当前的亚托莉会永远忘记你",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (reply == QMessageBox::Yes) {
            ATRI_Memory = false;
        }
        });

	userInputEdit = new QTextEdit(this);
	userInputEdit->installEventFilter(this);
	userInputEdit->setPlaceholderText("请输入您要发送的消息...");
	userInputEdit->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	userInputEdit->setStyleSheet(R"(
    QTextEdit {
        background-color: rgba(80, 80, 80, 140);
        color: white;
        border: 1px solid rgba(255, 255, 255, 50);
        padding: 12px;
        font-size: 18px;
        border-radius: 16px;
    }

    QTextEdit:focus {
        background-color: rgba(80, 80, 80, 140);  
        border: 1px solid rgba(255, 255, 255, 120);
    }

    QTextEdit::viewport {
        background-color: transparent;
        border: none;
    }

    )");


	apiChat = new DeepseekAPI(this);
    translateJP = new TranslateJP(this);

	connect(apiChat, &DeepseekAPI::SignalResponse, this, [this](QString& deepseekResponse) {
		qDebug() << "Chat请求:" << deepseekResponse;
        deepseekResponse.remove(QRegularExpression(R"(([\*][^*]+[\*])|([\(（][^\)）]*[\)）]))"));


        //当音频播放后，才开始回复
        if (ATRI_Sound) {
            responseDisplayed = true;
            userInputEdit->setPlainText("亚托莉正在思考中...");
            translateJP->GetResponseChineseString(deepseekResponse);
            translateJP->SendRequestTranslate();

            connect(translateJP, &TranslateJP::SignalJPString, this, [=]() {
                vits->SendRequestSound();
            });

            connect(vits, &Vits::SoundGenerated, this, [=]() {
            // 只有当音频生成完成时，才执行
            StartLipSync();
            userInputEdit->setPlainText(deepseekResponse);
            });

            //如果请求失败，则改变ATRI_sound为false
            connect(vits, &Vits::SoundRequestFailed, this, [=]() {
                ATRI_Sound = false;
                qWarning() << "音频请求失败，已关闭语音模式";
                userInputEdit->setPlainText(deepseekResponse);
            });
        }
        else {
            userInputEdit->setPlainText("亚托莉正在思考中...");
            userInputEdit->setPlainText(deepseekResponse);
            responseDisplayed = true; // 标记已显示
        }
		});

}

bool GLCore::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == userInputEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            // Shift+Enter → 换行，不拦截
            if (keyEvent->modifiers() & Qt::ShiftModifier)
                return false;

            QString currentText = userInputEdit->toPlainText().trimmed();
            if (currentText.isEmpty())
                return true;  // 阻止空输入触发换行

            // 如果正在显示模型回复，则清空输入框准备新的输入
            if (responseDisplayed)
            {
                userInputEdit->clear();
                responseDisplayed = false;
                return true;  // 拦截，不让 QTextEdit 换行
            }

            //根据 ATRI_Memory 状态调用不同的发送逻辑
            if (!ATRI_Memory)
            {
                apiChat->SendRequestChat(currentText);
                ATRI_Memory = true; // 进入记忆模式
            }
            else
            {
                apiChat->SendRequestMemory(currentText);
            }

            userInputEdit->clear();   
            return true;              
        }
    }

    // 默认处理其他事件
    return QOpenGLWidget::eventFilter(obj, event);
}


//槽函数，当按下回车键时，调用DeepseekAPI的SendRequestChat函数
void GLCore::UserSendRequest()
{
    QString userInput = userInputEdit->toPlainText().trimmed();
    if (userInput.isEmpty()) return;

    userInputEdit->clear();

    if (!ATRI_Memory)
    {
        // 第一次输入
        apiChat->SendRequestChat(userInput);
        ATRI_Memory = true;  // 激活记忆模式
    }
    else
    {
        // 之后输入只进入记忆
        apiChat->SendRequestMemory(userInput);
    }
}

void GLCore::resizeEvent(QResizeEvent* event)
{
	QOpenGLWidget::resizeEvent(event);

	int w = width();
	int h = height();

	int inputWidth = w * 0.7;   // 输入框宽度 
	int inputHeight = h / 3;        

	int x = (w - inputWidth) / 2; // 水平居中
	int y = h - inputHeight - 20; // 底部留出边距

	userInputEdit->setGeometry(x, y, inputWidth, inputHeight);

    //按钮布局
    int btnWidth = 120;
    int btnHeight = 48;
    int btnY = y + inputHeight - btnHeight;  // 底部对齐输入框
    int margin = 20;

    roleAudioBtn->setGeometry(x - btnWidth - margin, btnY, btnWidth, btnHeight);
    clearMemoryBtn->setGeometry(x + inputWidth + margin, btnY, btnWidth, btnHeight);

}


GLCore::~GLCore()
{
    LAppDelegate::GetInstance()->Release();
    LAppDelegate::ReleaseInstance();
}

void GLCore::initializeGL()
{
	LAppDelegate::GetInstance()->Initialize(this);
}

void GLCore::resizeGL(int w, int h)
{
	LAppDelegate::GetInstance()->resize(w, h);
}

void GLCore::paintGL()
{
	LAppDelegate::GetInstance()->update();
}

void GLCore::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isPressed = true;
        LAppDelegate::GetInstance()->GetView()->OnTouchesBegan(event->position().x(), event->position().y());
    }
    
    event->ignore();
}

void GLCore::mouseReleaseEvent(QMouseEvent* event) {

    if (event->button() == Qt::LeftButton) {
        isPressed = false;
        LAppDelegate::GetInstance()->GetView()->OnTouchesEnded(event->position().x(), event->position().y());
        update();
    }

    event->ignore();
}

void GLCore::mouseMoveEvent(QMouseEvent* event) {
    LAppDelegate::GetInstance()->GetView()->OnTouchesMoved(event->position().x(), event->position().y());
    update();
    event->ignore();
}

void GLCore::StartLipSync() {
    LAppLive2DManager::GetInstance()->StartLipSync("output.wav");
}
