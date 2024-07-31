#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <jansson.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <embree3/rtcore.h>
#include "model.h"
#include "renderer.h"

#define TILE_SIZE 3.3
#define CLEARANCE_HEIGHT (0.5*TILE_SIZE/sqrt(6.0))
#define MAX_MESHES_PER_MODEL 16
#define MAX_FRAMES 16
#define MAX_ITEMS 256


typedef struct
{
	int32_t num_meshes;
	int32_t mesh_index[MAX_MESHES_PER_MODEL][MAX_FRAMES];
	vector3_t position[MAX_MESHES_PER_MODEL][MAX_FRAMES];
	vector3_t orientation[MAX_MESHES_PER_MODEL][MAX_FRAMES];
}model_t;

typedef struct
{
	uint8_t* name;
	uint32_t rotations;
	uint32_t frames;
	model_t model;
}item_t;

typedef struct
{
	uint32_t num_meshes;
	uint32_t num_items;
	mesh_t meshes[MAX_MESHES];
	item_t items[MAX_ITEMS];
}project_t;

void print_msg(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	putchar('\r');
	putchar('\n');
	va_end(args);
	fflush(stdout);
}


//JSON loading

int load_mesh(mesh_t* model, json_t* mesh)
{
	if (json_is_string(mesh))
	{
		if (mesh_load(model, json_string_value(mesh)))
		{
			print_msg("Failed to load model \"%s\"", json_string_value(mesh));
			return 1;
		}
		return 0;
	}
	print_msg("Error: Mesh path is not a string");
	return 1;
}

int load_vector(vector3_t* vector, json_t* array)
{
	int size = json_array_size(array);
	if (size != 3)
	{
		print_msg("Vector must have 3 components");
		return 1;
	}

	json_t* x = json_array_get(array, 0);
	json_t* y = json_array_get(array, 1);
	json_t* z = json_array_get(array, 2);

	if (!json_is_number(x) || !json_is_number(y) || !json_is_number(z))
	{
		print_msg("Vector components must be numeric");
		return 1;
	}
	vector->x = json_number_value(x);
	vector->y = json_number_value(y);
	vector->z = json_number_value(z);
	return 0;
}

json_t* load_optional_array(json_t* json)
{
	if (!json)return NULL;
	else if (json_is_array(json))
	{
		if (json_array_size(json) == 0)
		{
			print_msg("Empty array");
			return NULL;
		}
		json_incref(json);
		return json;
	}
	else
	{
		json_t* arr = json_array();
		json_array_append(arr, json);
		return arr;
	}
}

int load_model(model_t* model, json_t* json, int num_meshes, int num_frames)
{
	if (!json)
	{
		print_msg("Error: Property \"model\" not found");
		return 1;
	}

	json_t* arr = load_optional_array(json);
	model->num_meshes = json_array_size(arr);
	for (int i = 0; i < model->num_meshes; i++)
	{
		json_t* elem = json_array_get(arr, i);
		if (model == NULL || !json_is_object(elem))
		{
			print_msg("Property \"model\" is not an object");
			return 1;
		}

		//Load mesh index
		json_t* mesh = json_object_get(elem, "mesh_index");
		if (mesh == NULL)
		{
			print_msg("Error: Property \"mesh_index\" not found");
			return 1;
		}
		json_t* mesh_arr = load_optional_array(mesh);
		if (mesh_arr == NULL || (json_array_size(mesh_arr) != 1 && json_array_size(mesh_arr) != num_frames))
		{
			print_msg("Error: Number of elements in \"mesh_index\"(%d) does not match number of frames(%d)", json_array_size(mesh_arr), num_frames);
			return 1;
		}

		for (int j = 0; j < json_array_size(mesh_arr); j++)
		{
			json_t* mesh_index = json_array_get(mesh_arr, j);
			if (!json_is_integer(mesh_index))
			{
				print_msg("Error: Property \"mesh_index\" is not an integer");
				return 1;
			}
			model->mesh_index[i][j] = json_integer_value(mesh_index);
			if (model->mesh_index[i][j] >= num_meshes || model->mesh_index[i][j] < -1)
			{
				print_msg("Mesh index %d is out of bounds", model->mesh_index[i][j]);
				return 1;
			}
		}
		if (json_array_size(mesh_arr) < num_frames)
		{
			for (int j = 0; j < num_frames; j++)model->mesh_index[i][j] = model->mesh_index[i][0];
		}
		json_decref(mesh_arr);

		//Load position
		json_t* position = json_object_get(elem, "position");
		if (position == NULL || !json_is_array(position))
		{
			print_msg("Error: Property \"position\" not found or is not an array");
			return 1;
		}

		if (json_array_size(position) == 3)
		{
			vector3_t vec;
			if (load_vector(&vec, position))return 1;
			for (int j = 0; j < num_frames; j++)model->position[i][j] = vec;
		}
		else if (json_array_size(position) == num_frames)
		{
			for (int j = 0; j < num_frames; j++)
			{
				if (load_vector(&(model->position[i][j]), json_array_get(position, j)))return 1;
			}
		}
		else
		{
			print_msg("Error: Number of elements in \"position\"(%d) does not match number of frames(%d)", json_array_size(position), num_frames);
			return 1;
		}
		//Load orientation
		json_t* orientation = json_object_get(elem, "orientation");
		if (orientation == NULL || !json_is_array(orientation))
		{
			print_msg("Error: Property \"orientation\" not found or is not an array");
			return 1;
		}

		if (json_array_size(orientation) == 3)
		{
			vector3_t vec;
			if (load_vector(&vec, orientation))return 1;
			for (int j = 0; j < num_frames; j++)model->orientation[i][j] = vec;
		}
		else if (json_array_size(orientation) == num_frames)
		{
			for (int j = 0; j < num_frames; j++)
			{
				if (load_vector(&(model->orientation[i][j]), json_array_get(orientation, j)))return 1;
			}
		}
		else
		{
			print_msg("Error: Number of elements in \"orientation\"(%d) does not match number of frames(%d)", json_array_size(orientation), num_frames);
			return 1;
		}
	}
	json_decref(arr);
	return 0;
}

int load_int(uint32_t* out, json_t* json, const char* property)
{
	if (json != NULL && json_is_integer(json))*out = json_integer_value(json);
	else
	{
		print_msg("Error: Property \"%s\" not found or is not a integer", property);
		return 1;
	}
	return 0;
}

int load_lights(light_t* lights, int* lights_count, json_t* json)
{
	int num_lights = json_array_size(json);
	for (int i = 0; i < num_lights; i++)
	{
		json_t* light = json_array_get(json, i);
		assert(light != NULL);
		if (!json_is_object(light))
		{
			print_msg("Warning: Light array contains an element which is not an object-ignoring");
			continue;
		}

		json_t* type = json_object_get(light, "type");
		if (type == NULL || !json_is_string(type))
		{
			print_msg("Error: Property \"type\" not found or is not a string");
			return 1;
		}

		const char* type_value = json_string_value(type);
		if (strcmp(type_value, "diffuse") == 0)lights[i].type = LIGHT_DIFFUSE;
		else if (strcmp(type_value, "specular") == 0)lights[i].type = LIGHT_SPECULAR;
		else
		{
			print_msg("Unrecognized light type \"%s\"", type);
			free(lights);
		}

		json_t* shadow = json_object_get(light, "shadow");
		if (shadow == NULL || !json_is_boolean(shadow))
		{
			print_msg("Error: Property \"shadow\" not found or is not a boolean");
			return 1;
		}
		if (json_boolean_value(shadow))lights[i].shadow = 1;
		else lights[i].shadow = 0;

		json_t* direction = json_object_get(light, "direction");
		if (direction == NULL || !json_is_array(direction))
		{
			print_msg("Error: Property \"direction\" not found or is not a direction");
			return 1;
		}
		if (load_vector(&(lights[i].direction), direction))return 1;
		lights[i].direction = vector3_normalize(lights[i].direction);

		json_t* strength = json_object_get(light, "strength");
		if (strength == NULL || !json_is_number(strength))
		{
			print_msg("Error: Property \"strength\" not found or is not a number");
			return 1;
		}
		lights[i].intensity = json_number_value(strength);
	}
	*lights_count = num_lights;
	return 0;
}

int load_project(project_t* project, json_t* json)
{
	//Load meshes
	json_t* meshes = json_object_get(json, "meshes");
	if (meshes == NULL)print_msg("Error: Property \"meshes\" does not exist or is not an array");
	project->num_meshes = json_array_size(meshes);

	for (int i = 0; i < project->num_meshes; i++)
	{
		json_t* mesh = json_array_get(meshes, i);
		assert(mesh != NULL);
		if (load_mesh(project->meshes + i, mesh))
		{
			for (int j = 0; j < i; j++)mesh_destroy(project->meshes + i);
			return 1;
		}
	}

	//Load items
	json_t* items = json_object_get(json, "items");
	if (items == NULL || !json_is_array(items))print_msg("Error: Property \"items\" does not exist or is not an array");
	project->num_items = json_array_size(items);

	for (int i = 0; i < project->num_items; i++)
	{
		json_t* item = json_array_get(items, i);
		assert(item != NULL);
		if (!json_is_object(item))
		{
			print_msg("Error: Item array contains an element which is not an object");
			return 1;
		}

		json_t* name = json_object_get(item, "name");
		if (name == NULL || !json_is_string(name))
		{
			print_msg("Error: No property \"name\" found");
			return 1;
		}
		project->items[i].name = (uint8_t*)strdup(json_string_value(name));


		if (load_int(&(project->items[i].rotations), json_object_get(item, "rotations"), "rotations"))return 1;
		if (load_int(&(project->items[i].frames), json_object_get(item, "frames"), "frames"))return 1;

		//Load model
		if (load_model(&(project->items[i].model), json_object_get(item, "model"), project->num_meshes, project->items[i].frames))return 1;
	}
	return 0;
}


//Rendering

void project_add_model_to_context(project_t* project, context_t* context, model_t* model, int frame, int rotation)
{
	float offsets[8] = { 0,-1,0,-1.5,0,-1,0,-1.5 };
	for (int i = 0; i < model->num_meshes; i++)
	{
		if (model->mesh_index[i][frame] == -1)continue;
		vector3_t orientation = vector3_mult(model->orientation[i][frame], M_PI / 180.0);
		context_add_model(context, project->meshes + model->mesh_index[i][frame], transform(matrix_mult(rotate_y(orientation.x), matrix_mult(rotate_z(orientation.y), rotate_x(orientation.z))), vector3_add(model->position[i][frame], vector3(CLEARANCE_HEIGHT * offsets[2 * rotation] / 8.0, CLEARANCE_HEIGHT * offsets[2 * rotation + 1] / 8.0, 0))), 0);
	}
}

int project_export(project_t* project, context_t* context, json_t* sprites, const char* base_dir, const char* output_dir)
{
	json_t* images_json = json_array();

	image_t images[4 * MAX_FRAMES];

	for (int i = 0; i < project->num_items; i++)
	{
		printf("Rendering item %d\n", i);
		//Render item
		int base = 0;
		for (int frame = 0; frame < project->items[i].frames; frame++)
		{
			for (int j = 0; j < project->items[i].rotations; j++)
			{
				context_begin_render(context);
				project_add_model_to_context(project, context, &(project->items[i].model), frame, j);
				context_finalize_render(context);
				context_render_view(context, rotate_y(0.5 * j * M_PI), images + frame * project->items[i].rotations + j);
			}
			context_end_render(context);
		}


		for (int frame = 0; frame < project->items[i].frames; frame++)
			for (int angle = 0; angle < project->items[i].rotations; angle++)
			{
				char final_filename[512];
				char relative_filename[512];
				if (project->items[i].frames > 1)snprintf(relative_filename, 512, "%s%s_%d_%d.png", output_dir, project->items[i].name, frame, angle + 1);
				else snprintf(relative_filename, 512, "%s%s_%d.png", output_dir, project->items[i].name, angle + 1);
				snprintf(final_filename, 512, "%s%s", base_dir, relative_filename);

				FILE* file = fopen(final_filename, "wb");
				if (file == NULL)
				{
					printf("Error: could not open %s for writing\n", final_filename);
					exit(1);
				}
				int index = frame * project->items[i].rotations + angle;
				image_crop(&images[index]);
				image_write_png(&images[index], file);
				fclose(file);

				json_t* sprite_entry = json_object();
				json_object_set(sprite_entry, "path", json_string(relative_filename));
				json_object_set(sprite_entry, "x", json_integer(images[index].x_offset));
				json_object_set(sprite_entry, "y", json_integer(images[index].y_offset));
				json_object_set(sprite_entry, "palette", json_string("keep"));
				json_array_append(sprites, sprite_entry);

				printf("%s (%d %d)\n", final_filename, images[index].x_offset, images[index].y_offset);
				image_destroy(images + index);
			}
	}
	return 0;
}








int main(int argc, char* argv[])
{
	project_t project;

	json_error_t error;
	json_t* project_json = json_load_file(argv[1], 0, &error);
	if (project_json == NULL)
	{
		print_msg("Error: %s at line %d column %d", error.text, error.line, error.column);
		return 1;
	}

	const char* base_dir = NULL;
	json_t* json_base_dir = json_object_get(project_json, "base_directory");
	if (json_base_dir != NULL && json_is_string(json_base_dir))base_dir = json_string_value(json_base_dir);
	else printf("Error: No property \"base_directory\" found\n");

	const char* sprite_dir = NULL;
	json_t* json_sprite_dir = json_object_get(project_json, "sprite_directory");
	if (json_sprite_dir != NULL && json_is_string(json_sprite_dir))sprite_dir = json_string_value(json_sprite_dir);
	else printf("Error: No property \"sprite_directory\" found\n");

	const char* spritefile_in = NULL;
	json_t* json_spritefile_in = json_object_get(project_json, "spritefile_in");
	if (json_spritefile_in != NULL && json_is_string(json_spritefile_in))spritefile_in = json_string_value(json_spritefile_in);
	else printf("Error: No property \"spritefile_in\" found\n");

	const char* spritefile_out = NULL;
	json_t* json_spritefile_out = json_object_get(project_json, "spritefile_out");
	if (json_spritefile_out != NULL && json_is_string(json_spritefile_out))spritefile_out = json_string_value(json_spritefile_out);
	else printf("Error: No property \"spritefile_out\" found\n");

	int num_lights = 9;

	light_t lights[16] = {
		{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.25},
		{LIGHT_DIFFUSE,0,vector3_normalize(vector3(1.0,0.3,0.0)),0.32},
		{LIGHT_SPECULAR,0,vector3_normalize(vector3(1,1,-1)),1.0},
		{LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,0.65,-1)),0.8},
		{LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),0.174},
		{LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,0.0)),0.15},
		{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,1.0,1.0)),0.2},
		{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.65,0.816,-0.65000000)),0.25},
		{LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,-1.0)),0.25},
		{0,0,{0,0,0},0},
		{0,0,{0,0,0},0},
		{0,0,{0,0,0},0},
		{0,0,{0,0,0},0},
		{0,0,{0,0,0},0},
		{0,0,{0,0,0},0},
		{0,0,{0,0,0},0}
	};

	json_t* light_array = json_object_get(project_json, "lights");
	if (light_array != NULL)
	{
		if (!json_is_array(light_array))
		{
			print_msg("Error: Property \"lights\" is not an array");
			return 1;
		}
		if (load_lights(lights, &num_lights, light_array))return 1;
	}

	if (load_project(&project, project_json))return 1;


	char full_path[256];
	snprintf(full_path, 256, "%s%s", base_dir, spritefile_in);
	json_t* sprites = json_load_file(full_path, 0, &error);
	if (sprites == NULL)
	{
		printf("Error: %s in file %s line %d column %d\n", error.text, error.source, error.line, error.column);
		return 1;
	}

	context_t context;
	context_init(&context, lights, num_lights, palette_rct2(), TILE_SIZE);

	if (project_export(&project, &context, sprites, base_dir, sprite_dir))return 1;

	snprintf(full_path, 256, "%s%s", base_dir, spritefile_out);
	json_dump_file(sprites, full_path, JSON_INDENT(4));
	context_destroy(&context);
	return 0;
}





