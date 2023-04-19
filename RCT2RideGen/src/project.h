#ifndef PROJECT_H_INCLUDED
#define PROJECT_H_INCLUDED

#include<stdint.h>
#include<renderer.h>

#define TILE_SIZE 3.3

#define PROJECT_MAX_MESHES 16
#define PROJECT_MAX_MESHES_PER_MODEL 8
#define PROJECT_MAX_FRAMES 4
#define PROJECT_MAX_VEHICLES 4
#define PROJECT_MAX_RIDERS 16
#define PROJECT_MAX_COLORS 8

enum
{
SPRITE_FLAT_SLOPE=0x0001,
SPRITE_GENTLE_SLOPE=0x0002,
SPRITE_STEEP_SLOPE=0x0004,
SPRITE_VERTICAL_SLOPE=0x0008,
SPRITE_DIAGONAL_SLOPE=0x0010,
SPRITE_BANKING=0x0020,
SPRITE_INLINE_TWIST=0x0040,
SPRITE_SLOPE_BANK_TRANSITION=0x0080,
SPRITE_DIAGONAL_BANK_TRANSITION=0x0100,
SPRITE_SLOPED_BANK_TRANSITION=0x0200,
SPRITE_SLOPED_BANKED_TURN=0x0400,
SPRITE_BANKED_SLOPE_TRANSITION=0x0800,
SPRITE_CORKSCREW=0x1000,
SPRITE_ZERO_G_ROLL=0x2000,
SPRITE_DIAGONAL_SLOPED_BANK_TRANSITION=0x4000
};

enum
{
RIDE_NO_COLLISION_CRASHES=1,
RIDE_RIDER_CONTROLS_SPEED=2
};

enum
{
VEHICLE_SECONDARY_REMAP=1,
VEHICLE_TERTIARY_REMAP=2,
VEHICLE_RIDERS_SCREAM=4,
VEHICLE_RESTRAINT_ANIMATION=8
};

enum
{
RUNNING_SOUND_WOODEN_OLD=1,
RUNNING_SOUND_WOODEN_MODERN=54,
RUNNING_SOUND_STEEL=2,
RUNNING_SOUND_STEEL_SMOOTH=57,
RUNNING_SOUND_WATERSLIDE=32,
RUNNING_SOUND_TRAIN=31,
RUNNING_SOUND_ENGINE=21,
RUNNING_SOUND_NONE=255
};

enum
{
SECONDARY_SOUND_SCREAMS_1=0,
SECONDARY_SOUND_SCREAMS_2=1,
SECONDARY_SOUND_SCREAMS_3=2,
SECONDARY_SOUND_WHISTLE=3,
SECONDARY_SOUND_BELL=4,
SECONDARY_SOUND_NONE=255,
};

enum
{
CAR_INDEX_DEFAULT=0,
CAR_INDEX_FRONT=1,
CAR_INDEX_SECOND=2,
CAR_INDEX_THIRD=4,
CAR_INDEX_REAR=3
};

enum
{
CATEGORY_TRANSPORT_RIDE=0,
CATEGORY_GENTLE_RIDE=1,
CATEGORY_ROLLERCOASTER=2,
CATEGORY_THRILL_RIDE=3,
CATEGORY_WATER_RIDE=4,
CATEGORY_SHOP=5
};


typedef struct
{
int32_t num_meshes;
int32_t mesh_index[PROJECT_MAX_MESHES_PER_MODEL][PROJECT_MAX_FRAMES];
vector3_t position[PROJECT_MAX_MESHES_PER_MODEL][PROJECT_MAX_FRAMES];
vector3_t orientation[PROJECT_MAX_MESHES_PER_MODEL][PROJECT_MAX_FRAMES];
}model_t;

typedef struct
{
model_t model;
uint32_t flags;
uint32_t mass;
uint32_t num_sprites;
uint32_t draw_order;
uint32_t num_riders;
uint32_t num_rider_models;
float spacing;
model_t riders[PROJECT_MAX_RIDERS];
}vehicle_t;

typedef struct
{
uint8_t* id;
uint8_t* original_id;
uint8_t* name;
uint8_t* description;
uint8_t* capacity;
uint8_t* author;
uint8_t* version;
uint8_t* ride_type;
uint8_t configuration[5];
uint32_t flags;
uint32_t zero_cars;
uint32_t min_cars_per_train;
uint32_t max_cars_per_train;
uint32_t category;
uint32_t build_menu_priority;
uint32_t tab_car;
uint32_t running_sound;
uint32_t secondary_sound;
uint32_t colors[PROJECT_MAX_COLORS][3];
uint32_t num_colors;
uint32_t sprite_flags;
uint32_t num_sprites;
uint32_t num_vehicles;
uint32_t num_meshes;
mesh_t meshes[PROJECT_MAX_MESHES];
vehicle_t vehicles[PROJECT_MAX_VEHICLES];
image_t preview;
}project_t;

int count_animation_frames(uint16_t sprites);
int count_sprites_from_flags(uint16_t sprites,int flags);
int project_export(project_t* project,context_t* context,const char* output_directory,int skip_render);
int project_export_test(project_t* project,context_t* context);


#define NUM_SPRITE_GROUPS 15
#define NUM_FLAGS 2
#define NUM_VEHICLE_FLAGS 4
#define NUM_RUNNING_SOUNDS 6
#define NUM_SECONDARY_SOUNDS 4
#define NUM_COLORS 32
#define NUM_CATEGORIES 4

extern const char* sprite_group_names[NUM_SPRITE_GROUPS];
extern const char* flag_names[NUM_FLAGS];
extern const char* vehicle_flag_names[NUM_VEHICLE_FLAGS];
extern const char* running_sounds[NUM_RUNNING_SOUNDS];
extern const char* secondary_sounds[NUM_SECONDARY_SOUNDS];
extern const char* color_names[NUM_COLORS];
extern const char* category_names[NUM_CATEGORIES];



#endif
