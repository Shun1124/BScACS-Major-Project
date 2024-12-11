#include <aws/core/Aws.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "settings.h"

#ifdef Q_OS_MAC
#include "MacOSMonitoring.h"
#elif defined(Q_OS_WIN)
#include "WindowsMonitoring.h"
#else
#error "Unsupported platform!"
#endif

int main(int argc, char *argv[]) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    Settings settings;

#ifdef Q_OS_MAC
    MacOSMonitoring monitoring(&settings);
#elif defined(Q_OS_WIN)
    WindowsMonitoring monitoring(&settings);
#endif

    engine.rootContext()->setContextProperty("Settings", &settings);
    engine.rootContext()->setContextProperty("Monitoring", &monitoring);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Monitor/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        Aws::ShutdownAPI(options);
        return -1;
    }

    QObject::connect(&monitoring, &MacOSMonitoring::logMessage, &engine, [&engine](const QString &message) {
        QObject *rootObject = engine.rootObjects().first();
        if (rootObject) {
            QMetaObject::invokeMethod(rootObject, "addLog", Q_ARG(QVariant, message));
        }
    });

    int result = app.exec();
    Aws::ShutdownAPI(options);
    return result;
}
