#pragma once

#include <jansson.h>
#include "track.h"

void print_masks(view_t views[NUM_TRACK_SECTIONS][4]);
int load_masks(const char* filename,view_t views[NUM_TRACK_SECTIONS][4]);
int dump_mask(const view_t* view,const char* filename);
void dump_masks(const char* set_name,const view_t masks[NUM_TRACK_SECTIONS][4]);
int compare_masks(view_t a[NUM_TRACK_SECTIONS][4],view_t b[NUM_TRACK_SECTIONS][4]);
