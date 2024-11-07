#include <aws/core/Aws.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "monitoring.h"
#include "settings.h"

int main(int argc, char *argv[]) { // Main entry point for the application
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    QGuiApplication app(argc, argv); //

    QQmlApplicationEngine engine; // Creates the QML application engine for loading QML files
    Settings settings;
    Monitoring monitoring(&settings);

    // Expose the settings and monitoring objects to QML
    engine.rootContext()->setContextProperty("Settings", &settings);
    engine.rootContext()->setContextProperty("Monitoring", &monitoring);
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/Monitor/Main.qml")));

    QObject::connect(&monitoring, &Monitoring::logMessage, &engine, [&engine](const QString &message) { // Connects the logMessage signal from Monitoring to a lambda function
        QObject *rootObject = engine.rootObjects().first();
        QMetaObject::invokeMethod(rootObject, "addLog", Q_ARG(QVariant, message));
    });

    int result = app.exec(); // Starts the event loop and waits until the application is closed
    Aws::ShutdownAPI(options);
    return result;
}
