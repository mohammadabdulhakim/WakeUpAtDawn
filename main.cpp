#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // الطريقة السهلة: تحميل QSS من نفس مجلد البرنامج
    QFile styleFile(":/styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style);
        qDebug() << "✅ Loaded style.qss successfully!";
    } else {
        qDebug() << "❌ Failed to load style.qss!";
    }

    MainWindow w;
    w.show();

    return a.exec();
}
