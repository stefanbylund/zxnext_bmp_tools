/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Program for converting an uncompressed 8-bit BMP file to a raw image file
 * for ZX Spectrum Next. The RGB888 colors in the BMP palette are converted to
 * RGB333 colors. If no destination raw image file is specified, the same
 * name as the source BMP file is used but with the extension ".nxi". If the
 * -sep-palette option is specified, the raw palette is written to a separate
 * file with the same name as the raw image file but with the extension ".nxp",
 * otherwise it is prepended to the raw image file. If the -no-palette option is
 * specified, no raw palette is created, e.g. if the BMP file uses the Spectrum
 * Next standard palette there is no need to create the raw palette.
 * This program is suitable for converting BMP files to raw layer 2 graphics
 * (256 x 192, 320 x 256 and 640 x 256) and sprite graphics (4-bit and 8-bit).
 *
 * If the -4bit option is specified, each pixel is 4 bits so that each byte in
 * the raw image data will contain two horizontally adjacent pixels (where the
 * 4 highest bits contain the leftmost pixel and the 4 lowest bits contain the
 * rightmost pixel) and the palette will contain 16 colors. By default, each
 * pixel is 8 bits so that each byte in the raw image data will contain one
 * pixel and the palette will contain 256 colors. Use this option for images
 * intended to be displayed in the 640 x 256 layer 2 mode or for 4-bit sprite
 * sheets.
 *
 * If the -columns option is specified, the raw image data will consist of
 * pixels in linear order from top-to-bottom and left-to-right, i.e. columns of
 * pixels from left to right. By default, the raw image data will consist of
 * pixels in linear order from left-to-right and top-to-bottom, i.e. rows of
 * pixels from top to bottom. Use this option for images intended to be
 * displayed in the 320 x 256 or 640 x 256 layer 2 mode.
 *
 * By default, the raw image file contains the raw palette followed by the raw
 * image data. If the -sep-palette option is specified, the raw palette is
 * instead written to a separate file. By default, the raw palette consists of
 * 256 RGB333 colors and is 512 bytes long. If the -4bit option is specified,
 * the raw palette consists of 16 RGB333 colors and is 32 bytes long. The RGB333
 * colors are stored as an RGB332 byte followed by a zero-extended byte
 * containing the lowest blue bit. By default, the raw image data consists of
 * linear rows of pixels from top to bottom. If the -columns option is specified,
 * the raw image data consists of linear columns of pixels from left to right.
 * The 4/8-bit pixels are indexes into the 16/256 color palette.
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
#define RAW_4BIT_PALETTE_SIZE 32
#define MIN_BMP_FILE_SIZE 1082

typedef enum palette_option
{
    EMBEDDED,
    SEPARATE,
    NONE
} palette_option_t;

typedef struct arguments
{
    palette_option_t palette_option;
    bool use_4bit;
    bool column_layout;
    char *in_filename;
    char *out_filename;
} arguments_t;

static uint8_t header[HEADER_SIZE];

static uint8_t palette[PALETTE_SIZE];

static uint8_t raw_palette[RAW_PALETTE_SIZE];

static uint8_t *image;

static uint8_t *raw_image;

static void exit_handler(void)
{
    if (image != NULL)
    {
        free(image);
        image = NULL;
    }

    if (raw_image != NULL)
    {
        free(raw_image);
        raw_image = NULL;
    }
}

static void print_usage(void)
{
    printf("Usage: nextraw [-embed-palette|-sep-palette|-no-palette] [-4bit] [-columns] <srcfile.bmp> [<dstfile>]\n");
    printf("Convert an uncompressed 8-bit BMP file to a raw image file for ZX Spectrum Next.\n");
    printf("If no destination raw image file is specified, the same basename as the source BMP file is\n"
           "used but with the extension \".nxi\".\n");
    printf("\n");
    printf("Options:\n");
    printf("  -embed-palette  The raw palette is prepended to the raw image file (default).\n");
    printf("  -sep-palette    The raw palette is written to a separate file with the same\n"
           "                  basename as the raw image file but with the extension \".nxp\".\n");
    printf("  -no-palette     No raw palette is created.\n");
    printf("  -4bit           If specified, use 4 bits per pixel (16 colors). Default is 8 bits per pixel (256 colors).\n");
    printf("  -columns        If specified, use column based memory layout. Default is row based memory layout.\n");
}

static bool parse_args(int argc, char *argv[], arguments_t *args)
{
    if (argc == 1)
    {
        print_usage();
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "-embed-palette"))
            {
                args->palette_option = EMBEDDED;
            }
            else if (!strcmp(argv[i], "-sep-palette"))
            {
                args->palette_option = SEPARATE;
            }
            else if (!strcmp(argv[i], "-no-palette"))
            {
                args->palette_option = NONE;
            }
            else if (!strcmp(argv[i], "-4bit"))
            {
                args->use_4bit = true;
            }
            else if (!strcmp(argv[i], "-columns"))
            {
                args->column_layout = true;
            }
            else if (!strcmp(argv[i], "-help"))
            {
                print_usage();
                return false;
            }
            else
            {
                fprintf(stderr, "Invalid option: %s\n", argv[i]);
                print_usage();
                return false;
            }
        }
        else
        {
            if (args->in_filename == NULL)
            {
                args->in_filename = argv[i];
            }
            else if (args->out_filename == NULL)
            {
                args->out_filename = argv[i];
            }
            else
            {
                fprintf(stderr, "Too many arguments.\n");
                print_usage();
                return false;
            }
        }
    }

    if (args->in_filename == NULL)
    {
        fprintf(stderr, "Input file not specified.\n");
        print_usage();
        return false;
    }

    return true;
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

static uint8_t c8_to_c3(uint8_t c8)
{
    return (uint8_t) round((c8 * 7.0) / 255.0);
}

static void create_raw_palette(uint32_t num_palette_colors)
{
    // Create the raw palette.
    // The RGB888 colors in the BMP palette are converted to RGB333 colors,
    // which are then split in RGB332 and B1 parts.
    for (int i = 0; i < num_palette_colors; i++)
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
    arguments_t args = {EMBEDDED, false, false, NULL, NULL};
    char out_filename[256];
    char palette_filename[256];
    uint32_t palette_offset;
    uint32_t image_offset;
    uint32_t image_width;
    int32_t image_height;

    atexit(exit_handler);

    // Parse program arguments.
    if (!parse_args(argc, argv, &args))
    {
        exit(EXIT_FAILURE);
    }

    // Create file names for raw image file and, if separate, raw palette file.
    if (args.out_filename != NULL)
    {
        // Given filename for raw image file.
        sprintf(out_filename, "%s", args.out_filename);
    }
    else
    {
        // Default filename for raw image file.
        create_filename(out_filename, args.in_filename, ".nxi");
    }
    if (!strcmp(args.in_filename, out_filename))
    {
        exit_with_msg("BMP file and raw image file cannot have the same name.\n");
    }
    if (args.palette_option == SEPARATE)
    {
        create_filename(palette_filename, out_filename, ".nxp");
    }

    // Open the BMP file and validate its header.
    FILE *in_file = fopen(args.in_filename, "rb");
    if (in_file == NULL)
    {
        exit_with_msg("Can't open file %s.\n", args.in_filename);
    }
    if (fread(header, sizeof(uint8_t), sizeof(header), in_file) != sizeof(header))
    {
        exit_with_msg("Can't read the BMP header in file %s.\n", args.in_filename);
    }
    if (!is_valid_bmp_file(&palette_offset, &image_offset, &image_width, &image_height))
    {
        exit_with_msg("The file %s is not a valid or supported BMP file.\n", args.in_filename);
    }

    // Allocate memory for image data.
    // Note: Image width is padded to a multiple of 4 bytes.
    bool bottom_to_top_image = (image_height > 0);
    uint32_t padded_image_width = (image_width + 3) & ~0x03;
    image_height = bottom_to_top_image ? image_height : -image_height;
    uint32_t image_size = padded_image_width * image_height;
    image = malloc(image_size);
    if (image == NULL)
    {
        exit_with_msg("Can't allocate memory for image data.\n");
    }

    // Read the palette and image data.
    if (args.palette_option != NONE)
    {
        if (fseek(in_file, palette_offset, SEEK_SET) != 0)
        {
            exit_with_msg("Can't access the BMP palette in file %s.\n", args.in_filename);
        }
        if (fread(palette, sizeof(uint8_t), sizeof(palette), in_file) != sizeof(palette))
        {
            exit_with_msg("Can't read the BMP palette in file %s.\n", args.in_filename);
        }
    }
    if (fseek(in_file, image_offset, SEEK_SET) != 0)
    {
        exit_with_msg("Can't access the BMP image data in file %s.\n", args.in_filename);
    }
    if (fread(image, sizeof(uint8_t), image_size, in_file) != image_size)
    {
        exit_with_msg("Can't read the BMP image data in file %s.\n", args.in_filename);
    }
    fclose(in_file);

    // Create the raw palette.
    if (args.palette_option != NONE)
    {
        uint32_t num_palette_colors = args.use_4bit ? 16 : 256;
        create_raw_palette(num_palette_colors);
    }

    // Open the raw image file.
    FILE *out_file = fopen(out_filename, "wb");
    if (out_file == NULL)
    {
        exit_with_msg("Can't create raw image file %s.\n", out_filename);
    }

    // Write the raw palette either prepended to the raw image file or as a separate file.
    uint32_t raw_palette_size = args.use_4bit ? RAW_4BIT_PALETTE_SIZE : RAW_PALETTE_SIZE;
    if (args.palette_option == EMBEDDED)
    {
        if (fwrite(raw_palette, sizeof(uint8_t), raw_palette_size, out_file) != raw_palette_size)
        {
            exit_with_msg("Can't write the raw palette to file %s.\n", out_filename);
        }
    }
    else if (args.palette_option == SEPARATE)
    {
        FILE *palette_file = fopen(palette_filename, "wb");
        if (palette_file == NULL)
        {
            exit_with_msg("Can't create raw palette file %s.\n", palette_filename);
        }
        if (fwrite(raw_palette, sizeof(uint8_t), raw_palette_size, palette_file) != raw_palette_size)
        {
            exit_with_msg("Can't write the raw palette file %s.\n", palette_filename);
        }
        fclose(palette_file);
    }

    uint8_t *image_ptr = image;
    if (bottom_to_top_image)
    {
        image_ptr += image_size - padded_image_width;
    }

    // Allocate memory for raw image data.
    uint32_t raw_image_width = args.use_4bit ? ((image_width + image_width % 2) / 2) : image_width;
    uint32_t raw_image_size = raw_image_width * image_height;
    raw_image = malloc(raw_image_size);
    if (raw_image == NULL)
    {
        exit_with_msg("Can't allocate memory for raw image data.\n");
    }

    // Convert the image data to raw image data.
    if (args.column_layout)
    {
        if (args.use_4bit)
        {
            // 640 x 256 layer 2 mode
            for (int y = 0; y < image_height; y++)
            {
                for (int x = 0; x < image_width; x += 2)
                {
                    uint8_t left_pixel = (image_ptr[x] & 0x0F) << 4;
                    uint8_t right_pixel = image_ptr[x + 1] & 0x0F;
                    raw_image[y + (x / 2) * image_height] = left_pixel | right_pixel;
                }
                image_ptr = bottom_to_top_image ? image_ptr - padded_image_width : image_ptr + padded_image_width;
            }
        }
        else
        {
            // 320 x 256 layer 2 mode
            for (int y = 0; y < image_height; y++)
            {
                for (int x = 0; x < image_width; x++)
                {
                    raw_image[y + x * image_height] = image_ptr[x];
                }
                image_ptr = bottom_to_top_image ? image_ptr - padded_image_width : image_ptr + padded_image_width;
            }
        }
    }
    else
    {
        if (args.use_4bit)
        {
            // 4-bit sprite sheets
            for (int y = 0; y < image_height; y++)
            {
                for (int x = 0; x < image_width; x += 2)
                {
                    uint8_t left_pixel = (image_ptr[x] & 0x0F) << 4;
                    uint8_t right_pixel = image_ptr[x + 1] & 0x0F;
                    raw_image[y * raw_image_width + x / 2] = left_pixel | right_pixel;
                }
                image_ptr = bottom_to_top_image ? image_ptr - padded_image_width : image_ptr + padded_image_width;
            }
        }
        else
        {
            // 256 x 192 layer 2 mode and 8-bit sprite sheets
            uint8_t *raw_image_ptr = raw_image;
            for (int y = 0; y < image_height; y++)
            {
                memcpy(raw_image_ptr, image_ptr, image_width);
                image_ptr = bottom_to_top_image ? image_ptr - padded_image_width : image_ptr + padded_image_width;
                raw_image_ptr += image_width;
            }
        }
    }

    // Write the raw image data to file.
    if (fwrite(raw_image, sizeof(uint8_t), raw_image_size, out_file) != raw_image_size)
    {
        exit_with_msg("Error writing raw image file %s.\n", out_filename);
    }
    fclose(out_file);

    return 0;
}
