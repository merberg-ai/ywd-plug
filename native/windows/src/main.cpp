#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QTimer>
#include <QUrl>
#include <QWindow>

#include "app/AppController.h"
#include "app/WindowsChrome.h"
#include "app/WindowStateManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("KJ6YWD"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kj6ywd.net"));
    QCoreApplication::setApplicationName(QStringLiteral("YWD-Plug"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0-dev"));

    const QString brandingImageResource = QStringLiteral(":/branding/ywd-plug-win-logo1.png");
    const QString brandingIconResource = QStringLiteral(":/branding/ywd-plug-win.ico");
    const QUrl brandingImageUrl(QStringLiteral("qrc:/branding/ywd-plug-win-logo1.png"));

    QGuiApplication::setWindowIcon(QIcon(brandingIconResource));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AppController controller;
    WindowStateManager windowState;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);
    engine.rootContext()->setContextProperty(QStringLiteral("windowState"), &windowState);
    engine.rootContext()->setContextProperty(QStringLiteral("brandingImageUrl"), brandingImageUrl);
    engine.rootContext()->setContextProperty(QStringLiteral("brandingImagePath"), brandingImageResource);
    engine.rootContext()->setContextProperty(QStringLiteral("brandingImageExists"), QFile::exists(brandingImageResource));
    engine.rootContext()->setContextProperty(QStringLiteral("brandingIconPath"), brandingIconResource);
    engine.rootContext()->setContextProperty(QStringLiteral("brandingIconExists"), QFile::exists(brandingIconResource));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("YWDPlug"), QStringLiteral("Main"));

    if (!engine.rootObjects().isEmpty()) {
        if (auto *window = qobject_cast<QWindow *>(engine.rootObjects().constFirst())) {
            windowState.attach(window);
            WindowsChrome::applyTerminalChrome(window);

            QObject::connect(window, &QWindow::visibilityChanged, window, [window](QWindow::Visibility visibility) {
                if (visibility == QWindow::Hidden) {
                    return;
                }
                QTimer::singleShot(0, window, [window] {
                    WindowsChrome::applyTerminalChrome(window);
                });
            });
        }
    }

    return app.exec();
}
