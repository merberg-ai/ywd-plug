#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {

constexpr std::array<UINT, 10> kIconSizes {16, 20, 24, 32, 40, 48, 64, 96, 128, 256};

void appendU16(std::vector<std::uint8_t> &out, std::uint16_t value)
{
    out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xFFu));
}

void appendU32(std::vector<std::uint8_t> &out, std::uint32_t value)
{
    out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xFFu));
}

bool failed(HRESULT hr, const wchar_t *what)
{
    if (SUCCEEDED(hr)) {
        return false;
    }

    std::wcerr << L"[XX] " << what << L" failed (HRESULT 0x"
               << std::hex << static_cast<unsigned long>(hr) << std::dec << L")\n";
    return true;
}

bool renderSquareIconFrame(
    IWICImagingFactory *factory,
    IWICBitmapFrameDecode *sourceFrame,
    UINT sourceWidth,
    UINT sourceHeight,
    UINT targetSize,
    std::vector<std::uint8_t> &squarePixels)
{
    if (!factory || !sourceFrame || sourceWidth == 0u || sourceHeight == 0u || targetSize == 0u) {
        return false;
    }

    const double scale = std::min(
        static_cast<double>(targetSize) / static_cast<double>(sourceWidth),
        static_cast<double>(targetSize) / static_cast<double>(sourceHeight));

    const UINT scaledWidth = std::max<UINT>(
        1u,
        static_cast<UINT>(static_cast<double>(sourceWidth) * scale + 0.5));
    const UINT scaledHeight = std::max<UINT>(
        1u,
        static_cast<UINT>(static_cast<double>(sourceHeight) * scale + 0.5));

    ComPtr<IWICBitmapScaler> scaler;
    if (failed(factory->CreateBitmapScaler(&scaler), L"Create WIC scaler")) {
        return false;
    }

    if (failed(
            scaler->Initialize(
                sourceFrame,
                scaledWidth,
                scaledHeight,
                WICBitmapInterpolationModeFant),
            L"Scale branding PNG")) {
        return false;
    }

    ComPtr<IWICFormatConverter> converter;
    if (failed(factory->CreateFormatConverter(&converter), L"Create WIC format converter")) {
        return false;
    }

    if (failed(
            converter->Initialize(
                scaler.Get(),
                GUID_WICPixelFormat32bppBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeCustom),
            L"Convert branding PNG to 32-bit BGRA")) {
        return false;
    }

    const UINT scaledStride = scaledWidth * 4u;
    std::vector<std::uint8_t> scaledPixels(
        static_cast<std::size_t>(scaledStride) * scaledHeight);

    if (failed(
            converter->CopyPixels(
                nullptr,
                scaledStride,
                static_cast<UINT>(scaledPixels.size()),
                scaledPixels.data()),
            L"Copy branding pixels")) {
        return false;
    }

    const UINT targetStride = targetSize * 4u;
    squarePixels.assign(
        static_cast<std::size_t>(targetStride) * targetSize,
        0u);

    const UINT offsetX = (targetSize - scaledWidth) / 2u;
    const UINT offsetY = (targetSize - scaledHeight) / 2u;

    for (UINT y = 0; y < scaledHeight; ++y) {
        const auto sourceOffset = static_cast<std::size_t>(y) * scaledStride;
        const auto targetOffset =
            static_cast<std::size_t>(offsetY + y) * targetStride
            + static_cast<std::size_t>(offsetX) * 4u;
        std::memcpy(
            squarePixels.data() + targetOffset,
            scaledPixels.data() + sourceOffset,
            scaledStride);
    }

    return true;
}

std::vector<std::uint8_t> buildClassicDib(
    const std::vector<std::uint8_t> &pixels,
    UINT size)
{
    const UINT stride = size * 4u;
    const UINT xorBytes = stride * size;
    const UINT maskStride = ((size + 31u) / 32u) * 4u;
    const UINT maskBytes = maskStride * size;

    std::vector<std::uint8_t> dib;
    dib.reserve(40u + xorBytes + maskBytes);

    appendU32(dib, 40u);              // BITMAPINFOHEADER size
    appendU32(dib, size);             // width
    appendU32(dib, size * 2u);        // XOR + AND mask height
    appendU16(dib, 1u);               // planes
    appendU16(dib, 32u);              // bits per pixel
    appendU32(dib, BI_RGB);
    appendU32(dib, xorBytes);
    appendU32(dib, 0u);
    appendU32(dib, 0u);
    appendU32(dib, 0u);
    appendU32(dib, 0u);

    // WIC gives us top-down BGRA. ICO DIB rows are bottom-up BGRA.
    for (int y = static_cast<int>(size) - 1; y >= 0; --y) {
        const auto rowStart = static_cast<std::size_t>(y) * stride;
        dib.insert(
            dib.end(),
            pixels.begin() + static_cast<std::ptrdiff_t>(rowStart),
            pixels.begin() + static_cast<std::ptrdiff_t>(rowStart + stride));
    }

    // Legacy 1-bpp AND mask derived from alpha.
    std::vector<std::uint8_t> mask(maskBytes, 0u);
    for (UINT outputRow = 0; outputRow < size; ++outputRow) {
        const UINT sourceY = size - 1u - outputRow;
        for (UINT x = 0; x < size; ++x) {
            const auto pixelOffset =
                static_cast<std::size_t>(sourceY) * stride
                + static_cast<std::size_t>(x) * 4u;
            const std::uint8_t alpha = pixels[pixelOffset + 3u];
            if (alpha < 128u) {
                const auto byteOffset =
                    static_cast<std::size_t>(outputRow) * maskStride
                    + (x / 8u);
                mask[byteOffset] |= static_cast<std::uint8_t>(0x80u >> (x % 8u));
            }
        }
    }
    dib.insert(dib.end(), mask.begin(), mask.end());

    return dib;
}

bool writeIcon(
    const std::filesystem::path &outputPath,
    const std::array<std::vector<std::uint8_t>, kIconSizes.size()> &frames)
{
    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output) {
        std::wcerr << L"[XX] Could not create output icon: " << outputPath.wstring() << L"\n";
        return false;
    }

    std::vector<std::uint8_t> directory;
    directory.reserve(6u + (16u * kIconSizes.size()));
    appendU16(directory, 0u);
    appendU16(directory, 1u);
    appendU16(directory, static_cast<std::uint16_t>(kIconSizes.size()));

    std::uint32_t dataOffset = static_cast<std::uint32_t>(6u + (16u * kIconSizes.size()));
    for (std::size_t i = 0; i < kIconSizes.size(); ++i) {
        const UINT size = kIconSizes[i];
        const std::uint8_t dimension = size >= 256u ? 0u : static_cast<std::uint8_t>(size);

        directory.push_back(dimension);
        directory.push_back(dimension);
        directory.push_back(0u);
        directory.push_back(0u);
        appendU16(directory, 1u);
        appendU16(directory, 32u);
        appendU32(directory, static_cast<std::uint32_t>(frames[i].size()));
        appendU32(directory, dataOffset);
        dataOffset += static_cast<std::uint32_t>(frames[i].size());
    }

    output.write(
        reinterpret_cast<const char *>(directory.data()),
        static_cast<std::streamsize>(directory.size()));

    for (const auto &frame : frames) {
        output.write(
            reinterpret_cast<const char *>(frame.data()),
            static_cast<std::streamsize>(frame.size()));
    }

    return output.good();
}

} // namespace

int wmain(int argc, wchar_t *argv[])
{
    if (argc != 3) {
        std::wcerr << L"usage: ywd_icon_builder <source-png> <output-ico>\n";
        return 2;
    }

    const std::filesystem::path inputPath(argv[1]);
    const std::filesystem::path outputPath(argv[2]);

    const HRESULT initHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(initHr)) {
        std::wcerr << L"[XX] COM initialization failed (HRESULT 0x"
                   << std::hex << static_cast<unsigned long>(initHr) << std::dec << L")\n";
        return 3;
    }

    int exitCode = 0;

    {
        ComPtr<IWICImagingFactory> factory;
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&factory));
        if (failed(hr, L"Create WIC imaging factory")) {
            exitCode = 4;
        } else {
            ComPtr<IWICBitmapDecoder> decoder;
            hr = factory->CreateDecoderFromFilename(
                inputPath.c_str(),
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnLoad,
                &decoder);

            if (failed(hr, L"Open branding PNG")) {
                exitCode = 5;
            } else {
                ComPtr<IWICBitmapFrameDecode> sourceFrame;
                hr = decoder->GetFrame(0u, &sourceFrame);
                if (failed(hr, L"Read branding PNG frame")) {
                    exitCode = 6;
                } else {
                    UINT sourceWidth = 0u;
                    UINT sourceHeight = 0u;
                    hr = sourceFrame->GetSize(&sourceWidth, &sourceHeight);
                    if (failed(hr, L"Read branding PNG dimensions") || sourceWidth == 0u || sourceHeight == 0u) {
                        exitCode = 7;
                    } else {
                        std::array<std::vector<std::uint8_t>, kIconSizes.size()> frames;
                        bool renderOk = true;

                        for (std::size_t i = 0; i < kIconSizes.size(); ++i) {
                            std::vector<std::uint8_t> pixels;
                            if (!renderSquareIconFrame(
                                    factory.Get(),
                                    sourceFrame.Get(),
                                    sourceWidth,
                                    sourceHeight,
                                    kIconSizes[i],
                                    pixels)) {
                                renderOk = false;
                                exitCode = 8;
                                break;
                            }
                            frames[i] = buildClassicDib(pixels, kIconSizes[i]);
                        }

                        if (renderOk) {
                            if (!writeIcon(outputPath, frames)) {
                                exitCode = 9;
                            } else {
                                std::wcout << L"[OK] Generated classic Win32 ICO from PNG "
                                           << inputPath.wstring() << L" -> "
                                           << outputPath.wstring() << L"\n";
                            }
                        }
                    }
                }
            }
        }
    }

    CoUninitialize();
    return exitCode;
}
