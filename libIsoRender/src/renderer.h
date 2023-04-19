#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED
#include "image.h"
#include "model.h"
#include "palette.h"
#include "raytrace.h"
#include "vectormath.h"

#define RENDER_WIDTH 255
#define RENDER_HEIGHT 256
#define RENDER_PIXELS (RENDER_WIDTH*RENDER_HEIGHT)
#define UNITS_PER_TILE 4096
#define UNITS_PER_PIXEL 128
#define FRAGMENT_UNUSED 255
#define REGION_MASK 0x7
#define MAX_REGIONS 8

typedef struct //Rect
{
    int32_t x_lower;
    int32_t y_lower;
    int32_t x_upper;
    int32_t y_upper;
}rect_t;

typedef struct //Fragment
{
    vector3_t color;
    float depth;
    float ghost_depth;
    uint8_t flags;
    uint8_t region;
}fragment_t;

enum light_type
{
    LIGHT_HEMI,
    LIGHT_DIFFUSE,
    LIGHT_SPECULAR,
};

typedef struct //Light
{
    uint16_t type;
    uint16_t shadow;
    vector3_t direction;
    float intensity;
}light_t;

typedef struct
{
    uint16_t width;
    uint16_t height;
    vector2_t offset;
    fragment_t* fragments;
}framebuffer_t;

typedef struct
{
    uint32_t num_lights;
    light_t* lights;
    matrix_t projection;
    device_t rt_device;
    scene_t rt_scene;
    palette_t palette;
}context_t;

extern matrix_t views[4];

void context_init(context_t* context, light_t* lights, uint32_t num_lights, palette_t palette, float upt);
void context_destroy(context_t* context);
void context_begin_render(context_t* context);
void context_add_model(context_t* context, mesh_t* mesh, transform_t transform, int mask);
void context_add_model_transformed(context_t* context, mesh_t* mesh, vertex_t(*transform)(vector3_t, vector3_t, void*), void* data, int mask);
void context_render_view(context_t* context, matrix_t view_matrix, image_t* image);
void context_render_silhouette(context_t* context, matrix_t view, image_t* image);
void context_finalize_render(context_t* context);
void context_end_render(context_t* context);

#endif // RENDERER_H_INCLUDED
