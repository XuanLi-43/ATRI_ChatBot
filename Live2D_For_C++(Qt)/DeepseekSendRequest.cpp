#include "DeepseekSendRequest.h"
#include "DeepseekAPI.h"
#include "TranslateJP.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>

DeepseekSendRequest::DeepseekSendRequest(QDialog* parent)
    : QDialog(parent)
{
    // 设置窗口标题和大小
    setWindowTitle("ATRI-ChatBot");
    setFixedSize(960, 600);

    // 创建标签
    QLabel* label = new QLabel("请输入API key", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 25px; padding: 5px; color: black;");

    // 输入框（加宽）
    inputEdit = new QLineEdit(this);
    inputEdit->setAlignment(Qt::AlignCenter);
    inputEdit->setFixedWidth(400);
    inputEdit->setFixedHeight(40);
    inputEdit->setStyleSheet(
        "background-color: rgba(200, 200, 200, 0.6); "
        "border: 2px solid gray; "
        "border-radius: 8px; "
        "font-size: 16px; color: black;"
    );

    // 发送请求按钮（加大）
    QPushButton* sendRequestButton = new QPushButton("发送请求", this);
    sendRequestButton->setFixedSize(200, 45);
    sendRequestButton->setStyleSheet(
        "background-color: rgba(169, 169, 169, 0.6); "
        "border: 2px solid gray; "
        "border-radius: 8px; "
        "color: black; font-size: 16px;"
    );

    // 输入框右侧两个模型按钮
    deepseekModelBtn = new QPushButton("Deepseek", this);
    qwenModelBtn = new QPushButton("通义千问", this);

    deepseekModelBtn->setFixedSize(120, 40);
    qwenModelBtn->setFixedSize(120, 40);

    QString normalStyle =
        "background-color: rgba(200, 200, 200, 0.5); "
        "border: 1px solid gray; "
        "color: black; border-radius: 6px; font-size: 15px;";
    QString selectedStyle =
        "background-color: rgba(70, 130, 180, 0.9); "
        "border: 2px solid white; "
        "color: white; border-radius: 6px; font-size: 15px;";

    deepseekModelBtn->setStyleSheet(selectedStyle); 
    qwenModelBtn->setStyleSheet(normalStyle);

    // 模型按钮垂直布局
    QVBoxLayout* modelBtnLayout = new QVBoxLayout;
    modelBtnLayout->addWidget(deepseekModelBtn);
    modelBtnLayout->addSpacing(10);
    modelBtnLayout->addWidget(qwenModelBtn);

    // 输入框与模型按钮的水平布局
    QHBoxLayout* inputWithButtonsLayout = new QHBoxLayout;
    inputWithButtonsLayout->addWidget(inputEdit, 0, Qt::AlignRight);
    inputWithButtonsLayout->addSpacing(10);
    inputWithButtonsLayout->addLayout(modelBtnLayout);
    inputWithButtonsLayout->setAlignment(Qt::AlignHCenter);

    // 主垂直布局
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addStretch(1);
    layout->addWidget(label);
    layout->addSpacing(30);
    layout->addLayout(inputWithButtonsLayout);
    layout->addSpacing(30);
    layout->addWidget(sendRequestButton, 0, Qt::AlignHCenter);
    layout->addStretch(1);

    setLayout(layout);

    // 创建 API 对象
    apiLogin = new DeepseekAPI(this);
    apiTranslate = new TranslateJP(this);

    //模型切换高亮逻辑 
    connect(deepseekModelBtn, &QPushButton::clicked, this, [=]() {
        deepseekModelBtn->setStyleSheet(selectedStyle);
        qwenModelBtn->setStyleSheet(normalStyle);
        QMessageBox::information(this, "模型切换", "已选择 Deepseek 模型");
        apiTranslate->GetModelName("Deepseek");
        apiLogin->GetModelName("Deepseek");
        });

    connect(qwenModelBtn, &QPushButton::clicked, this, [=]() {
        qwenModelBtn->setStyleSheet(selectedStyle);
        deepseekModelBtn->setStyleSheet(normalStyle);
        QMessageBox::information(this, "模型切换", "已选择 通义千问 模型");
        apiTranslate->GetModelName("Qwen");
        apiLogin->GetModelName("Qwen");
        });

    //默认
    apiTranslate->GetModelName("Deepseek");
    apiLogin->GetModelName("Deepseek");

    // 发送按钮逻辑
    connect(sendRequestButton, &QPushButton::clicked, this, &DeepseekSendRequest::onSendRequest);
}

auto DeepseekSendRequest::ReadSetting() {
    QString filePath = "Resources/atri_8_Setting.txt";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "打开文件失败:" << filePath << "错误:" << file.errorString();
        return QString();
    }
    QTextStream in(&file);
    QString deepseekSetting = in.readAll();
    file.close();
    return deepseekSetting;
}

void DeepseekSendRequest::onSendRequest() {
    if (requestInProgress) {
        QMessageBox::information(this, "提示", "请求正在进行中，请稍等");
        return;
    }

    QString apiKey = inputEdit->text();
    QString deepseekSetting = ReadSetting();
    qDebug() << "Deepseek Setting:" << deepseekSetting;

    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "API Key 不能为空");
        return;
    }

    requestInProgress = true; // 标记请求开始

    apiLogin->SendRequestLogin(apiKey);

    // 连接错误信号
    connect(apiLogin, &DeepseekAPI::requestFailed, this, [=](const QString& msg) {
        QMessageBox::warning(this, "连接错误", "请求失败：" + msg);
        requestInProgress = false; // 请求结束
        });

    // 连接正常响应信号
    connect(apiLogin, &DeepseekAPI::SignalResponse, this, [=](QString& deepseekResponse) {
        qDebug() << "请求:" << deepseekResponse;

        if (deepseekResponse.trimmed().isEmpty()) {
            requestInProgress = false; // 请求结束
            return;
        }

        QMessageBox::information(this, "请求成功", "请求成功, 稍等片刻");
        apiLogin->GetAPIkey(apiKey);
        apiLogin->GetDeepseekSetting(deepseekSetting);
        apiTranslate->GetAPIkey(apiKey);

        requestInProgress = false; // 请求结束
        this->accept();
        });
}

DeepseekSendRequest::~DeepseekSendRequest() {}

void DeepseekSendRequest::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.drawPixmap(rect(), QPixmap("Resources/bg017a.jpg"));
    QWidget::paintEvent(event);
}
