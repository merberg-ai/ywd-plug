#include "WindowsChrome.h"

#include <QWindow>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

namespace WindowsChrome {
void applyTerminalChrome(QWindow *window)
{
    if (!window) {
        return;
    }

#ifdef Q_OS_WIN
    // Force creation of the native HWND before applying DWM attributes.
    const auto hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) {
        return;
    }

    // Windows 10/11 immersive dark title bar. Older builds simply ignore
    // unsupported DWM attributes.
    BOOL darkMode = TRUE;
    constexpr DWORD useImmersiveDarkMode = 20;
    DwmSetWindowAttribute(hwnd, useImmersiveDarkMode, &darkMode, sizeof(darkMode));

    constexpr DWORD borderColorAttribute = 34;   // DWMWA_BORDER_COLOR
    constexpr DWORD captionColorAttribute = 35;  // DWMWA_CAPTION_COLOR
    constexpr DWORD textColorAttribute = 36;     // DWMWA_TEXT_COLOR

    const COLORREF borderColor = RGB(66, 70, 74);
    const COLORREF captionColor = RGB(7, 8, 9);
    const COLORREF textColor = RGB(211, 215, 219);

    DwmSetWindowAttribute(hwnd, borderColorAttribute, &borderColor, sizeof(borderColor));
    DwmSetWindowAttribute(hwnd, captionColorAttribute, &captionColor, sizeof(captionColor));
    DwmSetWindowAttribute(hwnd, textColorAttribute, &textColor, sizeof(textColor));
#else
    Q_UNUSED(window)
#endif
}
} // namespace WindowsChrome
