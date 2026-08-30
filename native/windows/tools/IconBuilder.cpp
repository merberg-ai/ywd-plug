#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QVector>
#include <QtEndian>

#include <cstdint>

namespace {

void appendU16(QByteArray &out, quint16 value)
{
    const quint16 le = qToLittleEndian(value);
    out.append(reinterpret_cast<const char *>(&le), sizeof(le));
}

void appendU32(QByteArray &out, quint32 value)
{
    const quint32 le = qToLittleEndian(value);
    out.append(reinterpret_cast<const char *>(&le), sizeof(le));
}

QByteArray buildClassicDib(const QImage &source, int size)
{
    QImage canvas(size, size, QImage::Format_ARGB32);
    canvas.fill(Qt::transparent);

    const QImage scaled = source.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    {
        QPainter painter(&canvas);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        const QPoint topLeft((size - scaled.width()) / 2, (size - scaled.height()) / 2);
        painter.drawImage(topLeft, scaled);
    }

    const int xorBytes = size * size * 4;
    const int maskStride = ((size + 31) / 32) * 4;
    const int maskBytes = maskStride * size;

    QByteArray dib;
    dib.reserve(40 + xorBytes + maskBytes);

    appendU32(dib, 40);                       // BITMAPINFOHEADER size
    appendU32(dib, static_cast<quint32>(size));
    appendU32(dib, static_cast<quint32>(size * 2)); // XOR + AND mask height
    appendU16(dib, 1);                        // planes
    appendU16(dib, 32);                       // bits per pixel
    appendU32(dib, 0);                        // BI_RGB
    appendU32(dib, static_cast<quint32>(xorBytes));
    appendU32(dib, 0);                        // x pixels/meter
    appendU32(dib, 0);                        // y pixels/meter
    appendU32(dib, 0);                        // colors used
    appendU32(dib, 0);                        // important colors

    // Classic ICO DIB pixels are bottom-up BGRA.
    for (int y = size - 1; y >= 0; --y) {
        for (int x = 0; x < size; ++x) {
            const QRgb pixel = canvas.pixel(x, y);
            dib.append(static_cast<char>(qBlue(pixel)));
            dib.append(static_cast<char>(qGreen(pixel)));
            dib.append(static_cast<char>(qRed(pixel)));
            dib.append(static_cast<char>(qAlpha(pixel)));
        }
    }

    // Alpha already carries transparency; keep the legacy AND mask clear.
    dib.append(QByteArray(maskBytes, '\0'));
    return dib;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QStringList args = app.arguments();
    if (args.size() != 3) {
        qCritical("usage: ywd_icon_builder <source-png> <output-ico>");
        return 2;
    }

    const QString inputPath = args.at(1);
    const QString outputPath = args.at(2);

    QImage source(inputPath);
    if (source.isNull()) {
        qCritical("failed to load source image: %s", qPrintable(inputPath));
        return 3;
    }

    const QVector<int> sizes {16, 20, 24, 32, 40, 48, 64, 96, 128, 256};
    QVector<QByteArray> frames;
    frames.reserve(sizes.size());
    for (const int size : sizes) {
        frames.append(buildClassicDib(source, size));
    }

    QFile output(outputPath);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical("failed to create output icon: %s", qPrintable(outputPath));
        return 4;
    }

    QByteArray header;
    appendU16(header, 0); // reserved
    appendU16(header, 1); // icon type
    appendU16(header, static_cast<quint16>(sizes.size()));

    quint32 dataOffset = static_cast<quint32>(6 + (16 * sizes.size()));
    for (int i = 0; i < sizes.size(); ++i) {
        const int size = sizes.at(i);
        const quint8 dimension = size >= 256 ? 0 : static_cast<quint8>(size);
        header.append(static_cast<char>(dimension)); // width
        header.append(static_cast<char>(dimension)); // height
        header.append('\0');                        // palette entries
        header.append('\0');                        // reserved
        appendU16(header, 1);                        // planes
        appendU16(header, 32);                       // bpp
        appendU32(header, static_cast<quint32>(frames.at(i).size()));
        appendU32(header, dataOffset);
        dataOffset += static_cast<quint32>(frames.at(i).size());
    }

    if (output.write(header) != header.size()) {
        qCritical("failed writing ICO header");
        return 5;
    }
    for (const QByteArray &frame : frames) {
        if (output.write(frame) != frame.size()) {
            qCritical("failed writing ICO image frame");
            return 6;
        }
    }

    output.close();
    return 0;
}
