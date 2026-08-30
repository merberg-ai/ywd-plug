#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include <QQuickStyle>
#include <QTimer>
#include <QUrl>
#include <QWindow>

#include "app/AppController.h"
#include "app/WindowsChrome.h"
#include "app/WindowStateManager.h"

using Microsoft::WRL::ComPtr;

namespace {

class StaticBrandingImageProvider final : public QQuickImageProvider
{
public:
    explicit StaticBrandingImageProvider(QImage image)
        : QQuickImageProvider(QQuickImageProvider::Image)
        , m_image(std::move(image))
    {
    }

    QImage requestImage(
        const QString &id,
        QSize *size,
        const QSize &requestedSize) override
    {
        Q_UNUSED(id);

        if (size) {
            *size = m_image.size();
        }

        if (!requestedSize.isValid() || requestedSize.isEmpty()) {
            return m_image;
        }

        return m_image.scaled(
            requestedSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    }

private:
    QImage m_image;
};

QImage decodePngResourceWithWic(const QString &resourcePath, QString *errorMessage)
{
    QFile input(resourcePath);
    if (!input.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("embedded branding resource could not be opened");
        }
        return {};
    }

    QByteArray encoded = input.readAll();
    if (encoded.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("embedded branding resource is empty");
        }
        return {};
    }

    const HRESULT comHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool uninitializeCom = SUCCEEDED(comHr);
    if (FAILED(comHr) && comHr != RPC_E_CHANGED_MODE) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("COM initialization failed: 0x%1")
                                .arg(static_cast<qulonglong>(static_cast<unsigned long>(comHr)), 8, 16, QLatin1Char('0'));
        }
        return {};
    }

    QImage decoded;
    QString localError;

    {
        ComPtr<IWICImagingFactory> factory;
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&factory));

        if (FAILED(hr)) {
            localError = QStringLiteral("WIC factory creation failed: 0x%1")
                             .arg(static_cast<qulonglong>(static_cast<unsigned long>(hr)), 8, 16, QLatin1Char('0'));
        } else {
            ComPtr<IWICStream> stream;
            hr = factory->CreateStream(&stream);
            if (FAILED(hr)) {
                localError = QStringLiteral("WIC stream creation failed: 0x%1")
                                 .arg(static_cast<qulonglong>(static_cast<unsigned long>(hr)), 8, 16, QLatin1Char('0'));
            } else {
                hr = stream->InitializeFromMemory(
                    reinterpret_cast<BYTE *>(encoded.data()),
                    static_cast<DWORD>(encoded.size()));

                if (FAILED(hr)) {
                    localError = QStringLiteral("WIC stream initialization failed: 0x%1")
                                     .arg(static_cast<qulonglong>(static_cast<unsigned long>(hr)), 8, 16, QLatin1Char('0'));
                } else {
                    ComPtr<IWICBitmapDecoder> decoder;
                    hr = factory->CreateDecoderFromStream(
                        stream.Get(),
                        nullptr,
                        WICDecodeMetadataCacheOnLoad,
                        &decoder);

                    if (FAILED(hr)) {
                        localError = QStringLiteral("WIC PNG decoder creation failed: 0x%1")
                                         .arg(static_cast<qulonglong>(static_cast<unsigned long>(hr)), 8, 16, QLatin1Char('0'));
                    } else {
                        ComPtr<IWICBitmapFrameDecode> frame;
                        hr = decoder->GetFrame(0, &frame);
                        if (FAILED(hr)) {
                            localError = QStringLiteral("WIC PNG frame read failed: 0x%1")
                                             .arg(static_cast<qulonglong>(static_cast<unsigned long>(hr)), 8, 16, QLatin1Char('0'));
                        } else {
                            UINT width = 0;
                            UINT height = 0;
                            hr = frame->GetSize(&width, &height);

                            if (FAILED(hr) || width == 0 || height == 0) {
                                localError = QStringLiteral("WIC PNG dimensions are invalid");
                            } else {
                                ComPtr<IWICFormatConverter> converter;
                                hr = factory->CreateFormatConverter(&converter);

                                if (FAILED(hr)) {
                                    localError = QStringLiteral("WIC format converter creation failed: 0x%1")
                                                     .arg(static_cast<qulonglong>(static_cast<unsigned long>(hr)), 8, 16, QLatin1Char('0'));
                                } else {
                                    hr = converter->Initialize(
                                        frame.Get(),
                                        GUID_WICPixelFormat32bppBGRA,
                                        WICBitmapDitherTypeNone,
                                        nullptr,
                                        0.0,
                                        WICBitmapPaletteTypeCustom);

                                    if (FAILED(hr)) {
                                        localError = QStringLiteral("WIC BGRA conversion failed: 0x%1")
                                                         .arg(static_cast<qulonglong>(static_cast<unsigned long>(hr)), 8, 16, QLatin1Char('0'));
                                    } else {
                                        QImage image(
                                            static_cast<int>(width),
                                            static_cast<int>(height),
                                            QImage::Format_ARGB32);

                                        if (image.isNull()) {
                                            localError = QStringLiteral("could not allocate branding image buffer");
                                        } else {
                                            const UINT stride = static_cast<UINT>(image.bytesPerLine());
                                            const quint64 byteCount64 = static_cast<quint64>(stride) * height;

                                            if (byteCount64 > static_cast<quint64>(std::numeric_limits<UINT>::max())) {
                                                localError = QStringLiteral("branding image buffer is too large");
                                            } else {
                                                hr = converter->CopyPixels(
                                                    nullptr,
                                                    stride,
                                                    static_cast<UINT>(byteCount64),
                                                    image.bits());

                                                if (FAILED(hr)) {
                                                    localError = QStringLiteral("WIC pixel copy failed: 0x%1")
                                                                     .arg(static_cast<qulonglong>(static_cast<unsigned long>(hr)), 8, 16, QLatin1Char('0'));
                                                } else {
                                                    decoded = image;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (uninitializeCom) {
        CoUninitialize();
    }

    if (decoded.isNull() && errorMessage) {
        *errorMessage = localError.isEmpty()
                            ? QStringLiteral("unknown WIC branding decode failure")
                            : localError;
    }

    return decoded;
}

} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("KJ6YWD"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kj6ywd.net"));
    QCoreApplication::setApplicationName(QStringLiteral("YWD-Plug"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0-dev"));

    const QString brandingImageResource = QStringLiteral(":/branding/ywd-plug-win-logo1.png");
    const QString brandingIconResource = QStringLiteral(":/branding/ywd-plug-win.ico");

    QGuiApplication::setWindowIcon(QIcon(brandingIconResource));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QString brandingImageError;
    const QImage brandingImage = decodePngResourceWithWic(
        brandingImageResource,
        &brandingImageError);
    const bool brandingImageReady = !brandingImage.isNull();

    AppController controller;
    WindowStateManager windowState;
    QQmlApplicationEngine engine;

    if (brandingImageReady) {
        engine.addImageProvider(
            QStringLiteral("branding"),
            new StaticBrandingImageProvider(brandingImage));
    }

    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);
    engine.rootContext()->setContextProperty(QStringLiteral("windowState"), &windowState);
    engine.rootContext()->setContextProperty(
        QStringLiteral("brandingImageUrl"),
        QUrl(QStringLiteral("image://branding/logo")));
    engine.rootContext()->setContextProperty(
        QStringLiteral("brandingImageReady"),
        brandingImageReady);
    engine.rootContext()->setContextProperty(
        QStringLiteral("brandingImageError"),
        brandingImageError);
    engine.rootContext()->setContextProperty(
        QStringLiteral("brandingIconExists"),
        QFile::exists(brandingIconResource));

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
