#ifdef _MSC_VER
#pragma warning(disable:4305)
#pragma warning(disable:4244)
#endif
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <jansson.h>
#include <zip.h>
#include <image.h>
#include "project.h"

void print_msg(const char* fmt, ...);

#define MAX_PATH_LENGTH 512

#define M_PI_8 (M_PI/8.0)
#define M_PI_12 (M_PI/12.0)

#define TILE_SLOPE (1 / sqrt(6))

#define FLAT 0
#define GENTLE (atan(TILE_SLOPE))
#define STEEP (atan(4 * TILE_SLOPE))
#define VERTICAL M_PI_2
#define FG_TRANSITION ((FLAT + GENTLE) / 2)
#define GS_TRANSITION ((GENTLE + STEEP) / 2)
#define SV_TRANSITION ((STEEP + VERTICAL) / 2)

#define GENTLE_DIAGONAL (atan(TILE_SLOPE * M_SQRT1_2))
#define STEEP_DIAGONAL (atan(4 * TILE_SLOPE * M_SQRT1_2))
#define FG_TRANSITION_DIAGONAL ((FLAT + GENTLE_DIAGONAL) / 2)

#define BANK M_PI_4
#define BANK_TRANSITION (BANK / 2)

#define CORKSCREW_RIGHT_YAW(angle) \
    (atan2(0.5 * (1 - cos((angle))), 1 - 0.5 * (1 - cos((angle)))))
#define CORKSCREW_RIGHT_PITCH(angle) (-asin(-sin((angle)) / sqrt(2.0)))
#define CORKSCREW_RIGHT_ROLL(angle) (-atan2(sin((angle)) / sqrt(2.0), cos((angle))))

#define CORKSCREW_LEFT_YAW(angle) (-CORKSCREW_RIGHT_YAW((angle)))
#define CORKSCREW_LEFT_PITCH(angle) (-CORKSCREW_RIGHT_PITCH(-(angle)))
#define CORKSCREW_LEFT_ROLL(angle) (-CORKSCREW_RIGHT_ROLL((angle)))


const char* sprite_group_names[NUM_SPRITE_GROUPS] = { "flat","gentle_slopes","steep_slopes","vertical_slopes","diagonals","banked_turns","inline_twists","slope_bank_transition","diagonal_bank_transition","sloped_bank_transition","banked_sloped_turns","banked_slope_transition","corkscrews","zero_g_rolls","diagonal_sloped_bank_transition","dive_loops" };
const char* flag_names[NUM_FLAGS] = { "no_collision_crashes","rider_controls_speed" };
const char* vehicle_flag_names[NUM_VEHICLE_FLAGS] = { "secondary_remap","tertiary_remap","riders_scream","restraint_animation" };
const char* running_sounds[NUM_RUNNING_SOUNDS] = { "wooden_old","wooden","steel","steel_smooth","train","engine" };
const char* secondary_sounds[NUM_SECONDARY_SOUNDS] = { "scream1","scream2","scream3","bell" };
const char* color_names[NUM_COLORS] = { "black","grey","white","dark_purple","light_purple","bright_purple","dark_blue","light_blue","icy_blue","teal","aquamarine","saturated_green","dark_green","moss_green","bright_green","olive_green","dark_olive_green","bright_yellow","yellow","dark_yellow","light_orange","dark_orange","light_brown","saturated_brown","dark_brown","salmon_pink","bordeaux_red","saturated_red","bright_red","dark_pink","bright_pink","light_pink" };
const char* category_names[NUM_CATEGORIES] = { "transport","gentle","water","rollercoaster" };


json_t* json_image(const char* path, int x, int y, int src_x, int src_y, int src_width, int src_height)
{
    json_t* image = json_object();
    json_object_set_new(image, "path", json_string(path));
    json_object_set_new(image, "x", json_integer(x));
    json_object_set_new(image, "y", json_integer(y));
    assert(src_width != 0 && src_height != 0);

    if (src_x >= 0)json_object_set_new(image, "srcX", json_integer(src_x));
    if (src_y >= 0)json_object_set_new(image, "srcY", json_integer(src_y));
    if (src_width > 0)json_object_set_new(image, "srcWidth", json_integer(src_width));
    if (src_height > 0)json_object_set_new(image, "srcHeight", json_integer(src_height));

    json_object_set_new(image, "palette", json_string("keep"));
    return image;
}

void render_rotation(context_t* context, int num_frames, float pitch, float roll, float yaw, image_t* images)
{
    for (int i = 0; i < num_frames; i++)
    {
        context_render_view(context, matrix_mult(rotate_y(yaw + (2 * i * M_PI) / num_frames), matrix_mult(rotate_z(pitch), rotate_x(roll))), images + i);
    }

}

int render_vehicle(context_t* context, project_t* project, int i, image_t* images, int frame)
{
    //Currently only restraint animations are supported
    if (frame > 0)
    {
        print_msg("Rendering restraint animation");
        render_rotation(context, 4, FLAT, 0, 0, images);
        return 4;
    }

    int sprite_flags = project->sprite_flags;

    int base = 0;
    if (sprite_flags & SPRITE_FLAT_SLOPE)
    {
        print_msg("Rendering flat sprites");
        render_rotation(context, 32, FLAT, 0, 0, images + base);
        base += 32;
    }
    if (sprite_flags & SPRITE_GENTLE_SLOPE)
    {
        print_msg("Rendering gentle sprites");
        render_rotation(context, 4, FG_TRANSITION, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, -FG_TRANSITION, 0, 0, images + base);
        base += 4;
        render_rotation(context, 32, GENTLE, 0, 0, images + base);
        base += 32;
        render_rotation(context, 32, -GENTLE, 0, 0, images + base);
        base += 32;
    }
    if (sprite_flags & SPRITE_STEEP_SLOPE)
    {
        print_msg("Rendering steep sprites");
        render_rotation(context, 8, GS_TRANSITION, 0, 0, images + base);
        base += 8;
        render_rotation(context, 8, -GS_TRANSITION, 0, 0, images + base);
        base += 8;
        render_rotation(context, 32, STEEP, 0, 0, images + base);
        base += 32;
        render_rotation(context, 32, -STEEP, 0, 0, images + base);
        base += 32;
    }
    if (sprite_flags & SPRITE_VERTICAL_SLOPE)
    {
        print_msg("Rendering vertical sprites");
        render_rotation(context, 4, SV_TRANSITION, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, -SV_TRANSITION, 0, 0, images + base);
        base += 4;
        render_rotation(context, 32, VERTICAL, 0, 0, images + base);
        base += 32;
        render_rotation(context, 32, -VERTICAL, 0, 0, images + base);
        base += 32;
        render_rotation(context, 4, VERTICAL + M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, -VERTICAL - M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, VERTICAL + 2 * M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, -VERTICAL - 2 * M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, VERTICAL + 3 * M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, -VERTICAL - 3 * M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, VERTICAL + 4 * M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, -VERTICAL - 4 * M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, VERTICAL + 5 * M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, -VERTICAL - 5 * M_PI_12, 0, 0, images + base);
        base += 4;
        render_rotation(context, 4, M_PI, 0, 0, images + base);
        base += 4;
    }
    if (sprite_flags & SPRITE_DIAGONAL_SLOPE)
    {
        print_msg("Rendering diagonal sprites");
        render_rotation(context, 4, FG_TRANSITION_DIAGONAL, 0, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -FG_TRANSITION_DIAGONAL, 0, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE_DIAGONAL, 0, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE_DIAGONAL, 0, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, STEEP_DIAGONAL, 0, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -STEEP_DIAGONAL, 0, M_PI_4, images + base);
        base += 4;
    }
    if (sprite_flags & SPRITE_BANKING)
    {
        print_msg("Rendering banked sprites");
        render_rotation(context, 8, FLAT, BANK_TRANSITION, 0, images + base);
        base += 8;
        render_rotation(context, 8, FLAT, -BANK_TRANSITION, 0, images + base);
        base += 8;
        render_rotation(context, 32, FLAT, BANK, 0, images + base);
        base += 32;
        render_rotation(context, 32, FLAT, -BANK, 0, images + base);
        base += 32;
    }
    if (sprite_flags & SPRITE_INLINE_TWIST)
    {
        print_msg("Rendering inline twist sprites");
        render_rotation(context, 4, FLAT, 3.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, -3.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, M_PI_2, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, -M_PI_2, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, 5.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, -5.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, 3.0 * M_PI_4, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, -3.0 * M_PI_4, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, 7.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, FLAT, -7.0 * M_PI_8, 0, images + base);
        base += 4;
    }
    if (sprite_flags & SPRITE_SLOPE_BANK_TRANSITION)
    {
        print_msg("Rendering slope-bank transition sprites");
        render_rotation(context, 32, FG_TRANSITION, BANK_TRANSITION, 0, images + base);
        base += 32;
        render_rotation(context, 32, FG_TRANSITION, -BANK_TRANSITION, 0, images + base);
        base += 32;
        render_rotation(context, 32, -FG_TRANSITION, BANK_TRANSITION, 0, images + base);
        base += 32;
        render_rotation(context, 32, -FG_TRANSITION, -BANK_TRANSITION, 0, images + base);
        base += 32;
    }
    if (sprite_flags & SPRITE_DIAGONAL_BANK_TRANSITION)
    {
        print_msg("Rendering diagonal slope-bank transition sprites");
        render_rotation(context, 4, FG_TRANSITION_DIAGONAL, BANK_TRANSITION, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, FG_TRANSITION_DIAGONAL, -BANK_TRANSITION, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -FG_TRANSITION_DIAGONAL, BANK_TRANSITION, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -FG_TRANSITION_DIAGONAL, -BANK_TRANSITION, M_PI_4, images + base);
        base += 4;
    }
    if (sprite_flags & SPRITE_SLOPED_BANK_TRANSITION)
    {
        print_msg("Rendering sloped bank transition sprites");
        render_rotation(context, 4, GENTLE, BANK_TRANSITION, 0, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE, -BANK_TRANSITION, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, BANK_TRANSITION, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, -BANK_TRANSITION, 0, images + base);
        base += 4;
    }
    if (sprite_flags & SPRITE_DIAGONAL_SLOPED_BANK_TRANSITION)
    {
        print_msg("Rendering diagonal sloped bank transition sprites");
        render_rotation(context, 4, FG_TRANSITION_DIAGONAL, BANK, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, FG_TRANSITION_DIAGONAL, -BANK, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -FG_TRANSITION_DIAGONAL, BANK, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -FG_TRANSITION_DIAGONAL, -BANK, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE_DIAGONAL, BANK_TRANSITION, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE_DIAGONAL, -BANK_TRANSITION, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE_DIAGONAL, BANK_TRANSITION, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE_DIAGONAL, -BANK_TRANSITION, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE_DIAGONAL, BANK, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE_DIAGONAL, -BANK, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE_DIAGONAL, BANK, M_PI_4, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE_DIAGONAL, -BANK, M_PI_4, images + base);
        base += 4;
    }
    if (sprite_flags & SPRITE_SLOPED_BANKED_TURN)
    {
        print_msg("Rendering sloped banked sprites");
        render_rotation(context, 32, GENTLE, BANK, 0, images + base);
        base += 32;
        render_rotation(context, 32, GENTLE, -BANK, 0, images + base);
        base += 32;
        render_rotation(context, 32, -GENTLE, BANK, 0, images + base);
        base += 32;
        render_rotation(context, 32, -GENTLE, -BANK, 0, images + base);
        base += 32;
    }
    if (sprite_flags & SPRITE_BANKED_SLOPE_TRANSITION)
    {
        print_msg("Rendering banked slope transition sprites");
        render_rotation(context, 4, FG_TRANSITION, BANK, 0, images + base);
        base += 4;
        render_rotation(context, 4, FG_TRANSITION, -BANK, 0, images + base);
        base += 4;
        render_rotation(context, 4, -FG_TRANSITION, BANK, 0, images + base);
        base += 4;
        render_rotation(context, 4, -FG_TRANSITION, -BANK, 0, images + base);
        base += 4;
    }
    if (sprite_flags & SPRITE_ZERO_G_ROLL)
    {
        print_msg("Rendering zero G roll sprites");
        //Gentle bank 67.5
        render_rotation(context, 4, GENTLE, 3.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE, -3.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, 3.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, -3.0 * M_PI_8, 0, images + base);
        base += 4;
        //Gentle bank 90
        render_rotation(context, 4, GENTLE, M_PI_2, 0, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE, -M_PI_2, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, M_PI_2, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, -M_PI_2, 0, images + base);
        base += 4;

        //Gentle 112.5
        render_rotation(context, 4, GENTLE, 5.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE, -5.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, 5.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, -5.0 * M_PI_8, 0, images + base);
        base += 4;

        //Gentle bank 135
        render_rotation(context, 4, GENTLE, 3.0 * M_PI_4, 0, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE, -3.0 * M_PI_4, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, 3.0 * M_PI_4, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, -3.0 * M_PI_4, 0, images + base);
        base += 4;

        //Gentle bank 157.5	
        render_rotation(context, 4, GENTLE, 7.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, GENTLE, -7.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, 7.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GENTLE, -7.0 * M_PI_8, 0, images + base);
        base += 4;

        //Gentle-to-steep bank 22.5
        render_rotation(context, 4, GS_TRANSITION, M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, GS_TRANSITION, -M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GS_TRANSITION, M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GS_TRANSITION, -M_PI_8, 0, images + base);
        base += 4;

        //Gentle-to-steep bank 45
        render_rotation(context, 4, GS_TRANSITION, 2.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, GS_TRANSITION, -2.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GS_TRANSITION, 2.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GS_TRANSITION, -2.0 * M_PI_8, 0, images + base);
        base += 4;

        //Gentle-to-steep bank 67.5
        render_rotation(context, 4, GS_TRANSITION, 3.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, GS_TRANSITION, -3.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GS_TRANSITION, 3.0 * M_PI_8, 0, images + base);
        base += 4;
        render_rotation(context, 4, -GS_TRANSITION, -3.0 * M_PI_8, 0, images + base);
        base += 4;

        //Gentle-to-steep bank 90
	render_rotation(context, 4, GS_TRANSITION, M_PI_2, 0, images + base);
	base += 4;
	render_rotation(context, 4, GS_TRANSITION, -M_PI_2, 0, images + base);
	base += 4;
	render_rotation(context, 4, -GS_TRANSITION, M_PI_2, 0, images + base);
	base += 4;
	render_rotation(context, 4, -GS_TRANSITION, -M_PI_2, 0, images + base);
	base += 4;

        //Steep bank 22.5
		if (sprite_flags & SPRITE_DIVE_LOOP)
		{
		render_rotation(context, 8, STEEP, M_PI_8, 0, images + base);
		base += 8;
		render_rotation(context, 8, STEEP, -M_PI_8, 0, images + base);
		base += 8;
		render_rotation(context, 8, -STEEP, M_PI_8, 0, images + base);
		base += 8;
		render_rotation(context, 8, -STEEP, -M_PI_8, 0, images + base);
		base += 8;
		}
		else
		{
		render_rotation(context, 4, STEEP, M_PI_8, 0, images + base);
		base += 4;
		render_rotation(context, 4, STEEP, -M_PI_8, 0, images + base);
		base += 4;
		render_rotation(context, 4, -STEEP, M_PI_8, 0, images + base);
		base += 4;
		render_rotation(context, 4, -STEEP, -M_PI_8, 0, images + base);
		base += 4;
		}
    }
    if (sprite_flags & SPRITE_DIVE_LOOP)
    {
        print_msg("Rendering dive loop sprites");
        //Steep bank 45
        render_rotation(context, 8, STEEP_DIAGONAL, M_PI_4, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, STEEP_DIAGONAL, -M_PI_4, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, -STEEP_DIAGONAL, M_PI_4, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, -STEEP_DIAGONAL, -M_PI_4, M_PI_8, images + base);
        base += 8;

        //Steep bank 67.5
        render_rotation(context, 8, STEEP_DIAGONAL, 3*M_PI_8, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, STEEP_DIAGONAL, -3*M_PI_8, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, -STEEP_DIAGONAL, 3*M_PI_8, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, -STEEP_DIAGONAL, -3*M_PI_8, M_PI_8, images + base);
        base += 8;

        //Diagonal steep bank 90
        render_rotation(context, 8, STEEP_DIAGONAL, M_PI_2, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, STEEP_DIAGONAL, -M_PI_2, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, -STEEP_DIAGONAL, M_PI_2, M_PI_8, images + base);
        base += 8;
        render_rotation(context, 8, -STEEP_DIAGONAL, -M_PI_2, M_PI_8, images + base);
        base += 8;
    }

    if (sprite_flags & SPRITE_CORKSCREW)
    {
        print_msg("Rendering corkscrew sprites");
#define CORKSCREW_ANGLE_1 2.0 * M_PI_12
#define CORKSCREW_ANGLE_2 4.0 * M_PI_12
#define CORKSCREW_ANGLE_3 M_PI_2
#define CORKSCREW_ANGLE_4 8.0 * M_PI_12
#define CORKSCREW_ANGLE_5 10.0 * M_PI_12

        // Corkscrew right
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_1), CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_1), CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_1), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_2), CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_2), CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_2), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_3), CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_3), CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_3), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_4), CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_4), CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_4), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(CORKSCREW_ANGLE_5), CORKSCREW_RIGHT_ROLL(CORKSCREW_ANGLE_5), CORKSCREW_RIGHT_YAW(CORKSCREW_ANGLE_5), images + base);
        base += 4;

        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_1), CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_1), CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_1), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_2), CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_2), CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_2), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_3), CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_3), CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_3), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_4), CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_4), CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_4), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_RIGHT_PITCH(-CORKSCREW_ANGLE_5), CORKSCREW_RIGHT_ROLL(-CORKSCREW_ANGLE_5), CORKSCREW_RIGHT_YAW(-CORKSCREW_ANGLE_5), images + base);
        base += 4;

        // Half corkscrew left
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_1), CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_1), CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_1), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_2), CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_2), CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_2), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_3), CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_3), CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_3), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_4), CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_4), CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_4), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(CORKSCREW_ANGLE_5), CORKSCREW_LEFT_ROLL(CORKSCREW_ANGLE_5), CORKSCREW_LEFT_YAW(CORKSCREW_ANGLE_5), images + base);
        base += 4;

        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_1), CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_1), CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_1), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_2), CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_2), CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_2), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_3), CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_3), CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_3), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_4), CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_4), CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_4), images + base);
        base += 4;
        render_rotation(context, 4, CORKSCREW_LEFT_PITCH(-CORKSCREW_ANGLE_5), CORKSCREW_LEFT_ROLL(-CORKSCREW_ANGLE_5), CORKSCREW_LEFT_YAW(-CORKSCREW_ANGLE_5), images + base);
        base += 4;
    }

    return base;
}



int count_sprites_from_flags(uint16_t sprites, int flags)
{
    int count = 0;
    if (sprites & SPRITE_FLAT_SLOPE)count += 32;
    if (sprites & SPRITE_GENTLE_SLOPE)count += 72;
    if (sprites & SPRITE_STEEP_SLOPE)count += 80;
    if (sprites & SPRITE_VERTICAL_SLOPE)count += 116;
    if (sprites & SPRITE_DIAGONAL_SLOPE)count += 24;
    if (sprites & SPRITE_BANKING)count += 80;
    if (sprites & SPRITE_INLINE_TWIST)count += 40;
    if (sprites & SPRITE_SLOPE_BANK_TRANSITION)count += 128;
    if (sprites & SPRITE_DIAGONAL_BANK_TRANSITION)count += 16;
    if (sprites & SPRITE_SLOPED_BANK_TRANSITION)count += 16;
    if (sprites & SPRITE_DIAGONAL_SLOPED_BANK_TRANSITION)count += 48;
    if (sprites & SPRITE_SLOPED_BANKED_TURN)count += 128;
    if (sprites & SPRITE_BANKED_SLOPE_TRANSITION)count += 16;
    if (sprites & SPRITE_CORKSCREW)count += 80;
    if (sprites & SPRITE_ZERO_G_ROLL)count += 160;
    if (sprites & SPRITE_DIVE_LOOP)count += 112;
    if (flags & VEHICLE_RESTRAINT_ANIMATION)count += 12;
    return count;
}


void project_add_model_to_context(project_t* project, context_t* context, model_t* model, int frame, int mask)
{
    for (int i = 0; i < model->num_meshes; i++)
    {
        if (model->mesh_index[i][frame] == -1)continue;
        vector3_t orientation = vector3_mult(model->orientation[i][frame], M_PI / 180.0);
        context_add_model(context, project->meshes + model->mesh_index[i][frame], transform(matrix_mult(rotate_y(orientation.x), matrix_mult(rotate_z(orientation.y), rotate_x(orientation.z))), model->position[i][frame]), mask);
    }
}

int project_parkobj_add(project_t* project, zip_t* archive, const char* archive_path)
{
    char path[MAX_PATH_LENGTH];
    snprintf(path, MAX_PATH_LENGTH, "object/%s", archive_path);

    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        print_msg("Error: Unable to open \"%s\"", path);
        zip_close(archive);
        return 1;
    }

    zip_source_t* src = zip_source_filep(archive, file, 0, -1);
    if (src == NULL)
    {
        print_msg("Error: zip_source_file failed");
        zip_close(archive);
        return 1;
    }

    if (zip_file_add(archive, archive_path, src, ZIP_FL_OVERWRITE) < 0)
    {
        print_msg("Error: Unable to add \"%s\" to ZIP archive", archive_path);
        zip_close(archive);
        return 1;
    }
    return 0;
}

void project_clean_working_dir(project_t* project)
{
    remove("object/object.json");
    remove("object/images/preview.png");
    for (int i = 0; i < project->num_vehicles; i++)
    {
        char path[MAX_PATH_LENGTH];
        snprintf(path, MAX_PATH_LENGTH, "images/car_%d.png", i);
        remove(path);
    }
}

json_t* project_generate_json(project_t* project)
{
	//Create JSON file
	json_t* json = json_object();
	json_object_set_new(json, "id", json_string((char*)project->id));
	if (project->original_id)json_object_set_new(json, "originalId", json_string((char*)project->original_id));
	json_object_set_new(json, "version", json_string((char*)project->version));
	json_t* authors = json_array();
	if (project->author != NULL)json_array_append_new(authors, json_string((char*)project->author));
	json_object_set_new(json, "authors", authors);
	
	json_object_set_new(json, "objectType", json_string("ride"));
	
	//Ride header
	json_t* properties = json_object();
	json_t* types = json_array();
	json_array_append_new(types, json_string((char*)project->ride_type));
	json_object_set_new(properties, "type", types);
	json_object_set_new(properties, "category", json_string(category_names[project->category]));
	json_object_set_new(properties, "minCarsPerTrain", json_integer(project->min_cars_per_train));
	json_object_set_new(properties, "maxCarsPerTrain", json_integer(project->max_cars_per_train));
	json_object_set_new(properties, "numEmptyCars", json_integer(project->zero_cars));
	json_object_set_new(properties, "tabCar", json_integer(project->tab_car));
	json_object_set_new(properties, "defaultCar", json_integer(project->configuration[CAR_INDEX_DEFAULT]));
	if (project->configuration[CAR_INDEX_FRONT] != 0xFF)json_object_set_new(properties, "headCars", json_integer(project->configuration[CAR_INDEX_FRONT]));//TODO support multiple head cars
	if (project->configuration[CAR_INDEX_REAR] != 0xFF)json_object_set_new(properties, "tailCars", json_integer(project->configuration[CAR_INDEX_REAR]));
	json_object_set_new(properties, "buildMenuPriority", json_integer(project->build_menu_priority));
	
	
	if (project->flags & RIDE_NO_COLLISION_CRASHES)json_object_set_new(properties, "noCollisionCrashes", json_true());
	if (project->flags & RIDE_RIDER_CONTROLS_SPEED)json_object_set_new(properties, "riderControlsSpeed", json_true());
	
	//Color presets
	json_t* car_color_presets = json_array();
		for (int i = 0; i < project->num_colors; i++)
		{
		json_t* car_color_preset = json_array();
		json_array_append_new(car_color_preset, json_string(color_names[project->colors[i][0]]));
		json_array_append_new(car_color_preset, json_string(color_names[project->colors[i][1]]));
		json_array_append_new(car_color_preset, json_string(color_names[project->colors[i][2]]));
		json_t* arr = json_array();
		json_array_append_new(arr, car_color_preset);//Presets are arrays of arrays for some reason?
		json_array_append_new(car_color_presets, arr);
		}
	json_object_set_new(properties, "carColours", car_color_presets);
	
	json_t* cars = json_array();
	for (int i = 0; i < project->num_vehicles; i++)
	{
		json_t* car = json_object();
		json_object_set_new(car, "rotationFrameMask", json_integer(31));
		json_object_set_new(car, "spacing", json_integer((project->vehicles[i].spacing * 278912) / TILE_SIZE));
		json_object_set_new(car, "mass", json_integer(project->vehicles[i].mass));
		json_object_set_new(car, "numSeats", json_integer(project->vehicles[i].num_riders));
		json_object_set_new(car, "numSeatRows", json_integer(project->vehicles[i].num_rider_models));
		int friction_sound_ids[] = { RUNNING_SOUND_WOODEN_OLD,RUNNING_SOUND_WOODEN_MODERN,RUNNING_SOUND_STEEL,RUNNING_SOUND_STEEL_SMOOTH,RUNNING_SOUND_WATERSLIDE,RUNNING_SOUND_TRAIN,RUNNING_SOUND_ENGINE };
		json_object_set_new(car, "frictionSoundId", json_integer(friction_sound_ids[project->running_sound]));
		json_object_set_new(car, "soundRange", json_integer(project->secondary_sound));
		json_object_set_new(car, "drawOrder", json_integer(project->vehicles[i].draw_order));
		/*Legacy sprite groups
		json_t* frames=json_object();
		    if(project->sprite_flags&SPRITE_FLAT_SLOPE)json_object_set_new(frames,"flat",json_true());
		    if(project->sprite_flags&SPRITE_GENTLE_SLOPE)json_object_set_new(frames,"gentleSlopes",json_true());
		    if(project->sprite_flags&SPRITE_STEEP_SLOPE)json_object_set_new(frames,"steepSlopes",json_true());
		    if(project->sprite_flags&SPRITE_VERTICAL_SLOPE)json_object_set_new(frames,"verticalSlopes",json_true());
		    if(project->sprite_flags&SPRITE_DIAGONAL_SLOPE)json_object_set_new(frames,"diagonalSlopes",json_true());
		    if(project->sprite_flags&SPRITE_BANKING)json_object_set_new(frames,"flatBanked",json_true());
		    if(project->sprite_flags&SPRITE_INLINE_TWIST)json_object_set_new(frames,"inlineTwists",json_true());
		    if(project->sprite_flags&SPRITE_SLOPE_BANK_TRANSITION)json_object_set_new(frames,"flatToGentleSlopeBankedTransitions",json_true());
		    if(project->sprite_flags&SPRITE_DIAGONAL_BANK_TRANSITION)json_object_set_new(frames,"diagonalGentleSlopeBankedTransitions",json_true());
		    if(project->sprite_flags&SPRITE_SLOPED_BANK_TRANSITION)json_object_set_new(frames,"gentleSlopeBankedTransitions",json_true());
		    if(project->sprite_flags&SPRITE_SLOPED_BANKED_TURN)json_object_set_new(frames,"gentleSlopeBankedTurns",json_true());
		    if(project->sprite_flags&SPRITE_BANKED_SLOPE_TRANSITION)json_object_set_new(frames,"flatToGentleSlopeWhileBankedTransitions",json_true());
		    if(project->sprite_flags&SPRITE_CORKSCREW)json_object_set_new(frames,"corkscrews",json_true());
		    if(project->sprite_flags&SPRITE_ZERO_G_ROLL)json_object_set_new(frames,"zeroGRolls",json_true());
		    if(project->vehicles[i].flags&VEHICLE_RESTRAINT_ANIMATION)json_object_set_new(frames,"restraintAnimation",json_true());
		json_object_set_new(car,"frames",frames);
		*/
		json_t* sprite_groups = json_object();
		if (project->sprite_flags & SPRITE_FLAT_SLOPE)json_object_set_new(sprite_groups, "slopeFlat", json_integer(32));
		if (project->sprite_flags & SPRITE_GENTLE_SLOPE)
		{
		json_object_set_new(sprite_groups, "slopes12", json_integer(4));
		json_object_set_new(sprite_groups, "slopes25", json_integer(32));
		}
		if (project->sprite_flags & SPRITE_STEEP_SLOPE)
		{
		json_object_set_new(sprite_groups, "slopes42", json_integer(8));
		json_object_set_new(sprite_groups, "slopes60", json_integer(32));
		}
		if (project->sprite_flags & SPRITE_VERTICAL_SLOPE)
		{
		json_object_set_new(sprite_groups, "slopes75", json_integer(4));
		json_object_set_new(sprite_groups, "slopes90", json_integer(32));
		json_object_set_new(sprite_groups, "slopesLoop", json_integer(4));
		json_object_set_new(sprite_groups, "slopeInverted", json_integer(4));
		}
		if (project->sprite_flags & SPRITE_DIAGONAL_SLOPE)
		{
		json_object_set_new(sprite_groups, "slopes8", json_integer(4));
		json_object_set_new(sprite_groups, "slopes16", json_integer(4));
		json_object_set_new(sprite_groups, "slopes50", json_integer(4));
		}
		if (project->sprite_flags & SPRITE_BANKING)
		{
		json_object_set_new(sprite_groups, "flatBanked22", json_integer(8));
		json_object_set_new(sprite_groups, "flatBanked45", json_integer(32));
		}
		if (project->sprite_flags & SPRITE_INLINE_TWIST)
		{
		json_object_set_new(sprite_groups, "flatBanked67", json_integer(4));
		json_object_set_new(sprite_groups, "flatBanked90", json_integer(4));
		json_object_set_new(sprite_groups, "inlineTwists", json_integer(4));
		}
		if (project->sprite_flags & SPRITE_SLOPE_BANK_TRANSITION)json_object_set_new(sprite_groups, "slopes12Banked22", json_integer(32));
		if (project->sprite_flags & SPRITE_DIAGONAL_BANK_TRANSITION)json_object_set_new(sprite_groups, "slopes8Banked22", json_integer(4));
		if (project->sprite_flags & SPRITE_SLOPED_BANK_TRANSITION)json_object_set_new(sprite_groups, "slopes25Banked22", json_integer(4));
		if (project->sprite_flags & SPRITE_DIAGONAL_SLOPED_BANK_TRANSITION)
		{
		json_object_set_new(sprite_groups, "slopes8Banked45", json_integer(4));
		json_object_set_new(sprite_groups, "slopes16Banked22", json_integer(4));
		json_object_set_new(sprite_groups, "slopes16Banked45", json_integer(4));
		}
		if (project->sprite_flags & SPRITE_SLOPED_BANKED_TURN)json_object_set_new(sprite_groups, "slopes25Banked45", json_integer(32));
		if (project->sprite_flags & SPRITE_BANKED_SLOPE_TRANSITION)json_object_set_new(sprite_groups, "slopes12Banked45", json_integer(4));
		if (project->sprite_flags & SPRITE_ZERO_G_ROLL)
		{
		json_object_set_new(sprite_groups, "slopes25Banked67", json_integer(4));
		json_object_set_new(sprite_groups, "slopes25Banked90", json_integer(4));
		json_object_set_new(sprite_groups, "slopes25InlineTwists", json_integer(4));
		json_object_set_new(sprite_groups, "slopes42Banked22", json_integer(4));
		json_object_set_new(sprite_groups, "slopes42Banked45", json_integer(4));
		json_object_set_new(sprite_groups, "slopes42Banked67", json_integer(4));
		json_object_set_new(sprite_groups, "slopes42Banked90", json_integer(4));
			if (project->sprite_flags & SPRITE_DIVE_LOOP)
			{
			json_object_set_new(sprite_groups, "slopes60Banked22", json_integer(8));
			}
			else
			{
			json_object_set_new(sprite_groups, "slopes60Banked22", json_integer(4));
			}
		}
		if (project->sprite_flags & SPRITE_DIVE_LOOP)
		{
		json_object_set_new(sprite_groups, "slopes50Banked45", json_integer(8));
		json_object_set_new(sprite_groups, "slopes50Banked67", json_integer(8));
		json_object_set_new(sprite_groups, "slopes50Banked90", json_integer(8));
		}
		if (project->sprite_flags & SPRITE_CORKSCREW)json_object_set_new(sprite_groups, "corkscrews", json_integer(4));
		if (project->vehicles[i].flags & VEHICLE_RESTRAINT_ANIMATION)json_object_set_new(sprite_groups, "restraintAnimation", json_integer(4));
		json_object_set_new(car, "spriteGroups", sprite_groups);
		
		if (project->vehicles[i].flags & VEHICLE_SECONDARY_REMAP)json_object_set_new(car, "hasAdditionalColour1", json_true());
		if (project->vehicles[i].flags & VEHICLE_TERTIARY_REMAP)json_object_set_new(car, "hasAdditionalColour2", json_true());
		if (project->vehicles[i].flags & VEHICLE_RIDERS_SCREAM)json_object_set_new(car, "hasScreamingRiders", json_true());
		json_t* loading_positions = json_array();
		for (int j = 0; j < project->vehicles[i].num_rider_models; j++)
		{
		    int position = round(32.0 * project->vehicles[i].riders[j].position[0][0].x / TILE_SIZE);
		    if (project->vehicles[i].num_riders > 1)
		    {
		        json_array_append_new(loading_positions, json_integer(position - 1));
		        json_array_append_new(loading_positions, json_integer(position + 1));
		    }
		    else json_array_append_new(loading_positions, json_integer(position));
		}
		json_object_set_new(car, "loadingPositions", loading_positions);
		json_array_append_new(cars, car);
	}
	json_object_set_new(properties, "cars", cars);
	json_object_set_new(json, "properties", properties);
	
	//String tables
	json_t* strings = json_object();
	json_t* name = json_object();
	json_object_set_new(name, "en-GB", json_string((char*)project->name));
	json_object_set_new(strings, "name", name);
	json_t* description = json_object();
	json_object_set_new(description, "en-GB", json_string((char*)project->description));
	json_object_set_new(strings, "description", description);
	json_t* capacity = json_object();
	json_object_set_new(capacity, "en-GB", json_string((char*)project->capacity));
	json_object_set_new(strings, "capacity", capacity);
	json_object_set_new(json, "strings", strings);
	
	return json;
}

json_t* project_render_sprites(project_t* project, context_t* context)
{
    json_t* images_json = json_array();

    //Write preview image
    FILE* file = fopen("object/images/preview.png", "wb");
    if (file)
    {
        image_write_png(&(project->preview), file);
        fclose(file);
    }
    else
    {
        print_msg("Failed to write file object/images/preview.png");
        json_decref(images_json);
        return NULL;
    }
    //Write preview image JSON
    for (int i = 0; i < 3; i++)
    {
        json_array_append_new(images_json, json_image("images/preview.png", 0, 0, -1, -1, -1, -1));
    }

    for (int i = 0; i < project->num_vehicles; i++)
    {
        int num_frames = project->vehicles[i].flags & VEHICLE_RESTRAINT_ANIMATION ? 4 : 1;
        int num_car_images = count_sprites_from_flags(project->sprite_flags, project->vehicles[i].flags);
        int num_images = num_car_images * (1 + project->vehicles[i].num_rider_models);
        image_t* images = (image_t*)calloc(num_images, sizeof(image_t));

        //Render vehicle
        print_msg("Rendering car sprites");
        int base = 0;
        for (int frame = 0; frame < num_frames; frame++)
        {
            context_begin_render(context);
            project_add_model_to_context(project, context, &(project->vehicles[i].model), frame, 0);
            context_finalize_render(context);
            base += render_vehicle(context, project, i, images + base, frame);
            context_end_render(context);
        }

        for (int j = 0; j < project->vehicles[i].num_rider_models; j++)
        {
            print_msg("Rendering peep sprites %d", j);
            base = 0;
            for (int frame = 0; frame < num_frames; frame++)
            {
                context_begin_render(context);
                project_add_model_to_context(project, context, &(project->vehicles[i].model), frame, 1);
                for (int k = 0; k < j; k++)project_add_model_to_context(project, context, &(project->vehicles[i].riders[k]), frame, 1);
                project_add_model_to_context(project, context, &(project->vehicles[i].riders[j]), frame, 0);
                context_finalize_render(context);
                base += render_vehicle(context, project, i, images + (j + 1) * num_car_images + base, frame);
                context_end_render(context);
            }
        }

        //Pack images into atlas
        image_t atlas;
        int* x_coords = (int*)calloc(num_images, sizeof(int));
        int* y_coords = (int*)calloc(num_images, sizeof(int));
        image_create_atlas(&atlas, images, num_images, x_coords, y_coords);
        //Write image json
        char path[MAX_PATH_LENGTH];
        snprintf(path, MAX_PATH_LENGTH, "images/car_%d.png", i);
        for (int i = 0; i < num_images; i++)
        {
            json_array_append_new(images_json, json_image(path, images[i].x_offset, images[i].y_offset, x_coords[i], y_coords[i], images[i].width, images[i].height));
        }
        //Write image file	
        snprintf(path, MAX_PATH_LENGTH, "object/images/car_%d.png", i);
        FILE* file = fopen(path, "wb");
        if (file)
        {
            image_write_png(&atlas, file);
            fclose(file);
        }
        else
        {
            print_msg("Failed to write file %s", path);
            json_decref(images_json);
            return NULL;
        }
        for (int i = 0; i < num_images; i++)image_destroy(images + i);
        free(images);
        image_destroy(&atlas);
    }
    return images_json;
}

int project_make_parkobj(project_t* project, const char* path)
{
    int error = 0;
    zip_t* archive = zip_open(path, ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (archive == NULL)
    {
        print_msg("Error: Unable to create \"%s\"", path);
        zip_close(archive);
        return 1;
    }
    if (zip_dir_add(archive, "images", ZIP_FL_ENC_UTF_8) < 0)
    {
        print_msg("Error: Unable to add subdirectory \"images\" to ZIP archive");
        zip_close(archive);
        return 1;
    }

    if (project_parkobj_add(project, archive, "object.json"))return 1;
    if (project_parkobj_add(project, archive, "images/preview.png"))return 1;
    for (int i = 0; i < project->num_vehicles; i++)
    {
        char path[MAX_PATH_LENGTH];
        snprintf(path, MAX_PATH_LENGTH, "images/car_%d.png", i);
        if (project_parkobj_add(project, archive, path))return 1;
    }
    if (zip_close(archive) < 0)
    {
        print_msg("Error: Failed to write ZIP file \"%s\"", path);
        return 1;
    }
    return 0;
}

int project_export(project_t* project, context_t* context, const char* output_directory, int skip_render)
{
    json_t* json = project_generate_json(project);

    json_t* images_json = NULL;
    if (skip_render)
    {
        //Attempt to load image list from previous object.json
        json_error_t error;
        json_t* object_json = json_load_file("object/object.json", 0, &error);
        if (object_json == NULL)
        {
            print_msg("Error: Unable to load object/object.json (file does not exist or is invalid)");
            return 1;
        }
        images_json = json_object_get(object_json, "images");
        //Do sanity checks
        if (!json_is_array(images_json))
        {
            print_msg("Error: Property \"images\" is not an array");
            return 1;
        }
        //TODO consider also checking if images has the right length
    }
    else
    {
        //Render new images
        project_clean_working_dir(project);
        images_json = project_render_sprites(project, context);
    }
    json_object_set_new(json, "images", images_json);

    json_dump_file(json, "object/object.json", JSON_INDENT(4));

    //Export .parkobj file
    char path[MAX_PATH_LENGTH];
    snprintf(path, MAX_PATH_LENGTH, "%s/%s.parkobj", output_directory, project->id);
    if (project_make_parkobj(project, path))return 1;
    return 0;
}

int project_export_test(project_t* project, context_t* context)
{
    for (int i = 0; i < project->num_vehicles; i++)
    {
        int num_frames = project->vehicles[i].flags & VEHICLE_RESTRAINT_ANIMATION ? 4 : 1;
        for (int j = 0; j < num_frames; j++)
        {
            printf("Rendering vehicle %d frame %d\n", i, j);
            int num_car_images = count_sprites_from_flags(project->sprite_flags, project->vehicles[i].flags);
            int num_images = num_car_images * (1 + project->vehicles[i].num_rider_models);
            image_t image;
            //Render vehicle
            context_begin_render(context);
            project_add_model_to_context(project, context, &(project->vehicles[i].model), j, 0);
            for (int k = 0; k < project->vehicles[i].num_rider_models; k++)
            {
                project_add_model_to_context(project, context, &(project->vehicles[i].riders[k]), j, 0);
            }
            printf("Models added\n");
            context_finalize_render(context);
            printf("Render finalized\n");
            context_render_view(context, rotate_y(M_PI), &image);
            printf("Render complete\n");
            context_end_render(context);
            printf("Cleanup complete\n");
            //Write image file
            char path[MAX_PATH_LENGTH];
            snprintf(path, MAX_PATH_LENGTH, "test/car_%d_%d.png", i, j);
            FILE* file = fopen(path, "wb");
            if (file)
            {
                image_write_png(&image, file);
                fclose(file);
            }
            else
            {
                print_msg("Failed to write file %s\n", path);
                exit(1);
            }
            image_destroy(&image);
        }
    }
    return 0;
}
