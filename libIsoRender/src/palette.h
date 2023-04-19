#ifndef PALETTE_H_INCLUDED
#define PALETTE_H_INCLUDED
#include<stdint.h>
#include<stdbool.h>
#include "color.h"
#include "vectormath.h"

#define NUM_REGIONS 8

#define NUM_SUBREGIONS 4

typedef struct
{
    uint8_t subregions;
    uint8_t start_indices[NUM_SUBREGIONS];
    uint8_t end_indices[NUM_SUBREGIONS];
    bool remap;
}region_t;

typedef struct
{
    uint8_t transparent_index;
    region_t regions[NUM_REGIONS];
    color_t colors[256];
    color_t remap_colors[16];
}palette_t;

color_t color(uint8_t r, uint8_t g, uint8_t b);
vector3_t vector_from_color(color_t);
color_t color_from_vector(vector3_t vector);

//ASSUME: region<MAX_REGION
uint8_t palette_get_nearest(palette_t* palette, uint8_t region, vector3_t color, vector3_t* error);

palette_t palette_rct2();

#endif // PALETTE_H_INCLUDED





