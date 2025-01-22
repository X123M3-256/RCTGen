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
{192,  0,  0},
{  0,192,  0},
{  0,  0,192},
{192,192,  0},
{  0,192,192},
{192,  0,192},
{192,192,192},
{ 64,  0,  0},
{255,  0,  0},
{ 64,192,  0},
{ 64,  0,192},
{255,192,  0},
{ 64,192,192},
{255,  0,192},
{255,192,192},
{  0, 64,  0},
{192, 64,  0},
{  0,255,  0},
{  0, 64,192},
{192,255,  0},
{  0,255,192},
{192, 64,192},
{192,255,192},
{  0,  0, 64},
{192,  0, 64},
{  0,192, 64},
{  0,  0,255},
{192,192, 64},
{  0,192,255},
{192,  0,255},
{192,192,255},
{ 64, 64,  0},
{255, 64,  0},
{ 64,255,  0},
{ 64, 64,192},
{255,255,  0},
{ 64,255,192},
{255, 64,192},
{255,255,192},
{  0, 64, 64},
{192, 64, 64},
{  0,255, 64},
{  0, 64,255},
{192,255, 64},
{  0,255,255},
{192, 64,255},
{192,255,255},
{ 64,  0, 64},
{255,  0, 64},
{ 64,192, 64},
{ 64,  0,255},
{255,192, 64},
{ 64,192,255},
{255,  0,255},
{255,192,255},
{ 64, 64, 64},
{255, 64, 64},
{ 64,255, 64},
{ 64, 64,255},
{255,255, 64},
{ 64,255,255},
{255, 64,255},
{255,255,255},
{  0,  0,  0},
{ 96,  0,  0},
{  0, 96,  0},
{  0,  0, 96},
{ 96, 96,  0},
{  0, 96, 96},
{ 96,  0, 96},
{ 96, 96, 96},
{ 32,  0,  0},
{128,  0,  0},
{ 32, 96,  0},
{ 32,  0, 96},
{128, 96,  0},
{ 32, 96, 96},
{128,  0, 96},
{128, 96, 96},
{  0, 32,  0},
{ 96, 32,  0},
{  0,128,  0},
{  0, 32, 96},
{ 96,128,  0},
{  0,128, 96},
{ 96, 32, 96},
{ 96,128, 96},
{  0,  0, 32},
{ 96,  0, 32},
{  0, 96, 32},
{  0,  0,128},
{ 96, 96, 32},
{  0, 96,128},
{ 96,  0,128},
{ 96, 96,128},
{ 32, 32,  0},
{128, 32,  0},
{ 32,128,  0},
{ 32, 32, 96},
{128,128,  0},
{ 32,128, 96},
{128, 32, 96},
{128,128, 96},
{  0, 32, 32},
{ 96, 32, 32},
{  0,128, 32},
{  0, 32,128},
{ 96,128, 32},
{  0,128,128},
{ 96, 32,128},
{ 96,128,128},
{ 32,  0, 32},
{128,  0, 32},
{ 32, 96, 32},
{ 32,  0,128},
{128, 96, 32},
{ 32, 96,128},
{128,  0,128},
{128, 96,128},
{ 32, 32, 32},
{128, 32, 32},
{ 32,128, 32},
{ 32, 32,128},
{128,128, 32},
{ 32,128,128},
{128, 32,128},
{128,128,128},
}};

#define MAX_RECTS 1000000
#define MAX_MASKS 1000

typedef struct
{
rect_t* rects;
mask_t* masks;
view_t views[NUM_TRACK_SECTIONS][4];
}mask_list_t;



char mask_index(image_t* mask,int x, int y,int s)
{
	if(!s)return mask->pixels[x+y*mask->width]&0x7;
	else return (mask->pixels[x+y*mask->width]&0x38)>>3;
}

void add_rect(image_t* mask,rect_t rect,rect_t* rects,int* num_rects)
{
	if(rect.x_lower==0)rect.x_lower=INT32_MIN;
	else rect.x_lower+=mask->x_offset;
	if(rect.y_lower==0)rect.y_lower=INT32_MIN;
	else rect.y_lower+=mask->y_offset;
	if(rect.x_upper==mask->width)rect.x_upper=INT32_MAX;
	else rect.x_upper+=mask->x_offset;
	if(rect.y_upper==mask->height)rect.y_upper=INT32_MAX;
	else rect.y_upper+=mask->y_offset;

	for(int i=0;i<*num_rects;i++)
	{
		if(rects[i].x_lower==rect.x_lower&&rects[i].x_upper==rect.x_upper&&rects[i].y_upper==rect.y_lower)
		{
		rects[i].y_upper=rect.y_upper;
		return;
		}
	}
rects[*num_rects]=rect;
(*num_rects)++;
}

void process_slice(image_t* mask,int y,char sprite,rect_t* rects,int* num_rects,int s)
{
int start_x=-1;
	for(int x=0;x<mask->width;x++)
	{
		if(mask_index(mask,x,y,s)==sprite&&start_x==-1)start_x=x;

		if(mask_index(mask,x,y,s)!=sprite&&start_x>=0)
		{
		rect_t rect={start_x,y,x,y+1};
		add_rect(mask,rect,rects,num_rects);		
		start_x=-1;
		}

		if(x==mask->width-1&&start_x>=0)
		{
		rect_t rect={start_x,y,x+1,y+1};
		add_rect(mask,rect,rects,num_rects);	
		}
	}
}

void mask_list_add_rects(mask_list_t* mask_list,int* num_rects_total,rect_t* rects,int num_rects)
{
//TODO bounds check
memcpy(mask_list->rects+(*num_rects_total),rects,num_rects*sizeof(rect_t));
*num_rects_total+=num_rects;
}

void mask_list_add_mask(mask_list_t* mask_list,int* num_masks_total,mask_t* mask)
{
//TODO bounds check
mask_list->masks[*num_masks_total]=*mask;
(*num_masks_total)++;;
}


int process_mask(image_t* mask,int mirror,int split[8],int transfer[8],int x_offset[8],int y_offset[8],view_t* view,mask_list_t* mask_list,int* num_masks_total,int* num_rects_total)
{
int rect_counts[8]={0,0,0,0,0,0,0,0};
int secondary_rect_counts[8]={0,0,0,0,0,0,0,0};
rect_t* rects=(rect_t*)malloc(sizeof(rect_t)*(mask->width*mask->height+1)/2);
rect_t* secondary_rects=(rect_t*)malloc(sizeof(rect_t)*(mask->width*mask->height+1)/2);

//Find origin point and sprite count
int offset_found=0;
int num_sprites=0;
	for(int x=0;x<mask->width;x++)
	for(int y=0;y<mask->height;y++)
	{
	//Check for origin point
		if(mask->pixels[x+y*mask->width]&0x40)
		{
			if(offset_found)
			{
			printf("Error: Multiple origin points found in mask\n");
			return 1;
			}
		mask->x_offset=-x;
		mask->y_offset=-y;
		offset_found=1;
		}
	//Count number of sprites
		if(mask_index(mask,x,y,0)>num_sprites)num_sprites=mask_index(mask,x,y,0);
		if(mask_index(mask,x,y,1)>num_sprites)num_sprites=mask_index(mask,x,y,1);
	}
	if(!offset_found)
	{
	printf("Error: No origin point found in mask\n");
	return 1;
	}

int offset=*num_rects_total;
int mask_start=*num_masks_total;
	for(int sprite=0;sprite<num_sprites;sprite++)
	{
	//Generate list of rects from mask
	int num_rects=0;
	int secondary_num_rects=0;
		for(int y=0;y<mask->height;y++)
		{
		process_slice(mask,y,sprite+1,rects,&num_rects,0);
		process_slice(mask,y,sprite+1,secondary_rects,&secondary_num_rects,1);
		}
	//Check if secondary rects are the same as the first
	int secondary_identical=1;
		for(int i=0;i<num_rects;i++)
		{
			if(rects[i].x_lower!=secondary_rects[i].x_lower||rects[i].x_upper!=secondary_rects[i].x_upper||rects[i].y_lower!=secondary_rects[i].y_lower||rects[i].y_upper!=secondary_rects[i].y_upper)
			{
			secondary_identical=0;
			break;
			}
		}

	//Add rects to mask set
	mask_list_add_rects(mask_list,num_rects_total,rects,num_rects);
		if(!secondary_identical)mask_list_add_rects(mask_list,num_rects_total,secondary_rects,secondary_num_rects);

	//Add masks to mask set
		mask_t mask;
		mask.x_offset=x_offset[sprite];
		mask.y_offset=y_offset[sprite];
		mask.rects=mask_list->rects+offset;
		mask.num_rects=secondary_identical?num_rects:num_rects+secondary_num_rects;

		if(!transfer[sprite])
		{
			if(sprite>0&&transfer[sprite-1])
			{
				if(split[sprite])
				{
				printf("Error: Cannot use transfer and split simultaneously\n");
				return 1;
				}
			mask.track_mask_op=TRACK_MASK_DIFFERENCE;
			mask_list_add_mask(mask_list,num_masks_total,&mask);
			}
			else if(split[sprite])
			{
			mask.track_mask_op=TRACK_MASK_INTERSECT;
			mask_list_add_mask(mask_list,num_masks_total,&mask);

			mask.track_mask_op=TRACK_MASK_DIFFERENCE;
				if(!secondary_identical)
				{
				mask.rects=mask_list->rects+offset+num_rects;
				offset+=secondary_num_rects;
				}
			mask_list_add_mask(mask_list,num_masks_total,&mask);
			}
			else
			{
			mask.track_mask_op=TRACK_MASK_NONE;
			mask_list_add_mask(mask_list,num_masks_total,&mask);
			}
		}
		else
		{
			if(split[sprite])
			{
			printf("Error: Cannot use transfer and split simultaneously\n");
			return 1;
			}
		mask.track_mask_op=TRACK_MASK_DIFFERENCE;
		mask_list_add_mask(mask_list,num_masks_total,&mask);
		}
	offset+=num_rects;
	}

//Populate view
view->flags=VIEW_NEEDS_TRACK_MASK;
view->masks=mask_list->masks+mask_start;
view->num_sprites=*num_masks_total-mask_start;

putchar('\n');

free(rects);
free(secondary_rects);
return 0;
}

void print_value(int32_t value)
{
	if(value==INT32_MIN)printf("INT32_MIN");
	else if(value==INT32_MAX)printf("INT32_MAX");
	else printf("%d",value);
}

void print_masks(mask_list_t* mask_list,int i)
{
rect_t* start_rect=mask_list->views[i][0].masks[0].rects;//TODO this might be null

printf("rect_t %s_rects[]={\n",track_sections[i].name);
	for(int j=0;j<4;j++)
	{
	printf("//Angle %d\n",j);
		for(int k=0;k<mask_list->views[i][j].num_sprites;k++)
		{
			for(int l=0;l<mask_list->views[i][j].masks[k].num_rects;l++)
			{
			rect_t rect=mask_list->views[i][j].masks[k].rects[l];
			putchar('{');
			print_value(rect.x_lower);
			putchar(',');
			print_value(rect.y_lower);
			putchar(',');
			print_value(rect.x_upper);
			putchar(',');
			print_value(rect.y_upper);
			putchar('}');
			putchar(',');
			}
		putchar('\n');
		}
	}
printf("};\n");



printf("mask_t %s_masks[]={\n",track_sections[i].name);
	for(int j=0;j<4;j++)
	{
		for(int k=0;k<mask_list->views[i][j].num_sprites;k++)
		{
		mask_t* mask=mask_list->views[i][j].masks+k;
		const char* track_ops[4]={"TRACK_MASK_NONE","TRACK_MASK_DIFFERENCE","TRACK_MASK_INTERSECT","TRACK_MASK_TRANSFER_NEXT"};
		printf("{%s,%d,%d,%d,%s_rects+%d},",track_ops[mask->track_mask_op],mask->num_rects,mask->x_offset,mask->y_offset,track_sections[i].name,(mask->rects-start_rect)/sizeof(rect_t));
		}
	putchar('\n');
	}
printf("};\n");
}

int load_masks(const char* filename)
{
json_error_t error;
json_t* json=json_load_file(filename,0,&error);
	if(json==NULL)
	{
	printf("Error: %s at line %d column %d\n",error.text,error.line,error.column);
	return 1;
	}
	
	if(!json_is_object(json))
	{
	printf("Error: Top level must be object\n");
	return 1;
	}

int i=111;

printf("Loading %s\n",track_sections[i].name);

json_t* track_section=json_object_get(json,track_sections[i].name);
	if(!track_section||!json_is_array(track_section))
	{
	printf("Error: Property %s does not exist or is not an array\n",track_sections[i].name);
	return 1;
	}

int num_angles=json_array_size(track_section);


int num_rects=0;
int num_masks=0;
mask_list_t mask_list;
mask_list.rects=(rect_t*)calloc(MAX_RECTS,sizeof(rect_t));
mask_list.masks=(mask_t*)calloc(MAX_MASKS,sizeof(mask_t));
memset(mask_list.views,0,NUM_TRACK_SECTIONS*4*sizeof(view_t));

	for(int j=0;j<num_angles;j++)
	{
	json_t* item=json_array_get(track_section,j);
		if(!json_is_object(item))
		{
		printf("Error: Array element is not an object\n");
		return 1;
		}
	
	//Load mask
	json_t* mask=json_object_get(item,"mask");
		if(!mask||!json_is_string(mask))
		{
		printf("Error: \"mask\" not found or is not a string\n");
		return 1;
		}
	printf("Mask %s\n",json_string_value(mask));
	FILE* file=fopen(json_string_value(mask),"rb");
		if(file ==NULL)
		{
		printf("Error: could not open %s for writing\n",json_string_value(mask));
		return 1;
		}

	image_t image;
		if(image_read_png(&image,file))
		{
		printf("Error: failed loading %s\n",json_string_value(mask));
		return 1;
		}

	//Check if mirrored
	json_t* mirror_json=json_object_get(item,"mirror");
		if(mirror_json&&!json_is_boolean(mirror_json))
		{
		printf("Error: \"mirror\" is not a boolean\n");
		return 1;
		}
	int mirror=json_boolean_value(mirror_json);

	//Load split
	int split[8]={0,0,0,0,0,0,0,0};
	json_t* split_json=json_object_get(item,"split");
		if(split_json)
		{
			if(!json_is_array(split_json))
			{
			printf("Error: \"split\" is not an array\n");
			return 1;
			}
			for(int k=0;k<json_array_size(split_json)&&k<8;k++)
			{
			json_t* split_item=json_array_get(split_json,k);
				if(!json_is_boolean(split_item))
				{
				printf("Error: \"split\" contains non boolean value\n");
				return 1;
				}
			split[k]=json_boolean_value(split_item);
			}
		}

	//Load transfer
	int transfer[8]={0,0,0,0,0,0,0,0};
	json_t* transfer_json=json_object_get(item,"transfer");
		if(transfer_json)
		{
			if(!json_is_array(transfer_json))
			{
			printf("Error: \"transfer\" is not an array\n");
			return 1;
			}
			for(int k=0;k<json_array_size(transfer_json)&&k<8;k++)
			{
			json_t* transfer_item=json_array_get(transfer_json,k);
				if(!json_is_boolean(transfer_item))
				{
				printf("Error: \"transfer\" contains non boolean value\n");
				return 1;
				}
			transfer[k]=json_boolean_value(transfer_item);
			}
		}
	
	//Load offset
	int x_offset[8]={0,0,0,0,0,0,0,0};
	int y_offset[8]={0,0,0,0,0,0,0,0};
	json_t* offset_json=json_object_get(item,"offset");
		if(offset_json)
		{
			if(!json_is_array(offset_json))
			{
			printf("Error: \"offset\" is not an array\n");
			return 1;
			}
			for(int k=0;k<json_array_size(offset_json)&&k<8;k++)
			{
			json_t* offset_item=json_array_get(offset_json,k);
				if(!json_is_array(offset_item)||json_array_size(offset_item)!=2)
				{
				printf("Error: \"offset\" contains an element which not a pair of integers\n");
				return 1;
				}
			json_t* x_elem=json_array_get(offset_item,0);
			json_t* y_elem=json_array_get(offset_item,1);
				if(!json_is_integer(x_elem)||!json_is_integer(y_elem))
				{
				printf("Error: \"offset\" contains an element which not a pair of integers\n");
				return 1;
				}
			x_offset[k]=json_integer_value(x_elem);
			y_offset[k]=json_integer_value(y_elem);
			}
		}

	process_mask(&image,mirror,split,transfer,x_offset,y_offset,&(mask_list.views[i][j]),&mask_list,&num_masks,&num_rects);
	}


print_masks(&mask_list,i);

return 0;
}






void dump_mask(const view_t* view,const char* filename)
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

int color_index=1;
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

			uint8_t color=0;
				switch(view->masks[i].track_mask_op)
				{
				case TRACK_MASK_NONE:
				case TRACK_MASK_TRANSFER_NEXT:
					color=color_index|(color_index<<3);
				break;
				case TRACK_MASK_INTERSECT:
					color=color_index;
				break;
				case TRACK_MASK_DIFFERENCE:
					if(transfer)color=color_index|(color_index<<3);
					else color=(color_index-1)<<3;
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

void dump_masks(const char* set_name,const view_t masks[NUM_TRACK_SECTIONS][4])
{
load_masks("masks/default.json");
return;

		
char filename[256];
json_t* json=json_object();
	for(int i=0;i<NUM_TRACK_SECTIONS;i++)
	{
	printf("Dumping %s\n",track_sections[i].name);

	json_t* track_section=json_array();
		for(int j=0;j<4;j++)
		{
			if(masks[i][j].num_sprites==0)break;
			if(masks[i][j].masks==NULL)continue;
		json_t* view=json_object();

		//Write mask
		rect_t rect=masks[i][j].masks[0].rects[0];
		int single_tile=masks[i][j].num_sprites==1||(masks[i][j].num_sprites==2&&masks[i][j].masks[1].track_mask_op==TRACK_MASK_DIFFERENCE);
		//Check for single tile non diagonal mask, which is frequently repeated and should not be duplicated
			if(single_tile&&rect.x_lower==INT32_MIN&&rect.y_lower==INT32_MIN&&rect.x_upper==INT32_MAX&&rect.y_upper==INT32_MAX)
			{
			sprintf(filename,"masks/%s/single_tile.png",set_name);
			json_object_set_new(view,"mask",json_string(filename));
			}
		//Check for single tile diagonal mask
			else if(single_tile&&rect.x_lower==-32&&rect.y_lower==INT32_MIN&&rect.x_upper==32&&rect.y_upper==INT32_MAX)
			{
			sprintf(filename,"masks/%s/single_tile_diag.png",set_name);
			json_object_set_new(view,"mask",json_string(filename));
			}
		//Check for element that is mirror of other piece - masks should not be duplicated
		//TODO doing this based on the name is a bit of hack - this assumes that mirrored elements are always 
                //named "left" and "right" and that the right piece always comes directly after the left
			else if(strstr(track_sections[i].name,"right"))
			{
				if(track_sections[i].flags&TRACK_DIAGONAL||track_sections[i].flags&TRACK_DIAGONAL_2)sprintf(filename,"masks/%s/%s_%d.png",set_name,track_sections[i].name,(6-j)%4);
				else sprintf(filename,"masks/%s/%s_%d.png",set_name,track_sections[i].name,3-j);
			json_object_set_new(view,"mirror",json_true());
			}
			else
			{
			sprintf(filename,"masks/%s/%s_%d.png",set_name,track_sections[i].name,j);
			dump_mask(&(masks[i][j]),filename);
			}
		json_object_set_new(view,"mask",json_string(filename));

		//Write split and/or transfer if not all false
		int any_split=0;
		int any_transfer=0;
		json_t* split=json_array();
		json_t* transfer=json_array();
			for(int k=0;k<masks[i][j].num_sprites;k++)
			{
				if(masks[i][j].masks[k].track_mask_op==TRACK_MASK_TRANSFER_NEXT)
				{
				any_transfer=1;
				json_array_append_new(split,json_false());
				json_array_append_new(transfer,json_true());
				}
				else if(k+1<masks[i][j].num_sprites&&masks[i][j].masks[k+1].track_mask_op==TRACK_MASK_DIFFERENCE)
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

		//Write offsets if they are not all zero
		int any_nonzero_offset=0;
		json_t* offset=json_array();
			for(int k=0;k<masks[i][j].num_sprites;k++)
			{
				if(masks[i][j].masks[k].x_offset||masks[i][j].masks[k].y_offset)any_nonzero_offset=1;

				if(masks[i][j].masks[k].track_mask_op==TRACK_MASK_DIFFERENCE&&(k==0||masks[i][j].masks[k-1].track_mask_op!=TRACK_MASK_TRANSFER_NEXT))continue;

			json_t* coord=json_array();
			json_array_append_new(coord,json_integer(masks[i][j].masks[k].x_offset));
			json_array_append_new(coord,json_integer(masks[i][j].masks[k].y_offset));
			json_array_append_new(offset,coord);
			}
			if(any_nonzero_offset)json_object_set_new(view,"offset",offset);
			else json_decref(offset);
	

		json_array_append_new(track_section,view);
		}
	json_object_set_new(json,track_sections[i].name,track_section);	
	}

sprintf(filename,"masks/%s.json",set_name);
json_dump_file(json,filename,JSON_INDENT(4)|JSON_COMPACT);
}



