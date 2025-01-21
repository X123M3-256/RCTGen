#ifdef _MSC_VER
#pragma warning(disable : 4305)
#pragma warning(disable : 4244)
#define _USE_MATH_DEFINES
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include <jansson.h>
#include "track.h"



image_palette_t mask_palette={128,-1,{
{  0,  0,  0},
{ 64,  0,  0},
{192,  0,  0},
{255,  0,  0},
{ 0,  64,  0},
{ 64, 64,  0},
{192, 64,  0},
{255, 64,  0},
{ 0, 192,  0},
{ 64,192,  0},
{192,192,  0},
{255,192,  0},
{ 0, 255,  0},
{ 64,255,  0},
{192,255,  0},
{255,255,  0},
{  0,  0, 64},
{ 64,  0, 64},
{192,  0, 64},
{255,  0, 64},
{ 0,  64, 64},
{ 64, 64, 64},
{192, 64, 64},
{255, 64, 64},
{ 0, 192, 64},
{ 64,192, 64},
{192,192, 64},
{255,192, 64},
{ 0, 255, 64},
{ 64,255, 64},
{192,255, 64},
{255,255, 64},
{  0,  0,192},
{ 64,  0,192},
{192,  0,192},
{255,  0,192},
{ 0,  64,192},
{ 64, 64,192},
{192, 64,192},
{255, 64,192},
{ 0, 192,192},
{ 64,192,192},
{192,192,192},
{255,192,192},
{ 0, 255,192},
{ 64,255,192},
{192,255,192},
{255,255,192},
{  0,  0,255},
{ 64,  0,255},
{192,  0,255},
{255,  0,255},
{ 0,  64,255},
{ 64, 64,255},
{192, 64,255},
{255, 64,255},
{ 0, 192,255},
{ 64,192,255},
{192,192,255},
{255,192,255},
{ 0, 255,255},
{ 64,255,255},
{192,255,255},
{255,255,255},
{  0,  0,  0},
{ 32,  0,  0},
{ 96,  0,  0},
{128,  0,  0},
{ 0,  32,  0},
{ 32, 32,  0},
{ 96, 32,  0},
{128, 32,  0},
{ 0,  96,  0},
{ 32, 96,  0},
{ 96, 96,  0},
{128, 96,  0},
{ 0, 128,  0},
{ 32,128,  0},
{ 96,128,  0},
{128,128,  0},
{  0,  0, 32},
{ 32,  0, 32},
{ 96,  0, 32},
{128,  0, 32},
{ 0,  32, 32},
{ 32, 32, 32},
{ 96, 32, 32},
{128, 32, 32},
{ 0,  96, 32},
{ 32, 96, 32},
{ 96, 96, 32},
{128, 96, 32},
{ 0, 128, 32},
{ 32,128, 32},
{ 96,128, 32},
{128,128, 32},
{  0,  0, 96},
{ 32,  0, 96},
{ 96,  0, 96},
{128,  0, 96},
{ 0,  32, 96},
{ 32, 32, 96},
{ 96, 32, 96},
{128, 32, 96},
{ 0,  96, 96},
{ 32, 96, 96},
{ 96, 96, 96},
{128, 96, 96},
{ 0, 128, 96},
{ 32,128, 96},
{ 96,128, 96},
{128,128, 96},
{  0,  0,128},
{ 32,  0,128},
{ 96,  0,128},
{128,  0,128},
{ 0,  32,128},
{ 32, 32,128},
{ 96, 32,128},
{128, 32,128},
{ 0,  96,128},
{ 32, 96,128},
{ 96, 96,128},
{128, 96,128},
{ 0, 128,128},
{ 32,128,128},
{ 96,128,128},
{128,128,128},
}};

void dump_mask(view_t* view,const char* filename)
{
//Get image bounds
rect_t bounds={0,0,1,1};
	for(int i=0;i<view->num_sprites;i++)
	for(int j=0;j<view->masks[i].num_rects;j++)
	{
	rect_t rect=view->masks[i].rects[j];
		if(rect.x_lower!=INT32_MIN&&rect.x_lower<bounds.x_lower)bounds.x_lower=rect.x_lower;
		if(rect.y_lower!=INT32_MIN&&rect.y_lower<bounds.y_lower)bounds.y_lower=rect.y_lower;
		if(rect.x_upper!=INT32_MAX&&rect.x_upper>bounds.x_upper)bounds.x_upper=rect.x_upper;
		if(rect.y_upper!=INT32_MAX&&rect.y_upper>bounds.y_upper)bounds.y_upper=rect.y_upper;
	}

bounds.x_lower-=32;
bounds.x_upper+=32;
bounds.y_lower-=32;
bounds.y_upper+=32;

int x_offset=-bounds.x_lower;
int y_offset=-bounds.y_lower;
int width=bounds.x_upper-bounds.x_lower;
int height=bounds.y_upper-bounds.y_lower;

image_t image;

image_new(&image,width,height,x_offset,y_offset,0);

int color_index=0;
int transfer=0;
	for(int i=0;i<view->num_sprites;i++)
	{
	printf("color_index %d %d\n",color_index,view->masks[i].track_mask_op);
		for(int j=0;j<view->masks[i].num_rects;j++)
		{
		rect_t rect=view->masks[i].rects[j];
		//Clip rect to image
			if(rect.x_lower<bounds.x_lower)rect.x_lower=bounds.x_lower;
			if(rect.y_lower<bounds.y_lower)rect.y_lower=bounds.y_lower;
			if(rect.x_upper>bounds.x_upper)rect.x_upper=bounds.x_upper;
			if(rect.y_upper>bounds.y_upper)rect.y_upper=bounds.y_upper;

			for(int x=rect.x_lower;x<rect.x_upper;x++)	
			for(int y=rect.y_lower;y<rect.y_upper;y++)
			{
			//0 - red set on secondary
			//1 - red set on primary
			//2 - green set on secondary
			//3 - green set on primary
			//4 - blue set on secondary
			//5 - blue set on primary
			//6 - origin point

			uint8_t colors[8]={0x1,0x4,0x10,0x5,0x14,0x11,0x15};
			uint8_t color=0;
				switch(view->masks[i].track_mask_op)
				{
				case TRACK_MASK_NONE:
				case TRACK_MASK_TRANSFER_NEXT:
					color=colors[color_index]|colors[color_index]<<1;
				break;
				case TRACK_MASK_INTERSECT:
				case TRACK_MASK_UNION:
					color=colors[color_index]<<1;
				break;
				case TRACK_MASK_DIFFERENCE:
					if(transfer)color=colors[color_index]|colors[color_index]<<1;
					else color=colors[color_index-1];
				break;
				}

			image.pixels[(x+x_offset)+(y+y_offset)*width]|=color;
			}

		image.pixels[x_offset+y_offset*width]|=0x40;
		}
		if(transfer||view->masks[i].track_mask_op!=TRACK_MASK_DIFFERENCE)color_index++;
		if(view->masks[i].track_mask_op==TRACK_MASK_TRANSFER_NEXT)transfer=1;
		else transfer=0;
	}	
	FILE* file=fopen(filename,"wb");
	if(file ==NULL)
	{
		printf("Error: could not open file for writing\n");
		exit(1);
	}
image_write_png(&image,&mask_palette,file);
}

void dump_masks()
{
json_t* json=json_object();
	for(int i=0;i<NUM_TRACK_SECTIONS;i++)
	{
	printf("Dumping %s\n",track_sections[i].name);
		if(strstr(track_sections[i].name,"barrel_roll_right"))break;
	json_t* track_section=json_array();
		for(int j=0;j<4;j++)
		{
			if(default_masks[i][j].num_sprites==0)break;
			if(default_masks[i][j].masks==NULL)continue;
		json_t* view=json_object();

		//Write mask
		
		char filename[256];
		rect_t rect=default_masks[i][j].masks[0].rects[0];
		int single_tile=default_masks[i][j].num_sprites==1||(default_masks[i][j].num_sprites==2&&default_masks[i][j].masks[1].track_mask_op==TRACK_MASK_DIFFERENCE);
		//Check for single tile non diagonal mask, which is frequently repeated and should not be duplicated
			if(single_tile&&rect.x_lower==INT32_MIN&&rect.y_lower==INT32_MIN&&rect.x_upper==INT32_MAX&&rect.y_upper==INT32_MAX)
			{
			sprintf(filename,"masks/images/single_tile.png");
			json_object_set_new(view,"mask",json_string(filename));
			}
		//Check for single tile diagonal mask
			else if(single_tile&&rect.x_lower==-32&&rect.y_lower==INT32_MIN&&rect.x_upper==32&&rect.y_upper==INT32_MAX)
			{
			sprintf(filename,"masks/images/single_tile_diag.png");
			json_object_set_new(view,"mask",json_string(filename));
			}
		//Check for element that is mirror of other piece - masks should not be duplicated
		//TODO doing this based on the name is a bit of hack - this assumes that mirrored elements are always 
                //named "left" and "right" and that the right piece always comes directly after the left
			else if(strstr(track_sections[i].name,"right"))
			{
			sprintf(filename,"masks/images/right.png");
				if(track_sections[i].flags&TRACK_DIAGONAL||track_sections[i].flags&TRACK_DIAGONAL_2)sprintf(filename,"masks/images/%s_%d.png",track_sections[i].name,(6-j)%4);
				else sprintf(filename,"masks/images/%s_%d.png",track_sections[i].name,3-j);
			json_object_set_new(view,"mirror",json_true());
			}
			else
			{
			sprintf(filename,"masks/images/%s_%d.png",track_sections[i].name,j);
			dump_mask(&(default_masks[i][j]),filename);
			}
		json_object_set_new(view,"mask",json_string(filename));

		//Write offsets if they are not all zero
		int any_nonzero_offset=0;
		json_t* offset=json_array();
			for(int k=0;k<default_masks[i][j].num_sprites;k++)
			{
				if(default_masks[i][j].masks[k].x_offset||default_masks[i][j].masks[k].y_offset)any_nonzero_offset=1;
			json_t* coord=json_array();
			json_array_append_new(coord,json_integer(default_masks[i][j].masks[k].x_offset));
			json_array_append_new(coord,json_integer(default_masks[i][j].masks[k].y_offset));
			json_array_append_new(offset,coord);
			}
			if(any_nonzero_offset)json_object_set_new(view,"offset",offset);
			else json_decref(offset);
	
		//Write split and/or transfer if not all false
		int any_split=0;
		int any_transfer=0;
		json_t* split=json_array();
		json_t* transfer=json_array();
			for(int k=0;k<default_masks[i][j].num_sprites;k++)
			{
				if(default_masks[i][j].masks[k].track_mask_op==TRACK_MASK_TRANSFER_NEXT)
				{
				any_transfer=1;
				json_array_append_new(split,json_false());
				json_array_append_new(transfer,json_true());
				}
				else if(k+1<default_masks[i][j].num_sprites&&default_masks[i][j].masks[k+1].track_mask_op==TRACK_MASK_DIFFERENCE)
				{
				any_split=1;
				k++;
				json_array_append_new(split,json_true());
				json_array_append_new(transfer,json_false());
				}
				else
				{
				json_array_append_new(split,json_false());
				json_array_append_new(transfer,json_false());
				}
			}
			if(any_split)json_object_set_new(view,"split",split);
			else json_decref(split);
			if(any_transfer)json_object_set_new(view,"transfer",transfer);
			else json_decref(transfer);

		json_array_append_new(track_section,view);
		}
	json_object_set_new(json,track_sections[i].name,track_section);	
	}

json_dump_file(json,"masks/default.json",JSON_INDENT(4)|JSON_COMPACT);
}



