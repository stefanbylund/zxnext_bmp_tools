# Next BMP Tools

The **Next BMP tools** are BMP image conversion tools targetting Sinclair ZX
Spectrum Next. These tools are used for converting the palette of 8-bit BMP
files to Spectrum Next format and converting such 8-bit BMP files to raw image
files suitable for layer 2 and sprite graphics on Spectrum Next. The tools are
described in more detail below.

The Next BMP tools are inspired by Jim Bagley's BMPtoNext and NextGrab tools but
with added support for the Spectrum Next RGB333 palette and with more validations
and options.

## Download

The latest version of the Next BMP tools can be downloaded
**[HERE](https://github.com/stefanbylund/zxnext_bmp_tools/blob/master/build/zxnext_bmp_tools.zip)**.
This download contains prebuilt executables for Windows along with the source
code and this README.md file. If you want to build the Next BMP tools yourself,
see the "How to Build" section below.

## How to Use

Below are descriptions of the nextbmp and nextraw tools and information on how
to use them.

### nextbmp

Tool for converting the palette in an uncompressed 8-bit BMP file to Spectrum
Next format. The original RGB888 colors in the palette are converted to RGB333
colors and then back to their equivalent RGB888 colors. The resulting RGB888
colors are equivalent representations of the Spectrum Next RGB333 colors and
the converted BMP file should display exactly as when rendered natively on the
Spectrum Next (disregarding color calibration of the monitors being used).
If no destination BMP file is specified, the source BMP file is modified.
Run the nextbmp tool without any parameters to get a list of all options.

Tip: If you have an image in another format than BMP (e.g. PNG) or another color
depth than 8 bits/pixel (e.g. 24-bit BMP), first convert the image to an 8-bit
BMP image in a paint program (e.g. Adobe Photoshop, Paint.NET, GIMP or PaintShop
Pro) before converting it to Spectrum Next palette format using the nextbmp tool.

Tip: The [palettes](palettes) directory contains Spectrum Next palette files in
several common palette formats for use with many paint programs. This directory
also contains a palette in [256](palettes/zxnext-artistic-palette-256.png) and
[512](palettes/zxnext-artistic-palette-512.png) colors designed for graphics
artists.

Tip: [Color Quantizer](http://x128.ho.ua/color-quantizer.html) is an excellent
image processing tool for downscaling the number of colors in an image. Use it
together with the 512 color Spectrum Next palette file
[zxnext-palette-512.pal](palettes/zxnext-palette-512.pal) for best results.

Note that when the original RGB888 colors in the palette are converted to RGB333
colors, the resulting 3-bit color components may not end up as integers and must
be rounded to an integer (0 - 7). By default, the 3-bit color components are
rounded to the nearest integer (-round). However, depending on the original
RGB888 colors, this may not always be the best choice. Sometimes, better results
are achieved by rounding upwards (-ceil) or downwards (-floor).

If the -min-palette option is specified, the converted palette is minimized
by removing any duplicated colors, sorting it in ascending order (i.e. the
same order as in the Spectrum Next standard palette), and clearing any unused
palette entries at the end. This is useful if you convert images that you
have not created the palette for and which may end up having duplicated
palette colors when being converted. This option is ignored if the
-std-palette option is given.

If the -std-palette option is specified, the original RGB888 colors in the
palette are converted to the Spectrum Next standard palette RGB332 colors
(which are extended to RGB333 colors when displayed). This is useful if you need
to use the standard palette. However, better results are generally achieved when
converting to the closest matching RGB333 colors. This option is ignored if the
-4bit option is given.

If the -4bit option is specified, the first 16 unique colors in the palette
will be moved to the start of the palette and all other palette entries
will be cleared. Any pixels in the image referencing any of the discarded
colors will also be cleared. The result will be a 4-bit image saved as an
8-bit BMP file. Use this option for images intended to be displayed in the
640 x 256 layer 2 mode or for 4-bit sprite sheets.

Note: The BMP file format supports both 4 and 8 bits per pixel. However, most
modern paint programs only support 8 bits per pixel as the lowest color depth
when saving to a BMP file. Therefore, nextbmp assumes that a 16 color 4-bit
image is contained in an 8-bit BMP file and not a 4-bit BMP file. If you would
have an actual 4-bit BMP file, you must first save it as an 8-bit BMP file in
your paint program before using it with the nextbmp tool and its -4bit option.

**Examples:**
```
> nextbmp image.bmp
> nextbmp -ceil -min-palette image.bmp image2.bmp
> nextbmp -floor -std-palette image.bmp image2.bmp
> nextbmp -4bit image.bmp image2.bmp
```

The first example will convert the palette in the BMP file image.bmp.

The second example will convert the palette in the BMP file image.bmp and write
it to the new BMP file image2.bmp and round up the color values to the nearest
integer when converting the palette. The converted palette will be minimized by
removing any duplicated colors.

The third example will convert the palette in the BMP file image.bmp to the
Spectrum Next standard palette and write it to the new BMP file image2.bmp and
round down the color values to the nearest integer when converting the pixel
colors.

The fourth example will convert the palette in the BMP file image.bmp to 16
colors and write it to the new BMP file image2.bmp. This is suitable for
640 x 256 layer 2 graphics and 4-bit sprite sheets.

The table below shows how the values for the RGB332 2-bit blue color component
and RGB333/RGB332 3-bit red/green/blue color components are mapped to RGB888
8-bit red/green/blue color components:

| 2-bit | 3-bit | 8-bit |
|------:|------:|------:|
|   0   |   0   |    0  |
|       |   1   |   36  |
|       |   2   |   73  |
|   1   |   3   |  109  |
|       |   4   |  146  |
|   2   |   5   |  182  |
|       |   6   |  219  |
|   3   |   7   |  255  |

### nextraw

Tool for converting an uncompressed 8-bit BMP file to a raw image file for
Spectrum Next. The RGB888 colors in the BMP palette are converted to RGB333
colors. If no destination file for the raw image is specified, the same name as
the source BMP file is used but with the extension ".nxi". If the -sep-palette
option is specified, the raw palette is written to a separate file with the
same name as the raw image file but with the extension ".nxp", otherwise it is
prepended to the raw image file. If the -no-palette option is specified, no raw
palette is created, e.g. if the BMP file uses the Spectrum Next standard palette
there is no need to create the raw palette. Run the nextraw tool without any
parameters to get a list of all options.

Note: If the BMP file does not already contain an RGB888 palette representing
the Spectrum Next RGB333 colors, first convert the BMP file to Spectrum Next
format using the nextbmp tool.

If the -4bit option is specified, each pixel is 4 bits so that each byte in
the raw image data will contain two horizontally adjacent pixels (where the
4 highest bits contain the leftmost pixel and the 4 lowest bits contain the
rightmost pixel) and the palette will contain 16 colors. By default, each
pixel is 8 bits so that each byte in the raw image data will contain one
pixel and the palette will contain 256 colors. Use this option for images
intended to be displayed in the 640 x 256 layer 2 mode or for 4-bit sprite
sheets.

If the -columns option is specified, the raw image data will consist of
pixels in linear order from top-to-bottom and left-to-right, i.e. columns of
pixels from left to right. By default, the raw image data will consist of
pixels in linear order from left-to-right and top-to-bottom, i.e. rows of
pixels from top to bottom. Use this option for images intended to be
displayed in the 320 x 256 or 640 x 256 layer 2 mode.

By default, the raw image file contains the raw palette followed by the raw
image data. If the -sep-palette option is specified, the raw palette is
instead written to a separate file. By default, the raw palette consists of
256 RGB333 colors and is 512 bytes long. If the -4bit option is specified,
the raw palette consists of 16 RGB333 colors and is 32 bytes long. The RGB333
colors are stored as an RGB332 byte followed by a zero-extended byte
containing the lowest blue bit. By default, the raw image data consists of
linear rows of pixels from top to bottom. If the -columns option is specified,
the raw image data consists of linear columns of pixels from left to right.
The 4/8-bit pixels are indexes into the 16/256 color palette.

**Examples:**
```
> nextraw image.bmp
> nextraw -sep-palette image.bmp graphics.nxi
> nextraw -no-palette image.bmp graphics.nxi
> nextraw -4bit sprites.bmp
> nextraw -columns wide.bmp
> nextraw -columns -4bit hires.bmp
```

The first example will convert the BMP file image.bmp to a raw image file
containing 256 x 192 layer 2 graphics or an 8-bit sprite sheet. By default the
raw image file will get the name image.nxi and have the raw palette prepended
to it.

The second example will convert the BMP file image.bmp to the raw image file
graphics.nxi and write the raw palette to a separate file called graphics.nxp.

The third example will convert the BMP file image.bmp to the raw image file
graphics.nxi and don't create any raw palette.

The fourth example will convert the BMP file sprites.bmp to a raw image file
containing a 4-bit sprite sheet.

The fifth example will convert the BMP file wide.bmp to a raw image file
containing 320 x 256 layer 2 graphics.

The sixth example will convert the BMP file hires.bmp to a raw image file
containing 640 x 256 layer 2 graphics.

## How to Build

The Next BMP tools are written in portable C code and should compile and run on
any modern operating system with a C compiler supporting C99 (or actually a
quite small subset thereof). The tools have been successfully compiled with GCC
and Microsoft Visual Studio and has been verified on Windows and Linux. They
should also work on macOS but that has not yet been tested.

Use the following command-lines to build using GCC (add -std=c99 if you're using
an older version of GCC):
```
> gcc -O2 -Wall -lm -o nextbmp nextbmp.c
> gcc -O2 -Wall -lm -o nextraw nextraw.c
```

Use the following command-lines to build using Microsoft Visual Studio:
```
> cl /O2 /W2 nextbmp.c
> cl /O2 /W2 nextraw.c
```

Note that you don't need to use Microsoft Visual Studio when compiling on
Windows, you can also use [MinGW GCC](http://www.mingw.org/).

## License

This software is licensed under the terms of the MIT license.
