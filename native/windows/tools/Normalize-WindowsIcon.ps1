[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path $InputPath)) {
    throw "Source icon not found: $InputPath"
}

$source = @'
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;

public static class YwdClassicIconWriter
{
    private static readonly int[] Sizes = new int[] { 16, 20, 24, 32, 40, 48, 64, 96, 128, 256 };
    private static readonly byte[] PngSignature = new byte[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

    private sealed class IconEntry
    {
        public int Width;
        public int Height;
        public uint Length;
        public uint Offset;
    }

    public static void Convert(string inputPath, string outputPath)
    {
        byte[] sourceBytes = File.ReadAllBytes(inputPath);
        List<IconEntry> entries = ReadDirectory(sourceBytes);
        List<byte[]> images = new List<byte[]>();

        foreach (int size in Sizes)
        {
            using (Bitmap source = LoadFrame(sourceBytes, entries, inputPath, size))
            using (Bitmap bitmap = new Bitmap(size, size, PixelFormat.Format32bppArgb))
            {
                using (Graphics graphics = Graphics.FromImage(bitmap))
                {
                    graphics.Clear(Color.Transparent);
                    graphics.CompositingQuality = CompositingQuality.HighQuality;
                    graphics.InterpolationMode = InterpolationMode.HighQualityBicubic;
                    graphics.PixelOffsetMode = PixelOffsetMode.HighQuality;
                    graphics.SmoothingMode = SmoothingMode.HighQuality;
                    graphics.DrawImage(source, new Rectangle(0, 0, size, size));
                }

                images.Add(BuildClassicDib(bitmap));
            }
        }

        string directory = Path.GetDirectoryName(outputPath);
        if (!String.IsNullOrEmpty(directory))
        {
            Directory.CreateDirectory(directory);
        }

        using (BinaryWriter writer = new BinaryWriter(File.Create(outputPath)))
        {
            writer.Write((ushort)0);
            writer.Write((ushort)1);
            writer.Write((ushort)Sizes.Length);

            int dataOffset = 6 + (16 * Sizes.Length);
            for (int i = 0; i < Sizes.Length; ++i)
            {
                int size = Sizes[i];
                byte dimension = size >= 256 ? (byte)0 : (byte)size;

                writer.Write(dimension);
                writer.Write(dimension);
                writer.Write((byte)0);
                writer.Write((byte)0);
                writer.Write((ushort)1);
                writer.Write((ushort)32);
                writer.Write((uint)images[i].Length);
                writer.Write((uint)dataOffset);
                dataOffset += images[i].Length;
            }

            foreach (byte[] image in images)
            {
                writer.Write(image);
            }
        }
    }

    private static List<IconEntry> ReadDirectory(byte[] data)
    {
        using (MemoryStream stream = new MemoryStream(data, false))
        using (BinaryReader reader = new BinaryReader(stream))
        {
            ushort reserved = reader.ReadUInt16();
            ushort type = reader.ReadUInt16();
            ushort count = reader.ReadUInt16();
            if (reserved != 0 || type != 1 || count == 0)
            {
                throw new InvalidDataException("Input file is not a valid Windows ICO container.");
            }

            List<IconEntry> entries = new List<IconEntry>();
            for (int i = 0; i < count; ++i)
            {
                byte widthRaw = reader.ReadByte();
                byte heightRaw = reader.ReadByte();
                reader.ReadByte();
                reader.ReadByte();
                reader.ReadUInt16();
                reader.ReadUInt16();
                uint length = reader.ReadUInt32();
                uint offset = reader.ReadUInt32();

                IconEntry entry = new IconEntry();
                entry.Width = widthRaw == 0 ? 256 : widthRaw;
                entry.Height = heightRaw == 0 ? 256 : heightRaw;
                entry.Length = length;
                entry.Offset = offset;
                entries.Add(entry);
            }
            return entries;
        }
    }

    private static Bitmap LoadFrame(byte[] data, List<IconEntry> entries, string inputPath, int requestedSize)
    {
        IconEntry selected = null;
        int bestDistance = Int32.MaxValue;
        foreach (IconEntry entry in entries)
        {
            int distance = Math.Abs(entry.Width - requestedSize) + Math.Abs(entry.Height - requestedSize);
            if (distance < bestDistance)
            {
                selected = entry;
                bestDistance = distance;
            }
            if (entry.Width == requestedSize && entry.Height == requestedSize)
            {
                selected = entry;
                break;
            }
        }

        if (selected == null)
        {
            throw new InvalidDataException("ICO contains no image frames.");
        }

        long end = (long)selected.Offset + (long)selected.Length;
        if ((long)selected.Offset >= data.LongLength || end > data.LongLength)
        {
            throw new InvalidDataException("ICO frame points outside the source file.");
        }

        int frameOffset = checked((int)selected.Offset);
        int frameLength = checked((int)selected.Length);
        bool isPng = frameLength >= 8;
        for (int i = 0; i < 8 && isPng; ++i)
        {
            if (data[frameOffset + i] != PngSignature[i])
            {
                isPng = false;
            }
        }

        if (isPng)
        {
            byte[] payload = new byte[frameLength];
            Buffer.BlockCopy(data, frameOffset, payload, 0, frameLength);
            using (MemoryStream frameStream = new MemoryStream(payload, false))
            using (Image image = Image.FromStream(frameStream))
            {
                return new Bitmap(image);
            }
        }

        // Fallback for an already-classic ICO or a future mixed-frame replacement.
        using (Icon icon = new Icon(inputPath, requestedSize, requestedSize))
        using (Bitmap bitmap = icon.ToBitmap())
        {
            return new Bitmap(bitmap);
        }
    }

    private static byte[] BuildClassicDib(Bitmap bitmap)
    {
        int width = bitmap.Width;
        int height = bitmap.Height;
        int xorBytes = width * height * 4;
        int maskStride = ((width + 31) / 32) * 4;
        int maskBytes = maskStride * height;

        using (MemoryStream stream = new MemoryStream(40 + xorBytes + maskBytes))
        using (BinaryWriter writer = new BinaryWriter(stream))
        {
            // BITMAPINFOHEADER. ICO DIB height is XOR bitmap + AND mask.
            writer.Write((uint)40);
            writer.Write(width);
            writer.Write(height * 2);
            writer.Write((ushort)1);
            writer.Write((ushort)32);
            writer.Write((uint)0); // BI_RGB
            writer.Write((uint)xorBytes);
            writer.Write(0);
            writer.Write(0);
            writer.Write((uint)0);
            writer.Write((uint)0);

            // Classic ICO DIB pixels are bottom-up BGRA.
            for (int y = height - 1; y >= 0; --y)
            {
                for (int x = 0; x < width; ++x)
                {
                    Color pixel = bitmap.GetPixel(x, y);
                    writer.Write(pixel.B);
                    writer.Write(pixel.G);
                    writer.Write(pixel.R);
                    writer.Write(pixel.A);
                }
            }

            // Alpha already carries transparency; use an all-zero AND mask.
            writer.Write(new byte[maskBytes]);
            writer.Flush();
            return stream.ToArray();
        }
    }
}
'@

Add-Type -AssemblyName System.Drawing
if (-not ('YwdClassicIconWriter' -as [type])) {
    Add-Type -TypeDefinition $source -ReferencedAssemblies 'System.Drawing.dll'
}

[YwdClassicIconWriter]::Convert(
    (Resolve-Path $InputPath).Path,
    [System.IO.Path]::GetFullPath($OutputPath)
)

if (-not (Test-Path $OutputPath)) {
    throw "Icon conversion did not create output: $OutputPath"
}

$length = (Get-Item $OutputPath).Length
if ($length -lt 1024) {
    throw "Generated icon is suspiciously small ($length bytes): $OutputPath"
}

Write-Host "[OK] RC-compatible classic-DIB icon generated: $OutputPath ($length bytes)" -ForegroundColor Green
