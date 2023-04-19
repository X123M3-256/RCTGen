#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "image.h"

png_color palette[256] = {
{0,0,0},//0
{1,1,1},
{2,2,2},
{3,3,3},
{4,4,4},
{5,5,5},
{6,6,6},
{7,7,7},
{8,8,8},
{9,9,9},
{23,35,35},
{35,51,51},
{47,67,67},
{63,83,83},
{75,99,99},
{91,115,115},
{111,131,131},
{131,151,151},
{159,175,175},
{183,195,195},
{211,219,219},//20
{239,243,243},
{51,47,0},
{63,59,0},
{79,75,11},
{91,91,19},
{107,107,31},
{119,123,47},
{135,139,59},
{151,155,79},
{167,175,95},
{187,191,115},
{203,207,139},
{223,227,163},
{67,43,7},
{87,59,11},
{111,75,23},
{127,87,31},
{143,99,39},
{159,115,51},
{179,131,67},//40
{191,151,87},
{203,175,111},
{219,199,135},
{231,219,163},
{247,239,195},
{71,27,0},
{95,43,0},
{119,63,0},
{143,83,7},
{167,111,7},
{191,139,15},
{215,167,19},
{243,203,27},
{255,231,47},
{255,243,95},
{255,251,143},
{255,255,195},
{35,0,0},
{79,0,0},
{95,7,7},//60
{111,15,15},
{127,27,27},
{143,39,39},
{163,59,59},
{179,79,79},
{199,103,103},
{215,127,127},
{235,159,159},
{255,191,191},
{27,51,19},
{35,63,23},
{47,79,31},
{59,95,39},
{71,111,43},
{87,127,51},
{99,143,59},
{115,155,67},
{131,171,75},
{147,187,83},
{163,203,95},//80
{183,219,103},
{31,55,27},
{47,71,35},
{59,83,43},
{75,99,55},
{91,111,67},
{111,135,79},
{135,159,95},
{159,183,111},
{183,207,127},
{195,219,147},
{207,231,167},
{223,247,191},
{15,63,0},
{19,83,0},
{23,103,0},
{31,123,0},
{39,143,7},
{55,159,23},
{71,175,39},//100
{91,191,63},
{111,207,87},
{139,223,115},
{163,239,143},
{195,255,179},
{79,43,19},
{99,55,27},
{119,71,43},
{139,87,59},
{167,99,67},
{187,115,83},
{207,131,99},
{215,151,115},
{227,171,131},
{239,191,151},
{247,207,171},
{255,227,195},
{15,19,55},
{39,43,87},
{51,55,103},//120
{63,67,119},
{83,83,139},
{99,99,155},
{119,119,175},
{139,139,191},
{159,159,207},
{183,183,223},
{211,211,239},
{239,239,255},
{0,27,111},
{0,39,151},
{7,51,167},
{15,67,187},
{27,83,203},
{43,103,223},
{67,135,227},
{91,163,231},
{119,187,239},
{143,211,243},
{175,231,251},//140
{215,247,255},
{11,43,15},
{15,55,23},
{23,71,31},
{35,83,43},
{47,99,59},
{59,115,75},
{79,135,95},
{99,155,119},
{123,175,139},
{147,199,167},
{175,219,195},
{207,243,223},
{63,0,95},
{75,7,115},
{83,15,127},
{95,31,143},
{107,43,155},
{123,63,171},
{135,83,187},//160
{155,103,199},
{171,127,215},
{191,155,231},
{215,195,243},
{243,235,255},
{63,0,0},
{87,0,0},
{115,0,0},
{143,0,0},
{171,0,0},
{199,0,0},
{227,7,0},
{255,7,0},
{255,79,67},
{255,123,115},
{255,171,163},
{255,219,215},
{79,39,0},
{111,51,0},
{147,63,0},//180
{183,71,0},
{219,79,0},
{255,83,0},
{255,111,23},
{255,139,51},
{255,163,79},
{255,183,107},
{255,203,135},
{255,219,163},
{0,51,47},
{0,63,55},
{0,75,67},
{0,87,79},
{7,107,99},
{23,127,119},
{43,147,143},
{71,167,163},
{99,187,187},
{131,207,207},
{171,231,231},//200
{207,255,255},
{63,0,27},
{103,0,51},
{123,11,63},
{143,23,79},
{163,31,95},
{183,39,111},
{219,59,143},
{239,91,171},
{243,119,187},
{247,151,203},
{251,183,223},
{255,215,239},
{39,19,0},
{55,31,7},
{71,47,15},
{91,63,31},
{107,83,51},
{123,103,75},
{143,127,107},//220
{163,147,127},
{187,171,147},
{207,195,171},
{231,219,195},
{255,243,223},
{55,75,75},
{255,183,0},
{255,219,0},
{255,255,0},
{39,143,135},
{7,107,99},
{7,107,99},
{7,107,99},
{27,131,123},
{155,227,227},
{55,155,151},
{55,155,151},
{55,155,151},
{115,203,203},
{67,91,91},//240
{83,107,107},
{99,123,123},
{111,51,47},
{131,55,47},
{151,63,51},
{171,67,51},
{191,75,47},
{211,79,43},
{231,87,35},
{255,95,31},
{255,127,39},
{255,155,51},
{255,183,63},
{255,207,75}
/*Green remap
{8,67,8},
{16,85,16},
{24,103,24},
{32,121,32},
{40,139,40},
{48,157,48},
{56,175,56},
{64,193,64},
{72,211,72},
{80,219,80},
{88,237,88},
{92,255,92}
*/
};

void image_new(image_t* image, uint16_t width, uint16_t height, int16_t x_offset, int16_t y_offset, uint16_t flags)
{
    image->width = width;
    image->height = height;
    image->x_offset = x_offset;
    image->y_offset = y_offset;
    image->flags = flags;
    image->pixels = (uint8_t*)calloc(width * height, sizeof(uint8_t));
}


void image_copy(image_t* src, image_t* dst)
{
    dst->width = src->width;
    dst->height = src->height;
    dst->x_offset = src->x_offset;
    dst->y_offset = src->y_offset;
    dst->flags = src->flags;
    dst->pixels = (uint8_t*)calloc(src->width * src->height, sizeof(uint8_t));
    memmove(dst->pixels, src->pixels, src->width * src->height);
}
//TODO prevent writing outside image
void image_blit(image_t* dst, image_t* src, int16_t x_offset, int16_t y_offset)
{
    x_offset += src->x_offset - dst->x_offset;
    y_offset += src->y_offset - dst->y_offset;

    for (int y = 0; y < src->height; y++)
        for (int x = 0; x < src->width; x++)
        {
            int dst_x = x_offset + x;
            int dst_y = y_offset + y;

            if (src->pixels[y * src->width + x] && dst_x >= 0 && dst_y >= 0 && dst_x < dst->width && dst_y < dst->height)dst->pixels[dst_y * dst->width + dst_x] = src->pixels[y * src->width + x];
        }
}

int image_read_png(image_t* image, FILE* file)
{
    if (!file)
    {
        return 1;
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
    {
        fclose(file);
        return 1;
    }
    png_infop info = png_create_info_struct(png);
    if (!info)
    {
        fclose(file);
        return 1;
    }
    if (setjmp(png_jmpbuf(png)))abort();//TODO Not sure what this does but I don't think it's what I want

    png_init_io(png, file);
    png_read_info(png, info);

    image->width = png_get_image_width(png, info);
    image->height = png_get_image_height(png, info);
    image->x_offset = 0;
    image->y_offset = 0;
    image->flags = 1;

    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (color_type != PNG_COLOR_TYPE_PALETTE)
    {
        fclose(file);
        return 1;
    }
    png_read_update_info(png, info);

    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * image->height);
    for (int y = 0; y < image->height; y++)
    {
        row_pointers[y] = (png_bytep)malloc(sizeof(png_byte) * png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    image->pixels = (uint8_t*)malloc(image->width * image->height * sizeof(uint8_t));

    for (int y = 0; y < image->height; y++)
        for (int x = 0; x < image->width; x++)
        {
            image->pixels[x + y * image->width] = (uint8_t)row_pointers[y][x];
        }
    for (int y = 0; y < image->height; y++)
    {
        free(row_pointers[y]);
    }
    free(row_pointers);
    return 0;
}

int image_write_png(image_t* image, FILE* file)
{
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)return 1;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 1;
    }

    /*TODO make this actually work*/
    //if (setjmp (png_jmpbuf (png_ptr))) {
    //    goto png_failure;
    //}

//Set image attributes
    png_set_IHDR(png_ptr, info_ptr, image->width, image->height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    //Set palette
    png_set_PLTE(png_ptr, info_ptr, palette, 256);
    //Set transparent color
    png_byte transparency = 0;
    png_set_tRNS(png_ptr, info_ptr, &transparency, 1, NULL);

    png_byte** row_pointers = (png_byte**)png_malloc(png_ptr, image->height * sizeof(png_byte*));
    for (size_t y = 0; y < image->height; y++)
    {
        row_pointers[y] = image->pixels + y * image->width;
    }
    //Write file
    png_init_io(png_ptr, file);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_free(png_ptr, row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
}

void image_crop(image_t* image)
{
    int x_min = INT32_MAX;
    int x_max = INT32_MIN;
    int y_min = INT32_MAX;
    int y_max = INT32_MIN;

    for (int y = 0; y < image->height; y++)
        for (int x = 0; x < image->width; x++)
        {
            if (image->pixels[x + y * image->width] > 0)
            {
                if (x < x_min)x_min = x;
                if (x > x_max)x_max = x;
                if (y < y_min)y_min = y;
                if (y > y_max)y_max = y;
            }
        }

    if (x_max < x_min)
    {
        image->x_offset = 0;
        image->y_offset = 0;
        image->width = 1;
        image->height = 1;
    }
    else
    {
        int stride = image->width;
        image->x_offset += x_min;
        image->y_offset += y_min;
        image->width = x_max - x_min + 1;
        image->height = y_max - y_min + 1;

        for (int y = 0; y < image->height; y++)
            for (int x = 0; x < image->width; x++)
            {
                image->pixels[x + y * image->width] = image->pixels[(x + x_min) + (y + y_min) * stride];
            }

    }
}

void image_destroy(image_t* image)
{
    free(image->pixels);
}
