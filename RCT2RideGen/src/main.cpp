#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <jansson.h>
#include "project.h"

#define SQRT_2 1.4142135623731
#define SQRT1_2 0.707106781
#define SQRT_3 1.73205080757
#define SQRT_6 2.44948974278

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
            print_msg("Error: Number of elements in \"mesh_index\" (%d) does not match number of frames (%d)", json_array_size(mesh_arr), num_frames);
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
            print_msg("Error: Number of elements in \"position\" (%d) does not match number of frames (%d)", json_array_size(position), num_frames);
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
            print_msg("Error: Number of elements in \"orientation\" (%d) does not match number of frames (%d)", json_array_size(orientation), num_frames);
            return 1;
        }
    }
    json_decref(arr);
    return 0;
}

int load_configuration(uint8_t* configuration, json_t* json)
{
    if (json == NULL || !json_is_object(json))
    {
        print_msg("Error: Property \"configuration\" not found or is not a integer");
        return 1;
    }
    memset(configuration, 0xFF, 5);

    json_t* default_car = json_object_get(json, "default");
    if (default_car != NULL && json_is_integer(default_car))configuration[CAR_INDEX_DEFAULT] = json_integer_value(default_car);
    else
    {
        print_msg("Error: Property \"default\" not found or is not a integer");
        return 1;
    }

    json_t* front_car = json_object_get(json, "front");
    if (front_car != NULL && json_is_integer(front_car))configuration[CAR_INDEX_FRONT] = json_integer_value(front_car);

    json_t* second_car = json_object_get(json, "second");
    if (second_car != NULL && json_is_integer(second_car))configuration[CAR_INDEX_SECOND] = json_integer_value(second_car);

    json_t* third_car = json_object_get(json, "third");
    if (third_car != NULL && json_is_integer(third_car))configuration[CAR_INDEX_THIRD] = json_integer_value(third_car);

    json_t* rear_car = json_object_get(json, "rear");
    if (rear_car != NULL && json_is_integer(rear_car))configuration[CAR_INDEX_REAR] = json_integer_value(rear_car);

    return 0;
}

int load_flags(uint32_t* out, json_t* json, const char** strings, int n, const char* property, const char* item)
{
    if (json == NULL || !json_is_array(json))
    {
        print_msg("Error: Property \"%s\" not found or is not an array", property);
        return 1;
    }

    //Load sprites
    uint32_t flags = 0;
    for (int i = 0; i < json_array_size(json); i++)
    {
        json_t* flag_name = json_array_get(json, i);
        assert(flag_name != NULL);
        if (!json_is_string(flag_name))
        {
            print_msg("Error: Array \"sprites\" contains non-string value");
            return 1;
        }
        int j = 0;
        for (int j = 0; j < n; j++)
        {
            if (strcmp(json_string_value(flag_name), strings[j]) == 0)
            {
                flags |= 1 << j;
                break;
            }
        }
        if (j == n)
        {
            print_msg("Error: Unrecognized %s \"%s\"", item, json_string_value(flag_name));
            return 1;
        }
    }
    *out = flags;
    return 0;
}

int load_enum(uint32_t* out, json_t* json, const char** strings, int n, const char* property, const char* item)
{
    if (json == NULL || !json_is_string(json))
    {
        print_msg("Error: Property \"%s\" not found or is not a string", property);
        return 1;
    }

    for (int j = 0; j < n; j++)
    {
        if (strcmp(json_string_value(json), strings[j]) == 0)
        {
            *out = j;
            return 0;
        }
    }
    print_msg("Error: Unrecognized %s \"%s\"", item, json_string_value(json));
    return 1;
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

int load_colors(project_t* project, json_t* json)
{

    if (json == NULL || !json_is_array(json))
    {
        print_msg("Error: Property \"default_colors\" not found or is not an array");
        return 1;
    }

    project->num_colors = json_array_size(json);
    for (int i = 0; i < project->num_colors; i++)
    {
        json_t* colors = json_array_get(json, i);

        if (colors == NULL || !json_is_array(colors))
        {
            print_msg("Error: Property \"default_colors\" contains element which is not an array");
            return 1;
        }

        //TODO validate that correct number of colors has been supplied
        for (int j = 0; j < json_array_size(colors) && j < 3; j++)
        {
            uint32_t color;
            if (load_enum(&color, json_array_get(colors, j), color_names, NUM_COLORS, "default_colors", "color"))return 1;
            project->colors[i][j] = (uint8_t)color;
        }
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
            print_msg("Warning: Light array contains an element which is not an object - ignoring");
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
    json_t* id = json_object_get(json, "id");
    if (id == NULL || !json_is_string(id))
    {
        print_msg("Error: No property \"id\" found");
        return 1;
    }
    project->id = (uint8_t*)strdup(json_string_value(id));


    json_t* original_id = json_object_get(json, "original_id");
    if (original_id != NULL && !json_is_string(original_id))
    {
        print_msg("Error: Property \"original_id\" is not a string");
        return 1;
    }
    if (original_id != NULL)
    {
        printf("Original ID %s\n", json_string_value(original_id));
        project->original_id = (uint8_t*)strdup(json_string_value(original_id));
    }
    else project->original_id = NULL;

    json_t* name = json_object_get(json, "name");
    if (name == NULL || !json_is_string(name))
    {
        print_msg("Error: No property \"name\" found");
        return 1;
    }
    project->name = (uint8_t*)strdup(json_string_value(name));

    json_t* description = json_object_get(json, "description");
    if (description == NULL || !json_is_string(description))
    {
        print_msg("Error: No property \"description\" found");
        return 1;
    }
    project->description = (uint8_t*)strdup(json_string_value(description));

    json_t* capacity = json_object_get(json, "capacity");
    if (capacity == NULL || !json_is_string(capacity))
    {
        print_msg("Error: No property \"capacity\" found");
        return 1;
    }
    project->capacity = (uint8_t*)strdup(json_string_value(capacity));

    project->author = NULL;
    json_t* author_json = json_object_get(json, "author");
    if (author_json != NULL)
    {
        if (!json_is_string(author_json))
        {
            print_msg("Error: Property \"author\" is not a string");
            return 1;
        }
        else project->author = (uint8_t*)strdup(json_string_value(author_json));
    }

    project->version = (uint8_t*)"1.0";
    json_t* version_json = json_object_get(json, "version");
    if (version_json != NULL)
    {
        if (!json_is_string(version_json))
        {
            print_msg("Error: Property \"version\" is not a string");
            return 1;
        }
        else project->version = (uint8_t*)strdup(json_string_value(version_json));
    }


    json_t* preview = json_object_get(json, "preview");
    if (preview == NULL)
    {
        image_new(&(project->preview), 1, 1, 0, 0, 0);
    }
    else
    {
        if (!json_is_string(preview))
        {
            print_msg("Error: Property \"preview\" is not a string");
            return 1;
        }

        FILE* file = fopen(json_string_value(preview), "rb");
        if (image_read_png(&(project->preview), file))
        {
            print_msg("Error: Unable to open image file %s", json_string_value(preview));
            return 1;
        }
        fclose(file);
    }

    //TODO check if track type is valid
    json_t* ride_type = json_object_get(json, "ride_type");
    if (ride_type != NULL && json_is_string(ride_type))project->ride_type = (uint8_t*)strdup(json_string_value(ride_type));
    else
    {
        print_msg("Error: Property \"ride_type\" not found or is not a string");
        return 1;
    }

    json_t* flags = json_object_get(json, "flags");
    project->flags = 0;
    if (flags && load_flags(&(project->flags), flags, flag_names, NUM_FLAGS, "flags", "flag"))return 1;

    if (load_flags(&(project->sprite_flags), json_object_get(json, "sprites"), sprite_group_names, NUM_SPRITE_GROUPS, "sprites", "sprite group"))return 1;
    if (project->sprite_flags & SPRITE_BANKING)
    {
        project->sprite_flags |= SPRITE_DIAGONAL_BANK_TRANSITION;
        if (project->sprite_flags & SPRITE_GENTLE_SLOPE)project->sprite_flags |= SPRITE_SLOPE_BANK_TRANSITION;
        if (project->sprite_flags & SPRITE_SLOPED_BANKED_TURN)project->sprite_flags |= SPRITE_SLOPED_BANK_TRANSITION | SPRITE_BANKED_SLOPE_TRANSITION;
    }

    if (load_int(&(project->zero_cars), json_object_get(json, "zero_cars"), "zero_cars"))return 1;
    if (load_int(&(project->tab_car), json_object_get(json, "preview_tab_car"), "preview_tab_car"))return 1;
    if (load_int(&(project->build_menu_priority), json_object_get(json, "build_menu_priority"), "build_menu_priority"))return 1;
    if (load_enum(&(project->running_sound), json_object_get(json, "running_sound"), running_sounds, NUM_RUNNING_SOUNDS, "running_sound", "running sound"))return 1;
    if (load_enum(&(project->secondary_sound), json_object_get(json, "secondary_sound"), secondary_sounds, NUM_RUNNING_SOUNDS, "secondary_sound", "running sound"))return 1;
    if (load_int(&(project->min_cars_per_train), json_object_get(json, "min_cars_per_train"), "min_cars_per_train"))return 1;
    if (load_int(&(project->max_cars_per_train), json_object_get(json, "max_cars_per_train"), "max_cars_per_train"))return 1;
    if (load_configuration(project->configuration, json_object_get(json, "configuration")))return 1;

    if (load_colors(project, json_object_get(json, "default_colors")))return 1;

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


    //Load vehicles
    json_t* vehicles = json_object_get(json, "vehicles");
    if (vehicles == NULL || !json_is_array(vehicles))print_msg("Error: Property \"vehicles\" does not exist or is not an array");
    project->num_sprites = 3;
    project->num_vehicles = json_array_size(vehicles);

    for (int i = 0; i < project->num_vehicles; i++)
    {
        json_t* vehicle = json_array_get(vehicles, i);
        assert(vehicle != NULL);
        if (!json_is_object(vehicle))
        {
            print_msg("Error: Vehicle array contains an element which is not an object");
            return 1;
        }

        json_t* spacing = json_object_get(vehicle, "spacing");
        if (spacing != NULL && json_is_number(spacing))project->vehicles[i].spacing = json_number_value(spacing);
        else
        {
            print_msg("Error: Property \"spacing\" not found or is not a number");
            return 1;
        }

        if (load_int(&(project->vehicles[i].mass), json_object_get(vehicle, "mass"), "mass"))return 1;
        if (load_int(&(project->vehicles[i].draw_order), json_object_get(vehicle, "draw_order"), "draw_order"))return 1;
        if (load_flags(&(project->vehicles[i].flags), json_object_get(vehicle, "flags"), vehicle_flag_names, NUM_VEHICLE_FLAGS, "flags", "flag"))return 1;

        int num_frames = project->vehicles[i].flags & VEHICLE_RESTRAINT_ANIMATION ? 4 : 1;

        //Load car model
        if (load_model(&(project->vehicles[i].model), json_object_get(vehicle, "model"), project->num_meshes, num_frames))return 1;

        //Load rider models
        json_t* riders = json_object_get(vehicle, "riders");
        if (riders != NULL && json_is_array(riders))
        {
            json_t* num_riders = json_object_get(vehicle, "capacity");
            if (num_riders == NULL || !json_is_integer(num_riders))
            {
                print_msg("Error: Property \"capacity\" not found or is not an integer");
                return 1;
            }
            project->vehicles[i].num_riders = json_integer_value(num_riders);


            project->vehicles[i].num_rider_models = json_array_size(riders);
            for (int j = 0; j < project->vehicles[i].num_rider_models; j++)
            {
                if (load_model(project->vehicles[i].riders + j, json_array_get(riders, j), project->num_meshes, num_frames))return 1;

            }

        }
        else if (riders != NULL && !json_is_array(riders))
        {
            print_msg("Error: Property \"riders\" is not an array");
            return 1;
        }
        else
        {
            project->vehicles[i].num_riders = 0;
            project->vehicles[i].num_rider_models = 0;
        }

        project->vehicles[i].num_sprites = count_sprites_from_flags(project->sprite_flags, project->vehicles[i].flags);
        project->num_sprites += project->vehicles[i].num_sprites;
    }

    project->category = 3;
    return 0;
}

int main(int argc, char** argv)
{
    project_t project;

    const char* filename = NULL;
    int mode = 0;

    if (argc == 3)
    {
        if (strcmp("--test", argv[1]) == 0)mode = 2;
        else if (strcmp("--skip-render", argv[1]) == 0)mode = 1;
        else
        {
            print_msg("Error: Unrecognized option %s", argv[1]);
            return 1;
        }
        filename = argv[2];
    }
    else if (argc == 2)
    {
        filename = argv[1];
    }
    else
    {
        print_msg("Usage: makevehicle <file>");
        return 1;
    }


    json_error_t error;
    json_t* project_json = json_load_file(filename, 0, &error);
    if (project_json == NULL)
    {
        print_msg("Error: %s at line %d column %d", error.text, error.line, error.column);
        return 1;
    }

    const char* output_directory = ".";
    json_t* output_directory_json = json_object_get(project_json, "output_directory");
    if (output_directory_json != NULL)
    {
        if (!json_is_string(output_directory_json))
        {
            print_msg("Error: Property \"output_directory\" is not a string");
            return 1;
        }
        else output_directory = json_string_value(output_directory_json);

    }

    int num_lights = 9;

    light_t lights[16] = {
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.1},//Bottom
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,0.5,-1.0)),0.8},//Front right
    {LIGHT_SPECULAR,1,vector3_normalize(vector3(1,1.65,-1)),0.5},//Main spec
    {LIGHT_DIFFUSE,1,vector3_normalize(vector3(1,1.7,-1)),0.8},//Main diffuse
    {LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),0.45},//Top
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.85,1.0)),0.475},//Front left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.75,0.4,-1.0)),0.6},//Top right
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,0.25,0)),0.5},//Back left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,-0.5,0)),0.1},//Back left
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

    context_t context;
    context_init(&context, lights, num_lights, palette_rct2(), (mode == 2) ? 0.125 * TILE_SIZE : TILE_SIZE);
    if (mode == 2)
    {
        if (project_export_test(&project, &context))return 1;
    }
    else
    {
        if (project_export(&project, &context, output_directory, mode == 1))return 1;
    }
    context_destroy(&context);
    return 0;
}
