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

    // Windows 10/11 immersive dark title bar. Attribute 20 is the current
    // DWMWA_USE_IMMERSIVE_DARK_MODE value; older builds simply ignore it.
    BOOL darkMode = TRUE;
    constexpr DWORD useImmersiveDarkMode = 20;
    DwmSetWindowAttribute(hwnd, useImmersiveDarkMode, &darkMode, sizeof(darkMode));

    // Windows 11 caption/border/text colors. Unsupported Windows builds ignore
    // these attributes and retain the dark-mode fallback above.
    constexpr DWORD borderColorAttribute = 34;  // DWMWA_BORDER_COLOR
    constexpr DWORD captionColorAttribute = 35; // DWMWA_CAPTION_COLOR
    constexpr DWORD textColorAttribute = 36;    // DWMWA_TEXT_COLOR

    const COLORREF borderColor = RGB(92, 61, 0);
    const COLORREF captionColor = RGB(5, 5, 2);
    const COLORREF textColor = RGB(255, 176, 0);

    DwmSetWindowAttribute(hwnd, borderColorAttribute, &borderColor, sizeof(borderColor));
    DwmSetWindowAttribute(hwnd, captionColorAttribute, &captionColor, sizeof(captionColor));
    DwmSetWindowAttribute(hwnd, textColorAttribute, &textColor, sizeof(textColor));
#else
    Q_UNUSED(window)
#endif
}
} // namespace WindowsChrome
