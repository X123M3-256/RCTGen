#ifndef RAYTRACE_H_INCLUDED
#define RAYTRACE_H_INCLUDED
#include <embree3/rtcore.h>
#include "model.h"
#include "vectormath.h"

#define MAX_MESHES 128

typedef RTCDevice device_t;

typedef struct
{
    vector3_t vertex;
    vector3_t normal;
}vertex_t;

typedef enum
{
    MESH_MASK = 1,
    MESH_GHOST = 2,
}mesh_flags_t;

typedef struct
{
    struct context_s* context;
    mesh_t* meshes[MAX_MESHES];
    uint64_t mask[2];
    uint64_t ghost[2];
    uint32_t num_meshes;
    float x_min, x_max, y_min, y_max, z_min, z_max;
    RTCDevice embree_device;
    RTCScene embree_scene;
}scene_t;

typedef struct
{
    uint32_t mesh_index;
    uint32_t face_index;
    vector3_t position;
    vector3_t normal;
    float ghost_distance;
    float distance;
    float u, v;
}ray_hit_t;

device_t device_init();
void device_destroy(device_t device);

void scene_init(scene_t* scene, device_t device);
void scene_finalize(scene_t* scene);
void scene_destroy(scene_t* scene);
void scene_add_model(scene_t* scene, mesh_t* mesh, vertex_t(*transform)(vector3_t, vector3_t, void*), void* data, int flags);
int scene_trace_ray(scene_t* scene, vector3_t origin, vector3_t direction, ray_hit_t* hit);
int scene_trace_occlusion_ray(scene_t* scene, vector3_t origin, vector3_t direction);
int scene_is_mask(scene_t* scene, int index);
int scene_is_ghost(scene_t* scene, int index);
#endif
