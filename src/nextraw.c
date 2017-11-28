/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Program for converting an uncompressed 8-bit BMP file to a raw image file for
 * Sinclair ZX Spectrum Next. The RGB888 colors in the BMP palette are converted
 * to RGB333 colors. If no destination raw image file is specified, the same
 * name as the source BMP file is used but with the extension ".nxi". If the
 * -own-palette option is specified, the raw palette is written to a separate
 * file with the same name as the raw image file but with the extension ".nxp",
 * otherwise it is prepended to the raw image file. This program is suitable for
 * converting BMP files to raw layer 2 and sprite graphics.
 *
 * By default, the raw image file contains the raw palette followed by the raw
 * image data. The raw palette consists of 256 RGB333 colors and is 512 bytes
 * long. The RGB333 colors are stored as an RGB332 byte followed by a zero-
 * extended byte containing the lowest blue bit. The raw image data consists of
 * pixel bytes in linear order from left-to-right and top-to-bottom. The pixel
 * bytes are indexes into the 256 color palette. If the -own-palette option is
 * specified, the raw palette is instead written to a separate file with the
 * same layout as when the palette is prepended to the raw image file.
 *
 * Note: If the BMP file does not already contain an RGB888 palette representing
 * the Spectrum Next RGB333 colors, first convert the BMP file to Spectrum Next
 * format using the nextbmp companion tool.
 *
 * This program is inspired by Jim Bagley's NextGrab tool but with added support
 * for the Spectrum Next RGB333 palette and with more validations.
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FILE_HEADER_SIZE 14
#define MIN_DIB_HEADER_SIZE 40
#define HEADER_SIZE 54
#define PALETTE_SIZE 1024
#define RAW_PALETTE_SIZE 512
#define MIN_BMP_FILE_SIZE 1082

static uint8_t header[HEADER_SIZE];

static uint8_t palette[PALETTE_SIZE];

static uint8_t raw_palette[RAW_PALETTE_SIZE];

static uint8_t *image;

static void print_usage(void)
{
    printf("Usage: nextraw [-own-palette] <srcfile.bmp> [<dstfile>]\n");
    printf("Convert an uncompressed 8-bit BMP file to a raw image file for "
           "Sinclair ZX Spectrum Next.\n");
    printf("If no destination raw image file is specified, the same basename as the source BMP file is\n"
           "used but with the extension \".nxi\".\n");
    printf("\n");
    printf("Options:\n");
    printf("  -own-palette   If specified, the raw palette is written to a separate file with the same\n"
           "                 basename as the raw image file but with the extension \".nxp\".\n");
}

static void exit_with_msg(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  exit(EXIT_FAILURE);
}

static void create_filename(char *out_filename, const char *in_filename, const char *extension)
{
    // The buffer for the out_filename is allocated by the caller to be of sufficient size.

    strcpy(out_filename, in_filename);

    char *end = strrchr(out_filename, '.');
    end = (end == NULL) ? out_filename + strlen(out_filename) : end;

    strcpy(end, extension);
}

static bool is_valid_bmp_file(uint32_t *palette_offset,
                              uint32_t *image_offset,
                              uint32_t *image_width,
                              int32_t *image_height)
{
    if ((header[0] != 'B') || (header[1] != 'M'))
    {
        fprintf(stderr, "Not a BMP file.\n");
        return false;
    }

    uint32_t file_size = *((uint32_t *) (header + 2));
    if (file_size < MIN_BMP_FILE_SIZE)
    {
        fprintf(stderr, "Invalid size of BMP file.\n");
        return false;
    }

    *image_offset = *((uint32_t *) (header + 10));
    if (*image_offset >= file_size)
    {
        fprintf(stderr, "Invalid header of BMP file.\n");
        return false;
    }

    uint32_t dib_header_size = *((uint32_t *) (header + 14));
    if (dib_header_size < MIN_DIB_HEADER_SIZE)
    {
        // At least a BITMAPINFOHEADER is required.
        fprintf(stderr, "Invalid/unsupported header of BMP file.\n");
        return false;
    }

    *palette_offset = FILE_HEADER_SIZE + dib_header_size;

    *image_width = *((uint32_t *) (header + 18));
    if (*image_width == 0)
    {
        fprintf(stderr, "Invalid image width in BMP file.\n");
        return false;
    }

    *image_height = *((int32_t *) (header + 22));
    if (*image_height == 0)
    {
        fprintf(stderr, "Invalid image height in BMP file.\n");
        return false;
    }

    if (*image_width * abs(*image_height) >= file_size)
    {
        fprintf(stderr, "Invalid image size in BMP file.\n");
        return false;
    }

    uint16_t bpp = *((uint16_t *) (header + 28));
    if (bpp != 8)
    {
        fprintf(stderr, "Not an 8-bit BMP file.\n");
        return false;
    }

    uint32_t compression = *((uint32_t *) (header + 30));
    if (compression != 0)
    {
        fprintf(stderr, "Not an uncompressed BMP file.\n");
        return false;
    }

    return true;
}

static void free_image(void)
{
    if (image != NULL)
    {
        free(image);
        image = NULL;
    }
}

static uint8_t c8_to_c3(uint8_t c8)
{
    return (uint8_t) round((c8 * 7.0) / 255.0);
}

static void create_raw_palette(void)
{
    // Create the raw palette.
    // The RGB888 colors in the BMP palette are converted to RGB333 colors,
    // which are then split in RGB332 and B1 parts.
    for (int i = 0; i < 256; i++)
    {
        // BMP palette contains BGRA colors.
        uint8_t r8 = palette[i * 4 + 2];
        uint8_t g8 = palette[i * 4 + 1];
        uint8_t b8 = palette[i * 4 + 0];

        uint8_t r3 = c8_to_c3(r8);
        uint8_t g3 = c8_to_c3(g8);
        uint8_t b3 = c8_to_c3(b8);

        uint16_t rgb333 = (r3 << 6) | (g3 << 3) | (b3 << 0);
        uint8_t rgb332 = (uint8_t) (rgb333 >> 1);
        uint8_t b1 = (uint8_t) (rgb333 & 0x01);

        raw_palette[i * 2 + 0] = rgb332;
        raw_palette[i * 2 + 1] = b1;
    }
}

int main(int argc, char *argv[])
{
    char out_filename[256];
    char palette_filename[256];
    uint32_t palette_offset;
    uint32_t image_offset;
    uint32_t image_width;
    int32_t image_height;

    if ((argc < 2) || (argc > 4))
    {
        print_usage();
        return 0;
    }

    // Parse program arguments.
    bool separate_palette = !strcmp(argv[1], "-own-palette");
    if (separate_palette && (argc == 2))
    {
        print_usage();
        return 0;
    }
    char *in_filename = separate_palette ? argv[2] : argv[1];
    if ((argc == 4) || ((argc == 3) && !separate_palette))
    {
        // Given filename for raw image file.
        sprintf(out_filename, "%s", (argc == 4) ? argv[3] : argv[2]);
    }
    else
    {
        // Default filename for raw image file.
        create_filename(out_filename, in_filename, ".nxi");
    }
    if (!strcmp(in_filename, out_filename))
    {
        exit_with_msg("BMP file and raw image file cannot have the same name.\n");
    }
    if (separate_palette)
    {
        create_filename(palette_filename, out_filename, ".nxp");
    }

    // Open the BMP file and validate its header.
    FILE *in_file = fopen(in_filename, "rb");
    if (in_file == NULL)
    {
        exit_with_msg("Can't open file %s.\n", in_filename);
    }
    if (fread(header, sizeof(uint8_t), sizeof(header), in_file) != sizeof(header))
    {
        exit_with_msg("Can't read the BMP header in file %s.\n", in_filename);
    }
    if (!is_valid_bmp_file(&palette_offset, &image_offset, &image_width, &image_height))
    {
        exit_with_msg("The file %s is not a valid or supported BMP file.\n", in_filename);
    }

    // Allocate memory for image data.
    bool bottom_to_top_image = (image_height > 0);
    // Note: Image width is padded to a multiple of 4 bytes.
    uint32_t padded_image_width = (image_width + 3) & ~0x03;
    image_height = bottom_to_top_image ? image_height : -image_height;
    uint32_t image_size = padded_image_width * image_height;
    image = malloc(image_size);
    if (image == NULL)
    {
        exit_with_msg("Can't allocate memory for image data.\n");
    }
    atexit(free_image);

    // Read the palette and image data.
    if (fseek(in_file, palette_offset, SEEK_SET) != 0)
    {
        exit_with_msg("Can't access the BMP palette in file %s.\n", in_filename);
    }
    if (fread(palette, sizeof(uint8_t), sizeof(palette), in_file) != sizeof(palette))
    {
        exit_with_msg("Can't read the BMP palette in file %s.\n", in_filename);
    }
    if (fseek(in_file, image_offset, SEEK_SET) != 0)
    {
        exit_with_msg("Can't access the BMP image data in file %s.\n", in_filename);
    }
    if (fread(image, sizeof(uint8_t), image_size, in_file) != image_size)
    {
        exit_with_msg("Can't read the BMP image data in file %s.\n", in_filename);
    }
    fclose(in_file);

    // Create the raw palette.
    create_raw_palette();

    // Open the raw image file.
    FILE *out_file = fopen(out_filename, "wb");
    if (out_file == NULL)
    {
        exit_with_msg("Can't create raw image file %s.\n", out_filename);
    }

    // Write the raw palette either as a separate file or prepended to the raw image file.
    if (separate_palette)
    {
        FILE *palette_file = fopen(palette_filename, "wb");
        if (palette_file == NULL)
        {
            exit_with_msg("Can't create raw palette file %s.\n", palette_filename);
        }
        if (fwrite(raw_palette, sizeof(uint8_t), sizeof(raw_palette), palette_file) != sizeof(raw_palette))
        {
            exit_with_msg("Can't write the raw palette file %s.\n", palette_filename);
        }
        fclose(palette_file);
    }
    else
    {
        if (fwrite(raw_palette, sizeof(uint8_t), sizeof(raw_palette), out_file) != sizeof(raw_palette))
        {
            exit_with_msg("Can't write the raw palette to file %s.\n", out_filename);
        }
    }

    // Write the raw image data.
    uint8_t *image_ptr = image;
    if (bottom_to_top_image)
    {
        image_ptr += image_size - padded_image_width;
    }
    for (int y = 0; y < image_height; y++)
    {
        if (fwrite(image_ptr, sizeof(uint8_t), image_width, out_file) != image_width)
        {
            exit_with_msg("Error writing raw image file %s.\n", out_filename);
        }
        image_ptr = bottom_to_top_image ? image_ptr - padded_image_width : image_ptr + padded_image_width;
    }
    fclose(out_file);

    return 0;
}
