#ifdef _MSC_VER
#pragma warning(disable : 4305)
#pragma warning(disable : 4244)
#define _USE_MATH_DEFINES
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
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

#define MAX_RECTS 500000
#define MAX_MASKS 2040

typedef struct
{
rect_t* rects;
mask_t* masks;
view_t (*views)[4];
}mask_list_t;

char mask_index(image_t* mask,int x, int y,int s)
{
	if(!s)return mask->pixels[x+y*mask->width]&0x7;
	else return (mask->pixels[x+y*mask->width]&0x38)>>3;
}

void mask_list_add_rects(mask_list_t* mask_list,int* num_rects_total,rect_t* rects,int num_rects)
{
assert(*num_rects_total<MAX_RECTS);
memcpy(mask_list->rects+(*num_rects_total),rects,num_rects*sizeof(rect_t));
*num_rects_total+=num_rects;
}

void mask_list_add_mask(mask_list_t* mask_list,int* num_masks_total,mask_t* mask)
{
assert(*num_masks_total<MAX_MASKS);
mask_list->masks[*num_masks_total]=*mask;
(*num_masks_total)++;;
}

void add_rect(image_t* mask,int mirror,rect_t rect,rect_t* rects,int* num_rects)
{
	if(mirror)
	{
	int temp=rect.x_upper;
		if(rect.x_lower==0)rect.x_upper=INT32_MAX;
		else rect.x_upper=-(rect.x_lower+mask->x_offset);
		if(temp==mask->width)rect.x_lower=INT32_MIN;
		else rect.x_lower=-(temp+mask->x_offset);
	}
	else
	{
		if(rect.x_lower==0)rect.x_lower=INT32_MIN;
		else rect.x_lower+=mask->x_offset;
		if(rect.x_upper==mask->width)rect.x_upper=INT32_MAX;
		else rect.x_upper+=mask->x_offset;
	}
	if(rect.y_lower==0)rect.y_lower=INT32_MIN;
	else rect.y_lower+=mask->y_offset;
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

void process_slice(image_t* mask,int mirror,int y,char sprite,rect_t* rects,int* num_rects,int s)
{
int start_x=-1;
	for(int x=0;x<mask->width;x++)
	{
		if(mask_index(mask,x,y,s)==sprite&&start_x==-1)start_x=x;

		if(mask_index(mask,x,y,s)!=sprite&&start_x>=0)
		{
		rect_t rect={start_x,y,x,y+1};
		add_rect(mask,mirror,rect,rects,num_rects);		
		start_x=-1;
		}

		if(x==mask->width-1&&start_x>=0)
		{
		rect_t rect={start_x,y,x+1,y+1};
		add_rect(mask,mirror,rect,rects,num_rects);	
		}
	}
}

int process_mask(image_t* mask,int mirror,int split[8],int transfer[8],int split_ends,int x_offset[8],int y_offset[8],int num_sprites,view_t* view,mask_list_t* mask_list,int* num_masks_total,int* num_rects_total)
{
int rect_counts[8]={0,0,0,0,0,0,0,0};
int secondary_rect_counts[8]={0,0,0,0,0,0,0,0};
rect_t* rects=(rect_t*)malloc(sizeof(rect_t)*(mask->width*mask->height+1)/2);
rect_t* secondary_rects=(rect_t*)malloc(sizeof(rect_t)*(mask->width*mask->height+1)/2);

//Find origin point and sprite count
int offset_found=0;
int nontrivial=0;
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
	//Check if this is the trivial mask
		if(mask_index(mask,x,y,0)!=1||mask_index(mask,x,y,1)!=1)nontrivial=1;
	}
	if(!offset_found)
	{
	printf("Error: No origin point found in mask\n");
	return 1;
	}

	if(!nontrivial&&!split[0]&&num_sprites==1)
	{
	view->masks=NULL;
	view->num_sprites=num_sprites;
	view->flags=0;
	return 0;
	}

int offset=*num_rects_total;
int mask_start=*num_masks_total;
	for(int sprite=0;sprite<num_sprites;sprite++)
	{
		//If split_ends set then last mask uses same rect as first
		if(split_ends&&sprite==num_sprites-1)
		{
			if(split[num_sprites-1])
			{
			printf("Error: Cannot use split and split_ends simultaneously\n");
			return 1;
			}
		//TODO handle case where secondary_identical is false
		mask_t mask;
		mask.x_offset=x_offset[sprite];
		mask.y_offset=y_offset[sprite];
		mask.rects=mask_list->masks[mask_start].rects;
		mask.num_rects=mask_list->masks[mask_start].num_rects;
		mask.track_mask_op=TRACK_MASK_DIFFERENCE;
		mask_list_add_mask(mask_list,num_masks_total,&mask);
		break;
		}


	//Generate list of rects from mask
	int num_rects=0;
	int secondary_num_rects=0;
		for(int y=0;y<mask->height;y++)
		{
		process_slice(mask,mirror,y,sprite+1,rects,&num_rects,0);
		process_slice(mask,mirror,y,sprite+1,secondary_rects,&secondary_num_rects,1);
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

	//Add masks to mask set
		mask_t mask;
		mask.x_offset=x_offset[sprite];
		mask.y_offset=y_offset[sprite];
		mask.rects=mask_list->rects+offset;
		mask.num_rects=num_rects;

		if(!transfer[sprite])
		{
			if(split_ends&&sprite==0)
			{
				if(split[0])
				{
				printf("Error: Cannot use split and split_ends simultaneously\n");
				return 1;
				}
			mask.track_mask_op=TRACK_MASK_INTERSECT;
			mask_list_add_mask(mask_list,num_masks_total,&mask);
			}
			else if(sprite>0&&transfer[sprite-1])
			{
				if(split[sprite])
				{
				printf("Error: Cannot use transfer and split simultaneously\n");
				return 1;
				}
			//TODO handle case where secondary_identical is false
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
				mask_list_add_rects(mask_list,num_rects_total,secondary_rects,secondary_num_rects);
				mask.num_rects=secondary_num_rects;
				mask.rects=mask_list->rects+offset+num_rects;
				offset+=secondary_num_rects;
				}
			mask_list_add_mask(mask_list,num_masks_total,&mask);
			}
			else
			{
			//TODO handle case where secondary_identical is false
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
			if(sprite+1<num_sprites&&transfer[sprite+1])
			{
			printf("Error: Cannot use transfer on consecutive sprites\n");
			return 1;
			}
		//TODO handle case where secondary_identical is false
		mask.track_mask_op=TRACK_MASK_TRANSFER_NEXT;
		mask_list_add_mask(mask_list,num_masks_total,&mask);
		}
	offset+=num_rects;
	}

//Populate view
view->masks=mask_list->masks+mask_start;
view->num_sprites=*num_masks_total-mask_start;
view->flags=0;
	for(int i=0;i<view->num_sprites;i++)
	{
		if(view->masks[i].track_mask_op!=TRACK_MASK_NONE)
		{
		view->flags|=VIEW_NEEDS_TRACK_MASK;
		break;
		}
	}

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

void print_masks(view_t views[NUM_TRACK_SECTIONS][4])
{
	for(int i=0;i<NUM_TRACK_SECTIONS;i++)
	{
	//Find first rect for this view
	rect_t* start_rect=NULL;
		for(int j=0;j<4;j++)
		{
			if(views[i][j].masks)
			{
			start_rect=views[i][j].masks[0].rects;
			break;
			}
		}

	//If no rects found then this view has no masks to write	
		if(!start_rect)continue;
	

	printf("rect_t %s_rects[]={\n",track_sections[i].name);
		for(int j=0;j<4;j++)
		{
			if(!views[i][j].masks)continue;
		printf("\t//Angle %d\n",j);
			for(int k=0;k<views[i][j].num_sprites;k++)
			{
				if(k>0&&(views[i][j].masks[k].track_mask_op==TRACK_MASK_DIFFERENCE||views[i][j].masks[k-1].track_mask_op!=TRACK_MASK_TRANSFER_NEXT)&&views[i][j].masks[k].rects==views[i][j].masks[k-1].rects)continue;
			putchar('\t');
				for(int l=0;l<views[i][j].masks[k].num_rects;l++)
				{
				rect_t rect=views[i][j].masks[k].rects[l];
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
		putchar('\t');
			if(!views[i][j].masks)continue;
			for(int k=0;k<views[i][j].num_sprites;k++)
			{
			mask_t* mask=views[i][j].masks+k;
			const char* track_ops[4]={"TRACK_MASK_NONE","TRACK_MASK_DIFFERENCE","TRACK_MASK_INTERSECT","TRACK_MASK_TRANSFER_NEXT"};
			printf("{%s,%d,%d,%d,%s_rects+%d},",track_ops[mask->track_mask_op],mask->num_rects,mask->x_offset,mask->y_offset,track_sections[i].name,mask->rects-start_rect);
			}
		putchar('\n');
		}
	printf("};\n");
	}

printf("view_t views[TRACK_SECTIONS][4]={\n");
	for(int i=0;i<NUM_TRACK_SECTIONS;i++)
	{
	mask_t* start_mask=NULL;
	putchar('\t');
		for(int j=0;j<4;j++)
		{
			if(!views[i][j].masks)printf("{0,%d,NULL},",views[i][j].num_sprites);
			else
			{
				if(start_mask==NULL)start_mask=views[i][j].masks;
			const char* flags[4]={"0","VIEW_NEEDS_TRACK_MASK"};
			printf("{%s,%d,%s_masks+%d},",flags[views[i][j].flags],views[i][j].num_sprites,track_sections[i].name,views[i][j].masks-start_mask);
			}
		}
	printf(" //%s\n",track_sections[i].name);
	}
printf("};\n");
}

int load_masks(const char* filename,view_t views[NUM_TRACK_SECTIONS][4])
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


int num_rects=0;
int num_masks=0;
mask_list_t mask_list;
mask_list.rects=(rect_t*)calloc(MAX_RECTS,sizeof(rect_t));
mask_list.masks=(mask_t*)calloc(MAX_MASKS,sizeof(mask_t));
mask_list.views=views;
memset(mask_list.views,0,NUM_TRACK_SECTIONS*4*sizeof(view_t));

	for(int i=0;i<NUM_TRACK_SECTIONS;i++)
	{
	printf("Loading %s\n",track_sections[i].name);

	json_t* track_section=json_object_get(json,track_sections[i].name);
		if(!track_section)continue;
		if(!json_is_array(track_section))
		{
		printf("Error: Property %s does not exist or is not an array\n",track_sections[i].name);
		return 1;
		}

	int num_angles=json_array_size(track_section);
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
			fclose(file);
			return 1;
			}
		fclose(file);

		//Check if mirrored
		json_t* mirror_json=json_object_get(item,"mirror");
			if(mirror_json&&!json_is_boolean(mirror_json))
			{
			printf("Error: \"mirror\" is not a boolean\n");
			return 1;
			}
		int mirror=json_boolean_value(mirror_json);


		//Check for split_ends
		json_t* split_ends_json=json_object_get(item,"split_ends");
			if(split_ends_json&&!json_is_boolean(split_ends_json))
			{
			printf("Error: \"split_ends\" is not a boolean\n");
			return 1;
			}
		int split_ends=json_boolean_value(split_ends_json);

		//Load split
		int num_sprites=0;
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
			num_sprites=json_array_size(split_json);
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
				if(num_sprites<json_array_size(transfer_json))num_sprites=json_array_size(transfer_json);
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
				if(num_sprites<json_array_size(offset_json))num_sprites=json_array_size(offset_json);
			}

			if(process_mask(&image,mirror,split,transfer,split_ends,x_offset,y_offset,num_sprites,&(mask_list.views[i][j]),&mask_list,&num_masks,&num_rects))return 1;
		}
	}

return 0;
}

rect_t single_tile_rect={INT32_MIN,INT32_MIN,INT32_MAX,INT32_MAX};
mask_t single_tile_mask={0,1,0,0,&single_tile_rect};
view_t single_tile={0,1,&single_tile_mask};

rect_t single_tile_diag_rect={-32,INT32_MIN,32,INT32_MAX};
mask_t single_tile_diag_mask={0,1,0,0,&single_tile_diag_rect};
view_t single_tile_diag={0,1,&single_tile_diag_mask};

int dump_mask(const view_t* view,const char* filename)
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

int num_sprites=0;
int color_index=1;
int transfer=0;
	for(int i=0;i<view->num_sprites;i++)
	{
		for(int j=0;j<view->masks[i].num_rects;j++)
		{
		rect_t rect=view->masks[i].rects[j];
		for(int j=0;j<view->masks[i].num_rects;j++)
		//Clip rect to image
			if(rect.x_lower<bounds.x_lower)rect.x_lower=bounds.x_lower;
			if(rect.y_lower<bounds.y_lower)rect.y_lower=bounds.y_lower;
			if(rect.x_upper>bounds.x_upper)rect.x_upper=bounds.x_upper;
			if(rect.y_upper>bounds.y_upper)rect.y_upper=bounds.y_upper;

			for(int x=rect.x_lower;x<rect.x_upper;x++)	
			for(int y=rect.y_lower;y<rect.y_upper;y++)
			{
			if(num_sprites<color_index)num_sprites=color_index;

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
					//Check for split ends
					else if(i>0&&view->masks[i-1].track_mask_op!=TRACK_MASK_INTERSECT)color=1<<3;
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
	fclose(file);
	exit(1);
	}
image_write_png(&image,&mask_palette,file);
fclose(file);
return num_sprites;
}

void dump_masks(const char* set_name,const view_t masks[NUM_TRACK_SECTIONS][4])
{
//Write single tile masks TODO ideally these would be written only if they are used
char filename[256];
sprintf(filename,"masks/%s/single_tile.png",set_name);
dump_mask(&single_tile,filename);
sprintf(filename,"masks/%s/single_tile_diag.png",set_name);
dump_mask(&single_tile_diag,filename);
		
json_t* json=json_object();
	for(int i=0;i<NUM_TRACK_SECTIONS;i++)
	{
	printf("Dumping %s\n",track_sections[i].name);

	json_t* track_section=json_array();
		for(int j=0;j<4;j++)
		{
			if(masks[i][j].num_sprites==0)break;
		json_t* view=json_object();
			if(masks[i][j].masks==NULL)
			{
			sprintf(filename,"masks/%s/single_tile.png",set_name);
			json_object_set_new(view,"mask",json_string(filename));
			json_array_append_new(track_section,view);
			continue;
			}
		//Write mask
		rect_t rect=masks[i][j].masks[0].rects[0];
		int mask_num_sprites=1;
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
			else if(strstr(track_sections[i].name,"right")&&i>0)
			{
			int new_index=3-j;
				if(track_sections[i].flags&TRACK_DIAGONAL||track_sections[i].flags&TRACK_DIAGONAL_2)new_index=(6-j)%4;
			//Check if track piece only has two angles
				if(masks[i][2].num_sprites==0)new_index=new_index%2;
			sprintf(filename,"masks/%s/%s_%d.png",set_name,track_sections[i-1].name,new_index);
			json_object_set_new(view,"mirror",json_true());
			}
			else
			{
			sprintf(filename,"masks/%s/%s_%d.png",set_name,track_sections[i].name,j);
			mask_num_sprites=dump_mask(&(masks[i][j]),filename);
			}
		json_object_set_new(view,"mask",json_string(filename));

		//Check for split_ends
		int split_ends=0;
			if(masks[i][j].masks!=NULL&&masks[i][j].num_sprites>1&&masks[i][j].masks[0].track_mask_op==TRACK_MASK_INTERSECT&&masks[i][j].masks[1].track_mask_op!=TRACK_MASK_DIFFERENCE)
			{
			split_ends=1;
			json_object_set_new(view,"split_ends",json_true());
			}

		//Write offsets if they are not all zero
		int any_nonzero_offset=0;
		json_t* offset=json_array();
			for(int k=0;k<masks[i][j].num_sprites;k++)
			{
				if(masks[i][j].masks[k].x_offset||masks[i][j].masks[k].y_offset)any_nonzero_offset=1;

				if(masks[i][j].masks[k].track_mask_op==TRACK_MASK_DIFFERENCE&&(k==0||masks[i][j].masks[k-1].track_mask_op!=TRACK_MASK_TRANSFER_NEXT)&&!(k==masks[i][j].num_sprites-1&&split_ends))continue;

			json_t* coord=json_array();
			json_array_append_new(coord,json_integer(masks[i][j].masks[k].x_offset));
			json_array_append_new(coord,json_integer(masks[i][j].masks[k].y_offset));
			json_array_append_new(offset,coord);
			}
			if(any_nonzero_offset)json_object_set_new(view,"offset",offset);
			else json_decref(offset);

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
				else if(k+1<masks[i][j].num_sprites&&masks[i][j].masks[k+1].track_mask_op==TRACK_MASK_DIFFERENCE&&!(k+1==masks[i][j].num_sprites-1&&split_ends))
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
			if(any_split||(!any_transfer&&!any_nonzero_offset&&mask_num_sprites<masks[i][j].num_sprites))json_object_set_new(view,"split",split);
			else json_decref(split);
			if(any_transfer)json_object_set_new(view,"transfer",transfer);
			else json_decref(transfer);


		json_array_append_new(track_section,view);
		}
		if(json_array_size(track_section)>0)json_object_set_new(json,track_sections[i].name,track_section);
		else json_decref(track_section);
	}

sprintf(filename,"masks/%s.json",set_name);
json_dump_file(json,filename,JSON_INDENT(4)|JSON_COMPACT);
}

int compare_masks(view_t a[NUM_TRACK_SECTIONS][4],view_t b[NUM_TRACK_SECTIONS][4])
{
	for(int i=0;i<NUM_TRACK_SECTIONS;i++)
	{
	printf("Comparing section %s\n",track_sections[i].name);
		for(int j=0;j<4;j++)
		{
			if(a[i][j].flags!=b[i][j].flags)
			{
			printf("flags differ for view %d\n",j);
			return 0;
			}

			if(a[i][j].num_sprites!=b[i][j].num_sprites)
			{
			printf("num_sprites differs for view %d\n",j);
			return 0;
			}

			if((a[i][j].masks==NULL)!=(b[i][j].masks==NULL))
			{
			printf("masks differ for view %d\n",j);
			return 0;
			}

			if(a[i][j].masks==NULL)continue;
			for(int k=0;k<a[i][j].num_sprites;k++)
			{
				if(a[i][j].masks[k].track_mask_op!=b[i][j].masks[k].track_mask_op)
				{
				printf("track_mask_op differ for view %d\n",j);
				return 0;
				}

				if(a[i][j].masks[k].x_offset!=b[i][j].masks[k].x_offset)
				{
				printf("x_offset differ for view %d\n",j);
				return 0;
				}

				if(a[i][j].masks[k].y_offset!=b[i][j].masks[k].y_offset)
				{
				printf("y_offset differ for view %d\n",j);
				return 0;
				}
				if(strstr(track_sections[i].name,"right"))continue;

				if(a[i][j].masks[k].num_rects!=b[i][j].masks[k].num_rects)
				{
				printf("num_rects differ for view %d mask %d\n",j,k);
				return 0;
				}
				if(memcmp(a[i][j].masks[k].rects,b[i][j].masks[k].rects,a[i][j].masks[k].num_rects*sizeof(rect_t))!=0)
				{
				printf("rects differ for view %d mask %d\n",j,k);
				return 0;
				}
			}
		}
	}
return 1;
}
