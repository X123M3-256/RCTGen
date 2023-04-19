#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <embree3/rtcore.h>
#include "model.h"
#include "renderer.h"

int main()
{
    context_t context;
    //vector3(0.65190909243,0.38733586252+0.2,-0.65190909243)
    /*
    LIM?
    light_t lights[9]={
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.25},//Bottom
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1.0,0.3,0.0)),0.32},//Back right
    {LIGHT_SPECULAR,0,vector3_normalize(vector3(1,0.63,-1)),1.0},//Main specular
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,0.63,-1)),0.8},//Main light
    {LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),1.0},//Top
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,0.0)),0.00},//Left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,1.0,1.0)),0.2},//Back left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.65,0.816,-0.65000000)),0.25},//Front right
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,-1.0)),0.35},//Front
    };
    */

    /*
    Original
    light_t lights[9]={
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.25},
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1.0,0.3,0.0)),0.32},
    {LIGHT_SPECULAR,0,vector3_normalize(vector3(1,1,-1)),1.0},
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,0.65,-1)),0.8},
    {LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),0.174},
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,0.0)),0.15},
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,1.0,1.0)),0.2},
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.65,0.816,-0.65000000)),0.25},
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,-1.0)),0.25},
    };
    */

    /*
    B&M match
    float diffuse=0.35;
    float specular=0.7;
    float exponent=5;
    light_t lights[9]={
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.0},//Bottom
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1.0,0.3,0.0)),0.1},//Back right
    {LIGHT_SPECULAR,0,vector3_normalize(vector3(1,0.63,-1)),1.0},//Main specular
    {LIGHT_DIFFUSE,1,vector3_normalize(vector3(1,0.63,-1)),0.8},//Main light
    {LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),0.09},//Top
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,0.0)),0.05},//Left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,1.0,1.0)),0.15},//Top left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.5,0,-1)),0.35},//Front right
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,-1.0,-1.0)),0.1},//Front bottom
    };
    */

    /*
    Intamin WIP
    light_t lights[9]={
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.0},//Bottom
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1.0,0.3,0.0)),0.0},//Back right
    {LIGHT_SPECULAR,0,vector3_normalize(vector3(1,1,-1)),1.0},//Main specular
    {LIGHT_DIFFUSE,1,vector3_normalize(vector3(1,1,-1)),0.8},//Main light
    {LIGHT_DIFFUSE,1,vector3(0.0,1.0,0.0),0.6},//Top
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,0.0)),0.0},//Left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,1.0,1.0)),0.0},//Top left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.5,0,-1)),0.0},//Front right
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,-1.0,-1.0)),0.0},//Front bottom
    };
    */


    light_t lights[9] = {
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.1},//Bottom
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,0.5,-1.0)),0.6},//Front right
    {LIGHT_SPECULAR,0,vector3_normalize(vector3(1,1.65,-1)),0.8},//Main spec
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,1.7,-1)),0.8},//Main diffuse
    {LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),0.45},//Top
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.85,1.0)),0.5},//Front left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.75,0.4,-1.0)),0.6},//Top right
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,0.25,0)),0.5},//Back left
    {LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,-0.5,0.0)),0.1},
    };

    context_init(&context, lights, 9, palette_rct2(), 3.3);

    matrix_t import_transform = { {1,0,0, 0,0,1, 0,1,0} };

    mesh_t mesh_base;
    if (mesh_load(&mesh_base, "test.obj"))
    {
        printf("Failed loading object\n");
        return 0;
    }

    context_begin_render(&context);
    context_add_model(&context, &mesh_base, transform(rotate_y(M_PI), vector3(0, 0, 0)), 0);
    context_finalize_render(&context);

    image_t images[4];
    for (int i = 0; i < 4; i++)
    {
        printf("Rendering image %d\r\n", i);
        fflush(stdout);
        context_render_view(&context, views[i], images + i);
    }
    context_end_render(&context);

    for (int i = 0; i < 4; i++)
    {
        char iamastupididiot[16];
        sprintf(iamastupididiot, "test_%d.png", i);
        FILE* f = fopen(iamastupididiot, "wb");
        image_write_png(images + i, f);
        fclose(f);
        image_destroy(images + i);
    }

    context_destroy(&context);

    return 0;
}





