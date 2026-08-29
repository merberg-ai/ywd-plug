#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "app/AppController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("KJ6YWD"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kj6ywd.net"));
    QCoreApplication::setApplicationName(QStringLiteral("YWD-Plug"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0-dev"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AppController controller;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("YWDPlug"), QStringLiteral("Main"));
    return app.exec();
}
