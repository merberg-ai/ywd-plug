#pragma once

#include <QObject>
#include <QPointer>
#include <QRect>
#include <QWindow>

class WindowStateManager final : public QObject
{
    Q_OBJECT

public:
    explicit WindowStateManager(QObject *parent = nullptr);

    void attach(QWindow *window);
    Q_INVOKABLE void showRestored();

private:
    void captureNormalGeometry();
    void persist();
    QRect validatedGeometry(const QRect &candidate) const;

    QPointer<QWindow> m_window;
    QRect m_normalGeometry;
    QWindow::Visibility m_restoreVisibility {QWindow::Windowed};
    QWindow::Visibility m_lastVisibility {QWindow::Windowed};
    bool m_started {false};
};
