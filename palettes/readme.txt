Palette Files
=============

This directory contains Spectrum Next palette files in four common palette
formats. The palette files are available in 256 (8-bit) colors and, if supported
by the palette format, 512 (9-bit) colors.

The order of the colors in the palette files are in ascending RGB order. This
is fine for image conversion needs. However, if you are a graphics artist you
may want another layout of the palette, see the artistic palette below.

Palette files for the following palette formats are provided:

* .act (Adobe Photoshop)
* .gpl (GIMP)
* .pal (JASC PaintShop Pro)
* .txt (Paint.NET)

If you are using another paint program than one from the list above, it is
possible that it will still support one or more of the palette formats above.
If not, you can use the provided PNG image palettes as a color picker in your
paint program.

Note 1: The .act palette format only supports 256 colors.

Note 2: Paint.NET will only read the first 96 colors of a .txt palette file.
So the palette files must be split up to your discretion.


Artistic Palette
================

The file zxnext-artistic-palette.png is an image of a palette designed for
graphics artists where the colors are grouped and ordered in a logical way that
is useful when drawing pixel art. Open this palette image in your favorite paint
program and use it as a color picker.

The palette contains several visualizations of the 512 color Spectrum Next
palette, where each one is explained below. The palette also contains a basic
section with commonly used colors.

This palette is partly inspired by a palette for the PC Engine by sunteam_paul.

Basic Palette
-------------

The basic palette contains the following:

* Shades and tints of the primary colors (red, green, blue) and secondary colors
  (cyan, magenta, yellow).
* Shades of the tertiary colors (rose, orange, chartreuse green, spring green,
  azure and violet). Note that there are only three shades of the tertiary
  colors in the Spectrum Next palette. The other four shades in this palette
  are approximations.
* Shades of gray and earth colors (brown etc).

Color Cube
----------

The RGB color cube consists of 8 x 8 x 8 = 512 cubelets, where each cubelet
represents a color in the 512 color Spectrum Next palette. The colors with the
lowest and highest intensity in the color space are located on the surface of
the cube. The other colors of intermediate intensity are represented by the
cubelets inside the cube.

This means that the color cube displays 296 of the 512 colors in the palette.
To see the other 216 colors, the cube needs to be sliced up in planes.

Color Planes
------------

The RGB color planes display all 512 colors in the Spectrum Next palette in
three different ways.

There are three groups of planes, one for each primary color (red, green, blue).
Each group contains 8 planes of size 8 by 8, i.e. 512 colors in total. The group
for one primary color (e.g. red) has that color as an increasing constant per
plane and varies the other two colors (e.g. blue and green) for each plane.

The RGB color planes are actually the RGB color cube sliced up in all three
directions.

Color Spectrum
--------------

The RGB color spectrum displays the lowest and highest intensity colors in
the Spectrum Next palette as a spectrum of colors. This means that the color
spectrum displays 296 of the 512 colors in the palette.

The RGB color spectrum displays the same colors as the surface colors of the
RGB color cube.

Color Wheel
-----------

The RGB color wheel shows the relationships between the primary (red, green,
blue), secondary (cyan, magenta, yellow) and tertiary (rose, orange, chartreuse
green, spring green, azure and violet) colors.

Since there are only three shades of the tertiary colors in the Spectrum Next
palette, there are two versions of the color wheel. One version displays only
three shades of each color, i.e. in total 36 colors and black. The other version
approximates four additional shades of the tertiary colors so that the full
seven shades of the primary and secondary colors can be displayed, i.e. in total
78 colors and black.

Color Gradients
---------------

The RGB color gradients display linear gradients between a primary color and
its complementary secondary color, i.e. red/cyan, green/magenta and blue/yellow.
Different variations of these gradients are also provided. In total, these color
gradients display 488 of the 512 colors in the Spectrum Next palette.
