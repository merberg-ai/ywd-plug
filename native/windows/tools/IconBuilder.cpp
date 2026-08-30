#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
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

ComPtr<IWICBitmapFrameDecode> selectBestFrame(IWICBitmapDecoder *decoder, UINT requestedSize)
{
    UINT frameCount = 0;
    if (FAILED(decoder->GetFrameCount(&frameCount)) || frameCount == 0) {
        return {};
    }

    ComPtr<IWICBitmapFrameDecode> bestFrame;
    unsigned long bestScore = std::numeric_limits<unsigned long>::max();

    for (UINT i = 0; i < frameCount; ++i) {
        ComPtr<IWICBitmapFrameDecode> frame;
        if (FAILED(decoder->GetFrame(i, &frame))) {
            continue;
        }

        UINT width = 0;
        UINT height = 0;
        if (FAILED(frame->GetSize(&width, &height))) {
            continue;
        }

        const auto widthDistance = static_cast<unsigned long>(std::abs(
            static_cast<long long>(width) - static_cast<long long>(requestedSize)));
        const auto heightDistance = static_cast<unsigned long>(std::abs(
            static_cast<long long>(height) - static_cast<long long>(requestedSize)));
        const unsigned long score = widthDistance + heightDistance;

        if (score < bestScore) {
            bestScore = score;
            bestFrame = frame;
        }

        if (width == requestedSize && height == requestedSize) {
            break;
        }
    }

    return bestFrame;
}

bool decodeFrame(
    IWICImagingFactory *factory,
    IWICBitmapDecoder *decoder,
    UINT size,
    std::vector<std::uint8_t> &pixels)
{
    ComPtr<IWICBitmapFrameDecode> frame = selectBestFrame(decoder, size);
    if (!frame) {
        std::wcerr << L"[XX] No usable ICO frame found for " << size << L"x" << size << L"\n";
        return false;
    }

    UINT frameWidth = 0;
    UINT frameHeight = 0;
    if (failed(frame->GetSize(&frameWidth, &frameHeight), L"Read ICO frame size")) {
        return false;
    }

    ComPtr<IWICBitmapSource> bitmapSource;
    ComPtr<IWICBitmapScaler> scaler;

    if (frameWidth == size && frameHeight == size) {
        if (failed(frame.As(&bitmapSource), L"Use ICO frame as bitmap source")) {
            return false;
        }
    } else {
        if (failed(factory->CreateBitmapScaler(&scaler), L"Create WIC scaler")) {
            return false;
        }
        if (failed(
                scaler->Initialize(frame.Get(), size, size, WICBitmapInterpolationModeFant),
                L"Scale ICO frame")) {
            return false;
        }
        if (failed(scaler.As(&bitmapSource), L"Use scaled ICO frame as bitmap source")) {
            return false;
        }
    }

    ComPtr<IWICFormatConverter> converter;
    if (failed(factory->CreateFormatConverter(&converter), L"Create WIC format converter")) {
        return false;
    }

    if (failed(
            converter->Initialize(
                bitmapSource.Get(),
                GUID_WICPixelFormat32bppBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeCustom),
            L"Convert ICO frame to 32-bit BGRA")) {
        return false;
    }

    const UINT stride = size * 4u;
    pixels.resize(static_cast<std::size_t>(stride) * size);
    if (failed(
            converter->CopyPixels(
                nullptr,
                stride,
                static_cast<UINT>(pixels.size()),
                pixels.data()),
            L"Copy converted ICO pixels")) {
        return false;
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

    // BITMAPINFOHEADER. ICO DIB height includes the XOR bitmap and AND mask.
    appendU32(dib, 40u);
    appendU32(dib, size);
    appendU32(dib, size * 2u);
    appendU16(dib, 1u);
    appendU16(dib, 32u);
    appendU32(dib, BI_RGB);
    appendU32(dib, xorBytes);
    appendU32(dib, 0u);
    appendU32(dib, 0u);
    appendU32(dib, 0u);
    appendU32(dib, 0u);

    // WIC gives us top-down BGRA. Classic ICO DIB rows are bottom-up BGRA.
    for (int y = static_cast<int>(size) - 1; y >= 0; --y) {
        const auto rowStart = static_cast<std::size_t>(y) * stride;
        dib.insert(
            dib.end(),
            pixels.begin() + static_cast<std::ptrdiff_t>(rowStart),
            pixels.begin() + static_cast<std::ptrdiff_t>(rowStart + stride));
    }

    // Build a legacy 1-bpp AND mask from alpha for maximum shell compatibility.
    std::vector<std::uint8_t> mask(maskBytes, 0u);
    for (UINT outputRow = 0; outputRow < size; ++outputRow) {
        const UINT sourceY = size - 1u - outputRow;
        for (UINT x = 0; x < size; ++x) {
            const auto pixelOffset = static_cast<std::size_t>(sourceY) * stride + (x * 4u);
            const std::uint8_t alpha = pixels[pixelOffset + 3u];
            if (alpha < 128u) {
                const auto byteOffset = static_cast<std::size_t>(outputRow) * maskStride + (x / 8u);
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
        std::wcerr << L"usage: ywd_icon_builder <source-ico> <output-ico>\n";
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

    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory));
    if (failed(hr, L"Create WIC imaging factory")) {
        CoUninitialize();
        return 4;
    }

    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromFilename(
        inputPath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder);
    if (failed(hr, L"Open source ICO")) {
        CoUninitialize();
        return 5;
    }

    std::array<std::vector<std::uint8_t>, kIconSizes.size()> frames;
    for (std::size_t i = 0; i < kIconSizes.size(); ++i) {
        std::vector<std::uint8_t> pixels;
        if (!decodeFrame(factory.Get(), decoder.Get(), kIconSizes[i], pixels)) {
            CoUninitialize();
            return 6;
        }
        frames[i] = buildClassicDib(pixels, kIconSizes[i]);
    }

    if (!writeIcon(outputPath, frames)) {
        CoUninitialize();
        return 7;
    }

    std::wcout << L"[OK] Generated classic Win32 ICO from " << inputPath.wstring()
               << L" -> " << outputPath.wstring() << L"\n";

    CoUninitialize();
    return 0;
}
