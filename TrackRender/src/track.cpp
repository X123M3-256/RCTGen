#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "track.h"
#include "sprites.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

vector3_t start_offset;
vector3_t end_offset;

vector3_t change_coordinates(vector3_t a)
{
	vector3_t result={a.z,a.y,a.x};
	return result;
}

typedef struct
{
	track_point_t (*track_curve)(float);
	float scale;
	float offset;
	float z_offset;
	float length;
	int flags;
	float track_length;
}track_transform_args_t;

track_point_t get_track_point(track_point_t (*curve)(float distance),int flags,float z_offset,float length,float u)
{
	track_point_t track_point;
	if(u<0)
	{
		track_point=curve(0);
		track_point.position=vector3_add(track_point.position,vector3_mult(track_point.tangent,u));
	}
	else if(u>length)
	{
		track_point=curve(length);
		track_point.position=vector3_add(track_point.position,vector3_mult(track_point.tangent,(u-length)));
	}
	else
	{
		track_point=curve(u);
	}

	if(flags&TRACK_DIAGONAL)track_point.position.x+=0.5*TILE_SIZE;
	if(flags&TRACK_DIAGONAL_2)track_point.position.z+=0.5*TILE_SIZE;
	track_point.position.y+=z_offset-2*CLEARANCE_HEIGHT;
	if(!(flags&TRACK_VERTICAL))track_point.position.z-=0.5*TILE_SIZE;

	float v=u/length;
	if(v<0)v=0;
	else if(v>1)v=1;

	track_point.position=vector3_add(track_point.position,vector3_mult(start_offset,2*v*v*v-3*v*v+1));
	track_point.position=vector3_add(track_point.position,vector3_mult(end_offset,-2*v*v*v+3*v*v));
	return track_point;
}

track_point_t only_yaw(track_point_t input)
{
	track_point_t output;
	output.position=input.position;
	output.normal=vector3(0,1,0);
	output.binormal=vector3_normalize(vector3_cross(output.normal,change_coordinates(input.tangent)));
	output.tangent=vector3_normalize(vector3_cross(output.normal,output.binormal));
	return output;
}

vertex_t track_transform(vector3_t vertex,vector3_t normal,const bool flat_shaded,void* data)
{
	track_transform_args_t args=*((track_transform_args_t*)data);

	vertex.z=args.scale*vertex.z+args.offset;

	track_point_t track_point=get_track_point(args.track_curve,args.flags,args.z_offset,args.length,vertex.z);

	vertex_t out;
	out.vertex=change_coordinates(vector3_add(track_point.position,vector3_add(vector3_mult(track_point.normal,vertex.y),vector3_mult(track_point.binormal,vertex.x))));

	if (flat_shaded)
	{
		track_point_t central_track_point = get_track_point(args.track_curve, args.flags, args.z_offset, args.length, args.offset + (args.track_length / 2));
		out.normal = change_coordinates(vector3_add(vector3_mult(central_track_point.tangent, normal.z), vector3_add(vector3_mult(central_track_point.normal, normal.y), vector3_mult(central_track_point.binormal, normal.x))));
	}
	else
	{
		out.normal = change_coordinates(vector3_add(vector3_mult(track_point.tangent, normal.z), vector3_add(vector3_mult(track_point.normal, normal.y), vector3_mult(track_point.binormal, normal.x))));
	}
	return out;
}

vertex_t base_transform(vector3_t vertex,vector3_t normal,const bool flat_shaded,void* data)
{
	track_transform_args_t args=*((track_transform_args_t*)data);

	vertex.z=args.scale*vertex.z+args.offset;

	track_point_t track_point=get_track_point(args.track_curve,args.flags,args.z_offset,args.length,vertex.z);

	track_point.binormal=vector3_normalize(vector3_cross(vector3(0,1,0),track_point.tangent));
	track_point.normal=vector3_normalize(vector3_cross(track_point.tangent,track_point.binormal));

	vertex_t out;
	out.vertex=change_coordinates(vector3_add(track_point.position,vector3_add(vector3_mult(vector3(0,1,0),vertex.y),vector3_mult(track_point.binormal,vertex.x))));
	out.normal=change_coordinates(vector3_add(vector3_mult(track_point.tangent,normal.z),vector3_add(vector3_mult(track_point.normal,normal.y),vector3_mult(track_point.binormal,normal.x))));
	return out;
}

#define DENOM 6

int get_support_index(int bank)
{
	switch(bank)
	{
	case 0:
		return MODEL_FLAT;
		break;
	case -1:
	case 1:
		return MODEL_BANK_SIXTH;
		break;
	case -2:
	case 2:
		return MODEL_BANK_THIRD;
	case -3:
	case 3:
		return MODEL_BANK_HALF;
		break;
	case -4:
	case 4:
		return MODEL_BANK_TWO_THIRDS;
		break;
	case -5:
	case 5:
		return MODEL_BANK_FIVE_SIXTHS;
	case -6:
	case 6:
		return MODEL_BANK;
		break;
	}
	return MODEL_FLAT;
}

int get_special_index(int flags)
{
	switch(flags&TRACK_SPECIAL_MASK)
	{
	case TRACK_SPECIAL_STEEP_TO_VERTICAL:
		return MODEL_SPECIAL_STEEP_TO_VERTICAL;
		break;
	case TRACK_SPECIAL_VERTICAL_TO_STEEP:
		return MODEL_SPECIAL_VERTICAL_TO_STEEP;
		break;
	case TRACK_SPECIAL_VERTICAL:
		return MODEL_SPECIAL_VERTICAL;
		break;
	case TRACK_SPECIAL_VERTICAL_TWIST_LEFT:
	case TRACK_SPECIAL_VERTICAL_TWIST_RIGHT:
		return MODEL_SPECIAL_VERTICAL_TWIST;
		break;
	case TRACK_SPECIAL_BARREL_ROLL_LEFT:
	case TRACK_SPECIAL_BARREL_ROLL_RIGHT:
		return MODEL_SPECIAL_BARREL_ROLL;
		break;
	case TRACK_SPECIAL_HALF_LOOP:
		return MODEL_SPECIAL_HALF_LOOP;
		break;
	case TRACK_SPECIAL_QUARTER_LOOP:
		return MODEL_SPECIAL_QUARTER_LOOP;
		break;
	case TRACK_SPECIAL_CORKSCREW_LEFT:
	case TRACK_SPECIAL_CORKSCREW_RIGHT:
		return MODEL_SPECIAL_CORKSCREW;
		break;
	case TRACK_SPECIAL_ZERO_G_ROLL_LEFT:
	case TRACK_SPECIAL_ZERO_G_ROLL_RIGHT:
		return MODEL_SPECIAL_ZERO_G_ROLL;
		break;
	case TRACK_SPECIAL_LARGE_ZERO_G_ROLL_LEFT:
	case TRACK_SPECIAL_LARGE_ZERO_G_ROLL_RIGHT:
		return MODEL_SPECIAL_LARGE_ZERO_G_ROLL;
		break;
	case TRACK_SPECIAL_BRAKE:
		return MODEL_SPECIAL_BRAKE;
		break;
	case TRACK_SPECIAL_BLOCK_BRAKE:
		return MODEL_SPECIAL_BLOCK_BRAKE;
		break;
	case TRACK_SPECIAL_MAGNETIC_BRAKE:
		return MODEL_SPECIAL_MAGNETIC_BRAKE;
		break;
	case TRACK_SPECIAL_BOOSTER:
	case TRACK_SPECIAL_LAUNCHED_LIFT:
	case TRACK_SPECIAL_VERTICAL_BOOSTER:
		return MODEL_SPECIAL_BOOSTER;
		break;
	}
	assert(0);
	return 0;
}

void render_track_section(context_t* context,track_section_t* track_section,track_type_t* track_type,int extrude_behind,int extrude_in_front,int track_mask,int rendered_views,image_t* images)
{
	int num_meshes=(int)floor(0.5+track_section->length/track_type->length);
	float scale=track_section->length/(num_meshes*track_type->length);

	//If alternating track meshes are used,we would prefer to have an even number of meshes as long as it doesn't cause too much distortion
	if(track_type->models_loaded&(1<<MODEL_TRACK_ALT))
	{
		int num_meshes_even=2*(int)floor(0.5+track_section->length/(2*track_type->length));
		if(track_section->flags&TRACK_ALT_PREFER_ODD)num_meshes_even=2*(int)floor(track_section->length/(2*track_type->length))+1;
		float scale_even=track_section->length/(num_meshes_even*track_type->length);
		if(scale_even>0.9&&scale_even<1.11111)
		{
			num_meshes=num_meshes_even;
			scale=scale_even;
		}
	}

	float length=scale*track_type->length;
	float z_offset=((track_type->z_offset/8.0)*CLEARANCE_HEIGHT);

	mesh_t*	mesh=&(track_type->mesh);
	mesh_t* mesh_tie=&(track_type->mesh_tie);

	context_begin_render(context);

	//Add ghost models/track masks at start and end
	track_transform_args_t args;
	args.scale=scale;
	args.offset=-length;
	args.z_offset=z_offset;
	args.track_curve=track_section->curve;
	args.flags=track_section->flags;
	args.length=track_section->length;
	args.track_length = track_type->length;
	if(track_mask)context_add_model_transformed(context,&(track_type->mask),track_transform,&args,0);//);
	else if(!extrude_behind)context_add_model_transformed(context,mesh,track_transform,&args,MESH_GHOST);
	args.offset=track_section->length;
	if(track_mask)context_add_model_transformed(context,&(track_type->mask),track_transform,&args,0);//track_mask?0:MESH_GHOST);
	else if(!extrude_in_front)context_add_model_transformed(context,mesh,track_transform,&args,MESH_GHOST);

	if(track_type->flags&TRACK_TIE_AT_BOUNDARY)
	{
		//Determine start angle
		int start_angle=3;
		if(rendered_views&1)start_angle=0;
		else if(rendered_views&2)start_angle=1;
		else if(rendered_views&4)start_angle=2;
		if(track_section->flags&(TRACK_DIAGONAL|TRACK_DIAGONAL_2))start_angle=(start_angle+1) % 4;
		int end_angle=start_angle;
		if(track_section->flags&TRACK_EXIT_90_DEG_LEFT)end_angle-=1;
		else if(track_section->flags&TRACK_EXIT_90_DEG_RIGHT)end_angle+=1;
		else if(track_section->flags&TRACK_EXIT_180_DEG)end_angle+=2;
		else if((track_section->flags&TRACK_EXIT_45_DEG_LEFT)&&(track_section->flags&(TRACK_DIAGONAL|TRACK_DIAGONAL_2)))end_angle-=1;
		else if((track_section->flags&TRACK_EXIT_45_DEG_RIGHT)&&!(track_section->flags&(TRACK_DIAGONAL|TRACK_DIAGONAL_2)))end_angle+=1;
		if(end_angle<0)end_angle+=4;
		if(end_angle>3)end_angle-=4;

		int start_tie=start_angle <=1;
		int end_tie=end_angle>1;

		//Calculate corrected scale
		double corrected_length=num_meshes*track_type->length;
		if(!start_tie)corrected_length-=track_type->tie_length;
		if(end_tie)corrected_length+=track_type->tie_length;
		double corrected_scale=track_section->length/corrected_length;

		double tie_length=corrected_scale*track_type->tie_length;
		double inter_length=corrected_scale*(track_type->length-track_type->tie_length);

		double offset=0;

		if(extrude_behind)
		{
			num_meshes++;
			offset-=(extrude_behind ? 1 : 0)*corrected_scale*track_type->length;
		}
		if(extrude_in_front)num_meshes++;
		for(int i=0; i<2*num_meshes+1; i++)
		{
			track_transform_args_t args;
			args.scale=corrected_scale;
			args.offset=offset;
			args.z_offset=z_offset;
			args.track_curve=track_section->curve;
			args.flags=track_section->flags;
			args.length=track_section->length;
			args.track_length = track_type->length;
			if((!(i&1))&&(i !=0||start_tie)&&(i !=2*num_meshes||end_tie))
			{
				track_point_t track_point=get_track_point(track_section->curve,track_section->flags,z_offset,args.length,args.offset+track_type->tie_length/2);
				context_add_model(
				    context,&(track_type->tie_mesh),
				    transform(
				        matrix(track_point.binormal.z,track_point.normal.z,track_point.tangent.z,track_point.binormal.y,track_point.normal.y,track_point.tangent.y,track_point.binormal.x,track_point.normal.x,track_point.tangent.x),
				        change_coordinates(track_point.position)
				    ),
				    track_mask
				);
				context_add_model_transformed(context,mesh_tie,track_transform,&args,track_mask);
				offset+=tie_length;
			}
			else if(i&1)
			{
				int use_alt=i&2;
				if(track_section->flags&TRACK_ALT_INVERT)use_alt=!use_alt;
				if(extrude_behind)use_alt=!use_alt;
				if(!(track_type->models_loaded&(1<<MODEL_TRACK_ALT)))use_alt=0;
				//Add track model
				if(use_alt)context_add_model_transformed(context,&(track_type->models[MODEL_TRACK_ALT]),track_transform,&args,track_mask);
				else context_add_model_transformed(context,mesh,track_transform,&args,track_mask);
				//Add track mask
				if(track_mask)
				{
					if(start_tie)args.offset=offset-tie_length;
					context_add_model_transformed(context,&(track_type->mask),track_transform,&args,0);
				}
				offset+=inter_length;
			}
		}




	}
	else
	{
		if(extrude_behind)num_meshes++;
		if(extrude_in_front)num_meshes++;
		for(int i=0; i<num_meshes; i++)
		{
			track_transform_args_t args;
			args.scale=scale;
			args.offset=(i-(extrude_behind ? 1 : 0))*length;
			args.z_offset=z_offset;
			args.track_curve=track_section->curve;
			args.flags=track_section->flags;
			args.length=track_section->length;
			args.track_length = track_type->length;

			int alt_available=track_type->models_loaded&(1<<MODEL_TRACK_ALT);
			int use_alt=alt_available&&(i&1);
			if(alt_available&&(track_section->flags&TRACK_ALT_INVERT))use_alt=!use_alt;

			if(track_mask)context_add_model_transformed(context,&(track_type->mask),track_transform,&args,0);
			if(use_alt)context_add_model_transformed(context,&(track_type->models[MODEL_TRACK_ALT]),track_transform,&args,track_mask);
			else context_add_model_transformed(context,mesh,track_transform,&args,track_mask);

			if((track_type->models_loaded&(1<<MODEL_BASE))&&(track_type->flags&TRACK_HAS_SUPPORTS)&&!(track_section->flags&TRACK_NO_SUPPORTS))
				context_add_model_transformed(context,&(track_type->models[MODEL_BASE]),base_transform,&args,track_mask);
			if(track_type->flags&TRACK_SEPARATE_TIE)
			{
				track_point_t track_point=get_track_point(track_section->curve,track_section->flags,z_offset,args.length,args.offset+0.5*length);
				context_add_model(
				    context,&(track_type->tie_mesh),
				    transform(
				        matrix(track_point.binormal.z,track_point.normal.z,track_point.tangent.z,track_point.binormal.y,track_point.normal.y,track_point.tangent.y,track_point.binormal.x,track_point.normal.x,track_point.tangent.x),
				        change_coordinates(track_point.position)
				    ),
				    track_mask
				);
			}
		}
	}

	if(track_section->flags&TRACK_SPECIAL_MASK)
	{
		int index=get_special_index(track_section->flags);
		if(track_type->models_loaded&(1<<index))
		{
			matrix_t mat=views[1];
			if((track_section->flags&TRACK_SPECIAL_MASK) !=TRACK_SPECIAL_VERTICAL_TWIST_RIGHT&&(track_section->flags&TRACK_SPECIAL_MASK) !=TRACK_SPECIAL_BARREL_ROLL_RIGHT
			  &&(track_section->flags&TRACK_SPECIAL_MASK) !=TRACK_SPECIAL_CORKSCREW_RIGHT&&(track_section->flags&TRACK_SPECIAL_MASK) !=TRACK_SPECIAL_ZERO_G_ROLL_RIGHT
			  &&(track_section->flags&TRACK_SPECIAL_MASK) !=TRACK_SPECIAL_LARGE_ZERO_G_ROLL_RIGHT)
			{
				mat.entries[6]*=-1;
				mat.entries[7]*=-1;
				mat.entries[8]*=-1;
			}

			if((track_section->flags&TRACK_SPECIAL_MASK) == TRACK_SPECIAL_BRAKE || (track_section->flags&TRACK_SPECIAL_MASK) == TRACK_SPECIAL_MAGNETIC_BRAKE || (track_section->flags&TRACK_SPECIAL_MASK) == TRACK_SPECIAL_BLOCK_BRAKE || (track_section->flags&TRACK_SPECIAL_MASK) == TRACK_SPECIAL_BOOSTER)
			{
			float special_length=track_type->brake_length;
				if((track_section->flags&TRACK_SPECIAL_MASK) == TRACK_SPECIAL_BLOCK_BRAKE)special_length=TILE_SIZE;
			int num_special_meshes=(int)floor(0.5+track_section->length/special_length);
			float special_scale=track_section->length/(num_special_meshes*special_length);
			special_length=special_scale*special_length;
				for(int i=0; i<num_special_meshes; i++)
				{
				track_transform_args_t args;
				args.scale=special_scale;
				args.offset=i*special_length;
				args.z_offset=z_offset;
				args.track_curve=track_section->curve;
				args.flags=track_section->flags;
				args.length=track_section->length;
				args.track_length = track_type->length;

				context_add_model_transformed(context,&(track_type->models[index]),track_transform,&args,track_mask);
				}
			} else context_add_model(context,&(track_type->models[index]),transform(mat,vector3(!(track_section->flags&TRACK_VERTICAL) ? -0.5*TILE_SIZE : 0,z_offset-2*CLEARANCE_HEIGHT,0)),track_mask);
		}
	}

	if((track_type->flags&TRACK_HAS_SUPPORTS)&&!(track_section->flags&TRACK_NO_SUPPORTS))
	{
		int num_supports=(int)floor(0.5+track_section->length/track_type->support_spacing);
		float support_step=track_section->length/num_supports;
		int entry=0;
		int exit=0;
		if(track_section->flags&TRACK_ENTRY_BANK_LEFT)entry=DENOM;
		else if(track_section->flags&TRACK_ENTRY_BANK_RIGHT)entry=-DENOM;
		else entry=0;

		if(track_section->flags&TRACK_EXIT_BANK_LEFT)exit=DENOM;
		else if(track_section->flags&TRACK_EXIT_BANK_RIGHT)exit=-DENOM;
		else exit=0;

		for(int i=0; i<num_supports+1; i++)
		{
			int u=(i*DENOM)/num_supports;
			int bank_angle=(entry*(DENOM-u)+(exit*u))/DENOM;

			track_point_t track_point=get_track_point(track_section->curve,track_section->flags,z_offset,track_section->length,i*support_step);

			track_point_t support_point=only_yaw(track_point);

			matrix_t rotation=
			    matrix(support_point.binormal.x,support_point.normal.x,support_point.tangent.x,support_point.binormal.y,support_point.normal.y,support_point.tangent.y,support_point.binormal.z,support_point.normal.z,support_point.tangent.z);
			if(bank_angle >=0)rotation=matrix_mult(views[2],rotation);

			vector3_t translation=change_coordinates(support_point.position);
			translation.y-=track_type->pivot/sqrt(track_point.tangent.x*track_point.tangent.x+track_point.tangent.z*track_point.tangent.z)-track_type->pivot;

			context_add_model(context,&(track_type->models[get_support_index(bank_angle)]),transform(rotation,translation),track_mask);
		}
	}

	context_finalize_render(context);

	for(int i=0; i<4; i++)
	{
		if(rendered_views&(1<<i))
		{
			if(track_mask)context_render_silhouette(context,rotate_y(0.5*i*M_PI),images+i);
			else context_render_view(context,rotate_y(0.5*i*M_PI),images+i);
		}
	}
	context_end_render(context);
}

int is_in_mask(int x,int y,mask_t* mask)
{
	for(int i=0; i<mask->num_rects; i++)
	{
		if(x >=mask->rects[i].x_lower&&x<mask->rects[i].x_upper&&y >=mask->rects[i].y_lower&&y<mask->rects[i].y_upper)return 1;
	}
	return 0;
}

int compare_vec(vector3_t vec1,vector3_t vec2,int rot)
{
	return vector3_norm(vector3_sub(vec1,vector3_normalize(matrix_vector(views[rot],vec2))))<0.15;
}

int offset_table_index_with_rot(track_point_t track,int rot)
{

	//Get bank angle
	int banked=fabs(fabs(asin(sqrt(track.normal.x*track.normal.x+track.normal.z*track.normal.z)))-0.25*M_PI)<0.1;
	int right=(banked&&track.binormal.y<0) ? 0x10 : 0;
	//Flat
	if(compare_vec(track.tangent,vector3(0,0,TILE_SIZE),rot))
	{
		//Inverted
		if(track.normal.y<-0.9)return OFFSET_INVERTED;
		//Banked
		else if(banked)return right|OFFSET_BANK;
		//Unbanked
		else return OFFSET_FLAT;
	}
	//Gentle
	else if(compare_vec(track.tangent,vector3(0,2*CLEARANCE_HEIGHT,TILE_SIZE),rot))
	{
		if(banked)return right|OFFSET_GENTLE_BANK;
		else return OFFSET_GENTLE;
	}
	//Steep
	else if(compare_vec(track.tangent,vector3(0,8*CLEARANCE_HEIGHT,TILE_SIZE),rot))return OFFSET_STEEP;
	//Diagonal flat
	else if(compare_vec(track.tangent,vector3(-TILE_SIZE,0,TILE_SIZE),rot))
	{
		if(banked)return right|OFFSET_DIAGONAL_BANK;
		return OFFSET_DIAGONAL;
	}
	//Diagonal gentle
	else if(compare_vec(track.tangent,vector3(-TILE_SIZE,2*CLEARANCE_HEIGHT,TILE_SIZE),rot)&&!banked)
	{
	return OFFSET_DIAGONAL_GENTLE;
	}
	//Diagonal steep
	else if(compare_vec(track.tangent,vector3(-TILE_SIZE,8*CLEARANCE_HEIGHT,TILE_SIZE),rot))return OFFSET_DIAGONAL_STEEP;//Banked is true even for unbanked track; TODO fix this
	return 0xFF;
}

int offset_table_index(track_point_t track)
{
	//Check straight
	int index=offset_table_index_with_rot(track,0);
	if(index !=0xFF)return index;

	//Check left
	index=offset_table_index_with_rot(track,1);
	if(index !=0xFF)return 0x60|index;

	//Check right
	index=offset_table_index_with_rot(track,3);
	if(index !=0xFF)return 0x20|index;
	return 0xFF;

return 0xFF;
}

/*
y        z
|      -
|   -
|_
*/

//LIM
/*
float offset_tables[10][8]={
    {0,0.5,0,0,0,0.5,0,0},
    {0,1.0,0,0,0,0,0,0},//Gentle
    {-2.25,0,-2.0,0,-0.75,0,-1.5,-1.0},//Steep
    {0,1.0,0,1.0,0,1.0,0,0},//Bank
    {0,0,0,0,0,0,0,0},//Gentle Bank
    {0,-0.5,0,0,0,-0.5,0,0},//Inverted
    {0,0,0,0,0,0,0,0},//Diagonal
    {0,0,0,0,0,0,0,0},//Diagonal Bank
    {0,0.5,0,0,0,0.75,0,0},//Diagonal gentle
    {0,0,0,0,0,0,0,0},//Other
    };
*/

vector3_t get_offset(int table,int view_angle,float* offset_table)
{
	int index=table&0xF;
	int end_angle=table>>5;
	int right=(table&0x10)>>4;
	//printf("index %d end angle %d right %d\n",index,end_angle,right);
	int rotated_view_angle=(view_angle+end_angle+2*right) % 4;
	//printf("view %d rotated view %d\n",view_angle,rotated_view_angle);

	vector3_t offset=vector3(0,0,0);
	if(table ==0xFF)return offset;

	offset.x=0;
	offset.z=offset_table[8*index+2*rotated_view_angle]*TILE_SIZE/32.0;
	offset.y=offset_table[8*index+2*rotated_view_angle+1]*CLEARANCE_HEIGHT/8.0;

	//Check if right banked
	if(right)
	{
		offset.z*=-1;
	}

	//Check if diagonal
	if(index >=6&&index <=8)
	{
		offset.z*=M_SQRT1_2;
		offset.x=offset.z;
	}

	//Correct for end rotation
	if(end_angle !=0)offset=matrix_vector(rotate_y(-0.5*M_PI*end_angle),offset);

	//printf("Offset %d Rotation %d x %.2f y %.2f\n",index,rotated_view_angle,offset_table[8*index+2*rotated_view_angle],offset_table[8*index+2*rotated_view_angle+1]);
	return offset;
}

void set_offset(int view_angle,track_section_t* track_section,float* offset_table)
{
	int start_table=offset_table_index(track_section->curve(0));
	int end_table=offset_table_index(track_section->curve(track_section->length));

	start_offset=get_offset(start_table,view_angle,offset_table);
	end_offset=get_offset(end_table,view_angle,offset_table);
}

void render_track_sections(context_t* context,track_section_t* track_section,track_type_t* track_type,int track_mask,int views,image_t* sprites)
{
int extrude_behind=track_section->flags&TRACK_EXTRUDE_BEHIND;
int extrude_in_front_even=!(track_section->flags&TRACK_EXIT_45_DEG_LEFT)&&(track_section->flags&TRACK_EXTRUDE_IN_FRONT);
int extrude_in_front_odd=(track_section->flags&TRACK_EXIT_45_DEG_LEFT)&&(track_section->flags&TRACK_EXTRUDE_IN_FRONT);

	if(track_type->flags&TRACK_SPECIAL_OFFSETS)
	{
	set_offset(0,track_section,track_type->offset_table);
		if(views&0x1)render_track_section(context,track_section,track_type,extrude_behind,extrude_in_front_even,track_mask,0x1,sprites);
	set_offset(1,track_section,track_type->offset_table);
		if(views&0x2)render_track_section(context,track_section,track_type,0,extrude_in_front_odd,track_mask,0x2,sprites);
	set_offset(2,track_section,track_type->offset_table);
		if(views&0x4)render_track_section(context,track_section,track_type,extrude_behind,extrude_in_front_even,track_mask,0x4,sprites);
	set_offset(3,track_section,track_type->offset_table);
		if(views&0x8)render_track_section(context,track_section,track_type,0,extrude_in_front_odd,track_mask,0x8,sprites);
	return;
	}

	if((track_section->flags&TRACK_EXTRUDE_BEHIND)||(track_section->flags&TRACK_EXTRUDE_IN_FRONT))
	{
		if(track_type->flags&TRACK_SEPARATE_TIE)
		{
			if(views&0x1)render_track_section(context,track_section,track_type,extrude_behind,extrude_in_front_even,track_mask,0x1,sprites);
			if(views&0x2)render_track_section(context,track_section,track_type,0,extrude_in_front_odd,track_mask,0x2,sprites);
			if(views&0x4)render_track_section(context,track_section,track_type,extrude_behind,extrude_in_front_even,track_mask,0x4,sprites);
			if(views&0x8)render_track_section(context,track_section,track_type,0,extrude_in_front_odd,track_mask,0x8,sprites);
		}
		else
		{
			if(views&0x5)render_track_section(context,track_section,track_type,extrude_behind,extrude_in_front_even,track_mask,views&0x5,sprites);
			if(views&0xA)render_track_section(context,track_section,track_type,0,extrude_in_front_odd,track_mask,views&0xA,sprites);
		}
	}
	else
	{
		if((track_type->flags&TRACK_SEPARATE_TIE)&&(track_section->flags&TRACK_EXIT_90_DEG))
		{
			if(views&0x1)render_track_section(context,track_section,track_type,0,0,track_mask,0x1,sprites);
			if(views&0x2)render_track_section(context,track_section,track_type,0,0,track_mask,0x2,sprites);
			if(views&0x4)render_track_section(context,track_section,track_type,0,0,track_mask,0x4,sprites);
			if(views&0x8)render_track_section(context,track_section,track_type,0,0,track_mask,0x8,sprites);
		}
		else if((track_type->flags&TRACK_SEPARATE_TIE))
		{
			if(views&0x3)render_track_section(context,track_section,track_type,0,0,track_mask,views&0x3,sprites);
			if(views&0xC)render_track_section(context,track_section,track_type,0,0,track_mask,views&0xC,sprites);
		}
		else
		{
			render_track_section(context,track_section,track_type,0,0,track_mask,views,sprites);
		}
	}
}

void write_track_section(context_t* context,int track_section_id,track_type_t* track_type,const char* base_directory,const char* output_directory,json_t* sprites)
{
track_section_t* track_section=track_sections+track_section_id;
view_t* views=default_masks[track_section_id];
image_t* overlay=NULL;
const char* suffix="";

	int z_offset=(int)(track_type->z_offset+0.499999);
	image_t full_sprites[4];
	render_track_sections(context,track_section,track_type,0,0xF,full_sprites);

	if(overlay !=NULL&&!(track_type->flags&TRACK_NO_LIFT_SPRITE))
	{
		for(int i=0; i<4; i++)image_blit(full_sprites+i,overlay+i,0,track_type->lift_offset-z_offset);
	}

	image_t track_masks[4];
	int track_mask_views=0;
	for(int i=0; i<4; i++)track_mask_views|=(views[i].flags&VIEW_NEEDS_TRACK_MASK ? 1 : 0)<<i;
	if(track_mask_views !=0)render_track_sections(context,track_section,track_type,1,track_mask_views,track_masks);

	for(int angle=0; angle<4; angle++)
	{
		if(views[angle].num_sprites ==0)continue;

		view_t* view=views+angle;

		char final_filename[512];
		char relative_filename[512];
		snprintf(relative_filename,512,"%s%s%s_%d.png",output_directory,track_section->name,suffix,angle+1);

		for(int sprite=0; sprite<view->num_sprites; sprite++)
		{
			char final_filename[512];
			char relative_filename[512];
			if(view->num_sprites==1)snprintf(relative_filename,512,"%s%s%s_%d.png",output_directory,track_section->name,suffix,angle+1);
			else snprintf(relative_filename,512,"%s%s%s_%d_%d.png",output_directory,track_section->name,suffix,angle+1,sprite+1);
			snprintf(final_filename,512,"%s%s",base_directory,relative_filename);
			//y		snprintf(final_filename,512,"../ImageEncode/%s",relative_filename);
			printf("%s\n",final_filename);

			image_t part_sprite;
			image_copy(full_sprites+angle,&part_sprite);

			if(view->masks !=NULL)
			{
				for(int x=0; x<full_sprites[angle].width; x++)
					for(int y=0; y<full_sprites[angle].height; y++)
					{
						int in_mask=is_in_mask(x+full_sprites[angle].x_offset,y+full_sprites[angle].y_offset+((track_section->flags&TRACK_OFFSET_SPRITE_MASK) ? (z_offset-8) : 0),view->masks+sprite);

						if(view->masks[sprite].track_mask_op !=TRACK_MASK_NONE)
						{
							int mask_x=(x+full_sprites[angle].x_offset)-track_masks[angle].x_offset;
							int mask_y=(y+full_sprites[angle].y_offset)-track_masks[angle].y_offset;
							int in_track_mask=mask_x >=0&&mask_y >=0&&mask_x<track_masks[angle].width&&mask_y<track_masks[angle].height&&track_masks[angle].pixels[mask_x+mask_y*track_masks[angle].width] !=0;

							switch(view->masks[sprite].track_mask_op)
							{
							case TRACK_MASK_DIFFERENCE:
								in_mask=in_mask&&!in_track_mask;
								break;
							case TRACK_MASK_INTERSECT:
								in_mask=in_mask&&in_track_mask;
								break;
							}

							if(sprite<view->num_sprites-1&&(view->masks[sprite].track_mask_op&TRACK_MASK_TRANSFER_NEXT)&&in_track_mask
							  &&is_in_mask(x+full_sprites[angle].x_offset,y+full_sprites[angle].y_offset+((track_section->flags&TRACK_OFFSET_SPRITE_MASK) ? (z_offset-8) : 0),view->masks+sprite+1))
								in_mask=1;
						}

						if(in_mask)
						{
							part_sprite.pixels[x+part_sprite.width*y]=full_sprites[angle].pixels[x+full_sprites[angle].width*y];
						}
						else
						{
							part_sprite.pixels[x+part_sprite.width*y]=0;
						}
					}
				part_sprite.x_offset+=view->masks[sprite].x_offset;
				part_sprite.y_offset+=view->masks[sprite].y_offset;
			}

			FILE* file=fopen(final_filename,"wb");
			if(file ==NULL)
			{
				printf("Error: could not open %s for writing\n",final_filename);
				exit(1);
			}
			//if(view->flags&VIEW_NEEDS_TRACK_MASK)image_write_png(&(track_masks[angle]),file);
			image_crop(&part_sprite);
			image_write_png(&part_sprite, NULL, file);
			//image_write_png(full_sprites+angle,file);
			fclose(file);

			json_t* sprite_entry=json_object();
			json_object_set(sprite_entry,"path",json_string(relative_filename));
			json_object_set(sprite_entry,"x",json_integer(part_sprite.x_offset));
			json_object_set(sprite_entry,"y",json_integer(part_sprite.y_offset));
			json_object_set(sprite_entry,"palette",json_string("keep"));
			json_array_append(sprites,sprite_entry);
			image_destroy(&part_sprite);
		}

		if(view->flags&VIEW_NEEDS_TRACK_MASK)image_destroy(track_masks+angle);
		image_destroy(full_sprites+angle);
	}
}

int write_track_type(context_t* context,track_type_t* track_type,json_t* sprites,const char* base_dir,const char* output_dir)
{
uint64_t groups=track_type->groups;

	//Flat
	if(groups&TRACK_GROUP_FLAT)
	{
	write_track_section(context,FLAT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_BRAKES)
	{
	write_track_section(context,BRAKE,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_BLOCK_BRAKES)
	{
	write_track_section(context,BLOCK_BRAKE,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_SLOPED_BRAKES)
	{
	write_track_section(context,BRAKE_GENTLE,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_MAGNETIC_BRAKES)
	{
	write_track_section(context,MAGNETIC_BRAKE,track_type,base_dir,output_dir,sprites);
	}


	if(groups&TRACK_GROUP_BOOSTERS)
	{
	write_track_section(context,BOOSTER,track_type,base_dir,output_dir,sprites);
	}
	//Launched lift
	if(groups&TRACK_GROUP_LAUNCHED_LIFTS)
	{
	write_track_section(context,LAUNCHED_LIFT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_VERTICAL_BOOSTERS)
	{
	write_track_section(context,VERTICAL_BOOSTER,track_type,base_dir,output_dir,sprites);
	}

	//Slopes
	if(groups&TRACK_GROUP_GENTLE_SLOPES)
	{
	write_track_section(context,FLAT_TO_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_TO_FLAT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE,track_type,base_dir,output_dir,sprites);
	}
	//TODO should probably be inside slopes
		if(groups&TRACK_GROUP_MAGNETIC_BRAKES)
		{
		write_track_section(context,MAGNETIC_BRAKE_GENTLE,track_type,base_dir,output_dir,sprites);
		}

	if(groups&TRACK_GROUP_STEEP_SLOPES)
	{
	write_track_section(context,GENTLE_TO_STEEP,track_type,base_dir,output_dir,sprites);
	write_track_section(context,STEEP_TO_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,STEEP,track_type,base_dir,output_dir,sprites);
	}
	
	if(groups&TRACK_GROUP_VERTICAL_SLOPES)
	{
	write_track_section(context,STEEP_TO_VERTICAL,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL_TO_STEEP,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL,track_type,base_dir,output_dir,sprites);
	}

	//Turns
	if(groups&TRACK_GROUP_TURNS)
	{
	write_track_section(context,SMALL_TURN_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_TURN_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_LEFT_TO_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_RIGHT_TO_DIAG,track_type,base_dir,output_dir,sprites);
	}

	//Diagonals
	if(groups&TRACK_GROUP_DIAGONALS)
	{
	write_track_section(context,FLAT_DIAG,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_DIAGONAL_BRAKES)
	{
		if(groups&TRACK_GROUP_BRAKES)
		{
		write_track_section(context,BRAKE_DIAG,track_type,base_dir,output_dir,sprites);
		}
		if(groups&TRACK_GROUP_BLOCK_BRAKES)
		{
		write_track_section(context,BLOCK_BRAKE_DIAG,track_type,base_dir,output_dir,sprites);
		}
		if(groups&TRACK_GROUP_MAGNETIC_BRAKES)
		{
		write_track_section(context,MAGNETIC_BRAKE_DIAG,track_type,base_dir,output_dir,sprites);
		}
	};
	if((groups&TRACK_GROUP_DIAGONALS)&&(groups&TRACK_GROUP_GENTLE_SLOPES))
	{
	write_track_section(context,FLAT_TO_GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_TO_FLAT_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
	};
	if(groups&TRACK_GROUP_DIAGONAL_BRAKES)
	{
		if(groups&TRACK_GROUP_SLOPED_BRAKES)
		{
		write_track_section(context,BRAKE_GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
		}
		if(groups&TRACK_GROUP_MAGNETIC_BRAKES)
		{
		write_track_section(context,MAGNETIC_BRAKE_GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
		}
	};
	if((groups&TRACK_GROUP_DIAGONALS)&&(groups&TRACK_GROUP_STEEP_SLOPES))
	{
	write_track_section(context,GENTLE_TO_STEEP_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,STEEP_TO_GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,STEEP_DIAG,track_type,base_dir,output_dir,sprites);
	}


/*
	if((groups&TRACK_GROUP_DIAGONALS)&&(groups&TRACK_GROUP_VERTICAL_SLOPES))
	{
	write_track_section(context,STEEP_TO_VERTICAL_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL_TO_STEEP_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL_TWIST_LEFT_TO_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL_TWIST_RIGHT_TO_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL_TWIST_LEFT_TO_ORTHOGONAL,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL_TWIST_RIGHT_TO_ORTHOGONAL,track_type,base_dir,output_dir,sprites);
	}
*/
	//Banked turns
	if(groups&TRACK_GROUP_BANKED_TURNS)
	{
	write_track_section(context,FLAT_TO_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,FLAT_TO_RIGHT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LEFT_BANK_TO_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,RIGHT_BANK_TO_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_TO_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_TO_RIGHT_BANK,track_type,base_dir,output_dir,sprites);

	write_track_section(context,LEFT_BANK,track_type,base_dir,output_dir,sprites);

		if(groups&TRACK_GROUP_DIAGONALS)
		{
		write_track_section(context,FLAT_TO_LEFT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
		write_track_section(context,FLAT_TO_RIGHT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
		write_track_section(context,LEFT_BANK_TO_GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
		write_track_section(context,RIGHT_BANK_TO_GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
		write_track_section(context,GENTLE_TO_LEFT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
		write_track_section(context,GENTLE_TO_RIGHT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
		write_track_section(context,LEFT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
		}

	write_track_section(context,SMALL_TURN_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_TURN_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_LEFT_TO_DIAG_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_RIGHT_TO_DIAG_BANK,track_type,base_dir,output_dir,sprites);
	}

	//Sloped turns
	if(groups&TRACK_GROUP_SLOPED_TURNS&&(groups&TRACK_GROUP_GENTLE_SLOPES))
	{
	write_track_section(context,SMALL_TURN_LEFT_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,SMALL_TURN_RIGHT_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_TURN_LEFT_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_TURN_RIGHT_GENTLE,track_type,base_dir,output_dir,sprites);
	}
	if((groups&TRACK_GROUP_STEEP_SLOPED_TURNS)&&(groups&TRACK_GROUP_STEEP_SLOPES))
	{
	write_track_section(context,VERY_SMALL_TURN_LEFT_STEEP,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERY_SMALL_TURN_RIGHT_STEEP,track_type,base_dir,output_dir,sprites);
	}
	if((groups&TRACK_GROUP_SLOPED_TURNS)&&(groups&TRACK_GROUP_VERTICAL_SLOPES))
	{
	write_track_section(context,VERTICAL_TWIST_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL_TWIST_RIGHT,track_type,base_dir,output_dir,sprites);
	}

	//Sloped banked turns

	if(groups&TRACK_GROUP_BANKED_SLOPED_TURNS)
	{
	write_track_section(context,GENTLE_TO_GENTLE_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_TO_GENTLE_RIGHT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_LEFT_BANK_TO_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_RIGHT_BANK_TO_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LEFT_BANK_TO_GENTLE_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,RIGHT_BANK_TO_GENTLE_RIGHT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_LEFT_BANK_TO_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_RIGHT_BANK_TO_RIGHT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_RIGHT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,FLAT_TO_GENTLE_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,FLAT_TO_GENTLE_RIGHT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_LEFT_BANK_TO_FLAT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_RIGHT_BANK_TO_FLAT,track_type,base_dir,output_dir,sprites);

	write_track_section(context,SMALL_TURN_LEFT_BANK_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,SMALL_TURN_RIGHT_BANK_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_TURN_LEFT_BANK_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_TURN_RIGHT_BANK_GENTLE,track_type,base_dir,output_dir,sprites);
	}

	//Miscellaneous
	if(groups&TRACK_GROUP_S_BENDS)
	{
	write_track_section(context,S_BEND_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,S_BEND_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_BANKED_S_BENDS)
	{
	write_track_section(context,S_BEND_LEFT_BANK,track_type,base_dir,output_dir,sprites);
	write_track_section(context,S_BEND_RIGHT_BANK,track_type,base_dir,output_dir,sprites);
	}

	if(groups&TRACK_GROUP_HELICES)
	{
	write_track_section(context,SMALL_HELIX_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,SMALL_HELIX_RIGHT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_HELIX_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_HELIX_RIGHT,track_type,base_dir,output_dir,sprites);
	}

	//Inversions
	if(groups&TRACK_GROUP_BARREL_ROLLS)
	{
	write_track_section(context,BARREL_ROLL_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,BARREL_ROLL_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_INLINE_TWISTS)
	{
	write_track_section(context,INLINE_TWIST_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,INLINE_TWIST_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_HALF_LOOPS)
	{
	write_track_section(context,HALF_LOOP,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_VERTICAL_LOOPS)
	{
	write_track_section(context,VERTICAL_LOOP_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,VERTICAL_LOOP_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_LARGE_SLOPE_TRANSITIONS)
	{
	write_track_section(context,FLAT_TO_STEEP,track_type,base_dir,output_dir,sprites);
	write_track_section(context,STEEP_TO_FLAT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,FLAT_TO_STEEP_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,STEEP_TO_FLAT_DIAG,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_QUARTER_LOOPS)
	{
	write_track_section(context,QUARTER_LOOP,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_CORKSCREWS)
	{
	write_track_section(context,CORKSCREW_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,CORKSCREW_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_LARGE_CORKSCREWS)
	{
	write_track_section(context,LARGE_CORKSCREW_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_CORKSCREW_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_TURN_BANK_TRANSITIONS)
	{
	write_track_section(context,SMALL_TURN_LEFT_BANK_TO_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,SMALL_TURN_RIGHT_BANK_TO_GENTLE,track_type,base_dir,output_dir,sprites);
	}

	if(groups&TRACK_GROUP_MEDIUM_HALF_LOOPS)
	{
	write_track_section(context,MEDIUM_HALF_LOOP_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,MEDIUM_HALF_LOOP_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_LARGE_HALF_LOOPS)
	{
	write_track_section(context,LARGE_HALF_LOOP_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_HALF_LOOP_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_ZERO_G_ROLLS)
	{
	write_track_section(context,ZERO_G_ROLL_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,ZERO_G_ROLL_RIGHT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_ZERO_G_ROLL_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_ZERO_G_ROLL_RIGHT,track_type,base_dir,output_dir,sprites);
	}
	if(groups&TRACK_GROUP_DIVE_LOOPS)
	{
	write_track_section(context,DIVE_LOOP_45_LEFT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,DIVE_LOOP_45_RIGHT,track_type,base_dir,output_dir,sprites);
/*
	write_track_section(context,&(track_list.dive_loop_90_left),track_type,base_dir,output_dir,sprites);
	write_track_section(context,&(track_list.dive_loop_90_right),track_type,base_dir,output_dir,sprites);*/
	}

	if(groups&TRACK_GROUP_SMALL_SLOPE_TRANSITIONS)
	{
	write_track_section(context,SMALL_FLAT_TO_STEEP,track_type,base_dir,output_dir,sprites);
	write_track_section(context,SMALL_STEEP_TO_FLAT,track_type,base_dir,output_dir,sprites);
	write_track_section(context,SMALL_FLAT_TO_STEEP_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,SMALL_STEEP_TO_FLAT_DIAG,track_type,base_dir,output_dir,sprites);
	}

	if(groups&TRACK_GROUP_LARGE_SLOPED_TURNS)
	{
	write_track_section(context,LARGE_TURN_LEFT_TO_DIAG_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_RIGHT_TO_DIAG_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_LEFT_TO_ORTHOGONAL_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_RIGHT_TO_ORTHOGONAL_GENTLE,track_type,base_dir,output_dir,sprites);
	}

	if(groups&TRACK_GROUP_LARGE_BANKED_SLOPED_TURNS)
	{
	write_track_section(context,GENTLE_TO_GENTLE_LEFT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_TO_GENTLE_RIGHT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_LEFT_BANK_TO_GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_RIGHT_BANK_TO_GENTLE_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LEFT_BANK_TO_GENTLE_LEFT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,RIGHT_BANK_TO_GENTLE_RIGHT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_LEFT_BANK_TO_LEFT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_RIGHT_BANK_TO_RIGHT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_LEFT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_RIGHT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,FLAT_TO_GENTLE_LEFT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,FLAT_TO_GENTLE_RIGHT_BANK_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_LEFT_BANK_TO_FLAT_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,GENTLE_RIGHT_BANK_TO_FLAT_DIAG,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_LEFT_BANK_TO_DIAG_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_RIGHT_BANK_TO_DIAG_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_LEFT_BANK_TO_ORTHOGONAL_GENTLE,track_type,base_dir,output_dir,sprites);
	write_track_section(context,LARGE_TURN_RIGHT_BANK_TO_ORTHOGONAL_GENTLE,track_type,base_dir,output_dir,sprites);
	}
	return 0;
}

