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

    public static void Convert(string inputPath, string outputPath)
    {
        List<byte[]> images = new List<byte[]>();

        foreach (int size in Sizes)
        {
            using (Icon icon = new Icon(inputPath, size, size))
            using (Bitmap source = icon.ToBitmap())
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
            writer.Write((ushort)0);                 // reserved
            writer.Write((ushort)1);                 // icon
            writer.Write((ushort)Sizes.Length);      // image count

            int dataOffset = 6 + (16 * Sizes.Length);
            for (int i = 0; i < Sizes.Length; ++i)
            {
                int size = Sizes[i];
                byte dimension = size >= 256 ? (byte)0 : (byte)size;

                writer.Write(dimension);             // width
                writer.Write(dimension);             // height
                writer.Write((byte)0);               // palette entries
                writer.Write((byte)0);               // reserved
                writer.Write((ushort)1);             // planes
                writer.Write((ushort)32);            // bits per pixel
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
            // BITMAPINFOHEADER. biHeight is doubled for ICO XOR + AND planes.
            writer.Write((uint)40);
            writer.Write(width);
            writer.Write(height * 2);
            writer.Write((ushort)1);
            writer.Write((ushort)32);
            writer.Write((uint)0);                   // BI_RGB
            writer.Write((uint)xorBytes);
            writer.Write(0);
            writer.Write(0);
            writer.Write((uint)0);
            writer.Write((uint)0);

            // Classic icon DIB pixels are bottom-up BGRA.
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

            // Fully transparent pixels are already represented by alpha, so a zero AND mask is correct.
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
