# Generates justplay.ico: a blue rounded square with a white play triangle
# and sound waves. Run from PowerShell 7 on Windows:  ./res/make-icon.ps1
Add-Type -AssemblyName System.Drawing

$sizes = 16, 24, 32, 48, 64, 128, 256
$out = Join-Path $PSScriptRoot 'justplay.ico'

function New-IconPng([int]$s) {
    $bmp = New-Object System.Drawing.Bitmap $s, $s, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = 'AntiAlias'
    $g.PixelOffsetMode = 'HighQuality'

    # Background: rounded square with a top-to-bottom blue gradient.
    $m = [Math]::Max(1, $s * 0.04)
    $w = $s - 2 * $m
    $r = $w * 0.22
    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    $path.AddArc($m, $m, 2 * $r, 2 * $r, 180, 90)
    $path.AddArc($m + $w - 2 * $r, $m, 2 * $r, 2 * $r, 270, 90)
    $path.AddArc($m + $w - 2 * $r, $m + $w - 2 * $r, 2 * $r, 2 * $r, 0, 90)
    $path.AddArc($m, $m + $w - 2 * $r, 2 * $r, 2 * $r, 90, 90)
    $path.CloseFigure()
    $rect = New-Object System.Drawing.RectangleF $m, $m, $w, $w
    $bg = New-Object System.Drawing.Drawing2D.LinearGradientBrush $rect,
        ([System.Drawing.Color]::FromArgb(255, 64, 150, 255)),
        ([System.Drawing.Color]::FromArgb(255, 22, 78, 190)), 90
    $g.FillPath($bg, $path)

    $white = [System.Drawing.Brushes]::White
    $small = $s -lt 32

    # Play triangle: centred on small sizes, shifted left to make room for waves.
    $th = $s * ($(if ($small) { 0.50 } else { 0.42 }))
    $tw = $th * 0.87
    $tx = $(if ($small) { ($s - $tw) / 2 + $s * 0.04 } else { $s * 0.22 })
    $ty = ($s - $th) / 2
    $tri = [System.Drawing.PointF[]]@(
        (New-Object System.Drawing.PointF $tx, $ty),
        (New-Object System.Drawing.PointF ($tx + $tw), ($ty + $th / 2)),
        (New-Object System.Drawing.PointF $tx, ($ty + $th)))
    $g.FillPolygon($white, $tri)

    # Two sound-wave arcs to the right of the triangle (32px and up).
    if (-not $small) {
        $pen = New-Object System.Drawing.Pen ([System.Drawing.Color]::White), ($s * 0.065)
        $pen.StartCap = 'Round'; $pen.EndCap = 'Round'
        $cx = $tx + $tw - $s * 0.02
        $cy = $s / 2
        foreach ($rad in ($s * 0.17), ($s * 0.29)) {
            $g.DrawArc($pen, $cx - $rad, $cy - $rad, 2 * $rad, 2 * $rad, -40, 80)
        }
        $pen.Dispose()
    }

    $g.Dispose()
    $ms = New-Object System.IO.MemoryStream
    $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    , $ms.ToArray()
}

# ICO container: header, one 16-byte directory entry per image, then PNG data.
$images = foreach ($s in $sizes) { , (New-IconPng $s) }
$ms = New-Object System.IO.MemoryStream
$bw = New-Object System.IO.BinaryWriter $ms
$bw.Write([UInt16]0); $bw.Write([UInt16]1); $bw.Write([UInt16]$sizes.Count)
$offset = 6 + 16 * $sizes.Count
for ($i = 0; $i -lt $sizes.Count; $i++) {
    $dim = $(if ($sizes[$i] -ge 256) { 0 } else { $sizes[$i] })
    $bw.Write([byte]$dim); $bw.Write([byte]$dim)
    $bw.Write([byte]0); $bw.Write([byte]0)
    $bw.Write([UInt16]1); $bw.Write([UInt16]32)
    $bw.Write([UInt32]$images[$i].Length); $bw.Write([UInt32]$offset)
    $offset += $images[$i].Length
}
foreach ($img in $images) { $bw.Write($img) }
$bw.Flush()
[System.IO.File]::WriteAllBytes($out, $ms.ToArray())
"Wrote $out ($($ms.Length) bytes, sizes: $($sizes -join ', '))"
