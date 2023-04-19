#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED
#include<stdio.h>
#include<stdint.h>
#include<png.h>

typedef struct
{
    uint16_t width;
    uint16_t height;
    int16_t x_offset;
    int16_t y_offset;
    uint16_t flags;
    uint8_t* pixels;
}image_t;

void image_new(image_t* image, uint16_t width, uint16_t height, int16_t x_offset, int16_t y_offset, uint16_t flags);
void image_copy(image_t* src, image_t* dst);
void image_blit(image_t* dst, image_t* src, int16_t x_offset, int16_t y_offset);
int image_read_png(image_t* image, FILE* file);
int image_write_png(image_t* image, FILE* file);
void image_create_atlas(image_t* image, image_t* images, int num_images, int* x_offsets, int* y_offsets);
void image_create_grid(image_t* image, image_t* images, int num_images, int* x_offsets, int* y_offsets, int columns);
void image_crop(image_t* image);
void image_destroy(image_t* image);

#endif // IMAGE_H_INCLUDED
