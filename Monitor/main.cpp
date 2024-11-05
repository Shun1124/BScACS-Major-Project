#include <aws/core/Aws.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "monitoring.h"
#include "settings.h"

int main(int argc, char *argv[]) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);  // Initialize the SDK

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    Settings settings;
    Monitoring monitoring(&settings);

    // Expose the settings and monitoring objects to QML
    engine.rootContext()->setContextProperty("Settings", &settings);
    engine.rootContext()->setContextProperty("Monitoring", &monitoring);
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Monitor/Main.qml")));

    QObject::connect(&monitoring, &Monitoring::logMessage, &engine, [&engine](const QString &message) {
        QObject *rootObject = engine.rootObjects().first();
        QMetaObject::invokeMethod(rootObject, "addLog", Q_ARG(QVariant, message));
    });

    int result = app.exec();
    Aws::ShutdownAPI(options);  // Shutdown the SDK
    return result;
}
