#include "GLCore.h"
#include "DeepseekSendRequest.h"
#include <QDebug>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/platforms");
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon("Resources/ATRI.ico"));
    DeepseekSendRequest sendAPIwindow;
    if (sendAPIwindow.exec() == QDialog::Accepted) {
        GLCore mainwindow;
        mainwindow.resize(1600, 900);
        mainwindow.show();
        return app.exec();
    }
    QString audioFileName = "output.wav";
    QFile::remove(audioFileName);
    return 0;
}
