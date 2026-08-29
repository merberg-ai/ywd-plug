#include "WindowStateManager.h"

#include <QGuiApplication>
#include <QScreen>
#include <QSettings>
#include <QTimer>

namespace {
constexpr auto geometryKey = "window/normalGeometry";
constexpr auto visibilityKey = "window/visibility";

bool isPersistableVisibility(QWindow::Visibility visibility)
{
    return visibility == QWindow::Windowed
        || visibility == QWindow::Maximized;
}
}

WindowStateManager::WindowStateManager(QObject *parent)
    : QObject(parent)
{
}

void WindowStateManager::attach(QWindow *window)
{
    if (!window || m_window == window) {
        return;
    }

    m_window = window;

    QSettings settings;
    const QRect savedGeometry = settings.value(QString::fromLatin1(geometryKey)).toRect();
    m_normalGeometry = validatedGeometry(savedGeometry);
    if (m_normalGeometry.isValid()) {
        m_window->setGeometry(m_normalGeometry);
    } else {
        m_normalGeometry = m_window->geometry();
    }

    const auto savedVisibility = static_cast<QWindow::Visibility>(
        settings.value(QString::fromLatin1(visibilityKey), static_cast<int>(QWindow::Windowed)).toInt());
    m_restoreVisibility = isPersistableVisibility(savedVisibility) ? savedVisibility : QWindow::Windowed;
    m_lastVisibility = m_restoreVisibility;

    const auto captureIfWindowed = [this] {
        if (!m_started || !m_window || m_window->visibility() != QWindow::Windowed) {
            return;
        }
        captureNormalGeometry();
        persist();
    };

    connect(m_window, &QWindow::xChanged, this, captureIfWindowed);
    connect(m_window, &QWindow::yChanged, this, captureIfWindowed);
    connect(m_window, &QWindow::widthChanged, this, captureIfWindowed);
    connect(m_window, &QWindow::heightChanged, this, captureIfWindowed);

    connect(m_window, &QWindow::visibilityChanged, this, [this](QWindow::Visibility visibility) {
        if (!m_started || !m_window || !isPersistableVisibility(visibility)) {
            return;
        }

        m_lastVisibility = visibility;

        if (visibility == QWindow::Windowed) {
            QTimer::singleShot(0, this, [this] {
                captureNormalGeometry();
                persist();
            });
            return;
        }

        persist();
    });

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this] {
        if (m_started) {
            persist();
        }
    });
}

void WindowStateManager::showRestored()
{
    if (!m_window || m_started) {
        return;
    }

    m_started = true;

    switch (m_restoreVisibility) {
    case QWindow::Maximized:
        m_window->showMaximized();
        break;
    case QWindow::Windowed:
    default:
        m_window->showNormal();
        break;
    }
}

void WindowStateManager::captureNormalGeometry()
{
    if (!m_window || m_window->visibility() != QWindow::Windowed) {
        return;
    }

    const QRect geometry = m_window->geometry();
    if (geometry.width() >= m_window->minimumWidth()
        && geometry.height() >= m_window->minimumHeight()) {
        m_normalGeometry = geometry;
    }
}

void WindowStateManager::persist()
{
    if (!m_window) {
        return;
    }

    QSettings settings;
    if (m_normalGeometry.isValid()) {
        settings.setValue(QString::fromLatin1(geometryKey), m_normalGeometry);
    }
    settings.setValue(QString::fromLatin1(visibilityKey), static_cast<int>(m_lastVisibility));
    settings.sync();
}

QRect WindowStateManager::validatedGeometry(const QRect &candidate) const
{
    if (!candidate.isValid() || candidate.width() < 640 || candidate.height() < 480) {
        return {};
    }

    for (QScreen *screen : QGuiApplication::screens()) {
        if (!screen) {
            continue;
        }

        const QRect visiblePart = candidate.intersected(screen->availableGeometry());
        if (visiblePart.width() >= 120 && visiblePart.height() >= 80) {
            return candidate;
        }
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return {};
    }

    const QRect available = screen->availableGeometry();
    QSize size = candidate.size();
    size.setWidth(qMin(size.width(), available.width()));
    size.setHeight(qMin(size.height(), available.height()));

    QRect centered(QPoint(0, 0), size);
    centered.moveCenter(available.center());
    return centered;
}
