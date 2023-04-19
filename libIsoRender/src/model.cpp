#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4244)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <png.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "model.h"
#include "palette.h"

void texture_init(texture_t* texture, uint16_t width, uint16_t height)
{
    texture->width = width;
    texture->height = height;
    texture->pixels = (vector3_t*)malloc(width * height * sizeof(color_t));
}

int texture_load_png(texture_t* texture, const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp)
    {
        return 1;
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
    {
        fclose(fp);
        return 2;
    }
    png_infop info = png_create_info_struct(png);
    if (!info)
    {
        fclose(fp);
        return 3;
    }
    if (setjmp(png_jmpbuf(png))) abort();//TODO Not sure what this does but I don't think it's what I want

    png_init_io(png, fp);
    png_read_info(png, info);

    texture->width = png_get_image_width(png, info);
    texture->height = png_get_image_height(png, info);

    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
    if (bit_depth == 16)png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * texture->height);
    for (int y = 0; y < texture->height; y++)
    {
        row_pointers[y] = (png_byte*)malloc(sizeof(png_byte) * png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    texture->pixels = (vector3_t*)malloc(texture->width * texture->height * sizeof(vector3_t));
    for (int y = 0; y < texture->height; y++)
        for (int x = 0; x < texture->width; x++)
        {
            color_t color = { (uint8_t)row_pointers[y][4 * x],(uint8_t)row_pointers[y][4 * x + 1],(uint8_t)row_pointers[y][4 * x + 2] };
            texture->pixels[x + y * texture->width] = vector_from_color(color);
        }
    for (int y = 0; y < texture->height; y++)
    {
        free(row_pointers[y]);
    }
    free(row_pointers);
    fclose(fp);
    return 0;
}

float wrap_coord(float coord)
{
    return fmax(0.0, fmin(1.0, coord - floor(coord)));
}

vector3_t texture_sample(texture_t* texture, vector2_t coord)
{
    uint16_t tex_x = (uint32_t)(texture->width * wrap_coord(coord.x));
    uint16_t tex_y = (uint32_t)(texture->height * wrap_coord(coord.y));
    if (tex_x == texture->width)tex_x = 0;
    if (tex_y == texture->height)tex_y = 0;
    if (tex_x >= texture->width)printf("X: %d Width %d\n", tex_x, texture->width);
    if (tex_y >= texture->height)printf("Y: %d Height %d\n", tex_y, texture->height);
    assert(tex_x < texture->width && tex_y < texture->height);
    return texture->pixels[tex_y * texture->width + tex_x];
}

void texture_destroy(texture_t* texture)
{
    free(texture->pixels);
}


material_t material_color(vector3_t color, vector3_t specular_color, float specular_exponent, uint8_t flags)
{
    material_t material;
    material.flags = flags & (~MATERIAL_HAS_TEXTURE);
    material.color = color;
    material.specular_color = specular_color;
    material.specular_exponent = specular_exponent;
    return material;
}

material_t material_texture(const char* filename, vector3_t specular_color, float specular_exponent, uint8_t flags)
{
    material_t material;
    if (texture_load_png(&(material.texture), filename))
    {
        printf("Failed to load %s\n", filename);
        material.flags = flags & (~MATERIAL_HAS_TEXTURE);
        material.color = vector3(0.25, 0.25, 0.25);
    }
    else material.flags = flags | MATERIAL_HAS_TEXTURE;
    material.specular_color = specular_color;
    material.specular_exponent = specular_exponent;
    return material;
}

int mesh_load_transform(mesh_t* output, const char* filename, matrix_t matrix)
{
    int import_flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals;

    //Check if the input transformation mirrors the scene
    double determinant = matrix_determinant(matrix);
    if (fabs(fabs(determinant) - 1.0) > 0.001)printf("Warning: transformation matrix is not orthonormal\n");
    if (determinant < 0)import_flags |= aiProcess_FlipWindingOrder;

    const struct aiScene* scene = aiImportFile(filename, import_flags);

    if (!scene)
    {
        printf("Importing file \"%s\" failed with error: %s\n", filename, aiGetErrorString());
        return 1;
    }

    output->num_materials = scene->mNumMaterials;
    output->materials = (material_t*)malloc(scene->mNumMaterials * sizeof(material_t));

    for (size_t i = 0; i < scene->mNumMaterials; i++)
    {
        output->materials[i].flags = 0;
        output->materials[i].region = 0;
        output->materials[i].color = vector3(0.5, 0.5, 0.5);
        output->materials[i].specular_color = vector3(0.5, 0.5, 0.5);
        output->materials[i].specular_exponent = 50;

        const struct aiMaterial* mat = scene->mMaterials[i];

        //Check for remappable materials
        struct aiString name;
        if (aiGetMaterialString(mat, AI_MATKEY_NAME, &name) == AI_SUCCESS)
        {
            if (strstr(name.data, "Remap1") != NULL)
            {
                output->materials[i].flags |= MATERIAL_IS_REMAPPABLE;
                output->materials[i].region = 1;
            }
            else if (strstr(name.data, "Remap2") != NULL)
            {
                output->materials[i].flags |= MATERIAL_IS_REMAPPABLE;
                output->materials[i].region = 2;
            }
            else if (strstr(name.data, "Remap3") != NULL)
            {
                output->materials[i].flags |= MATERIAL_IS_REMAPPABLE;
                output->materials[i].region = 3;
            }
            else if (strstr(name.data, "Greyscale") != NULL)
            {
                output->materials[i].region = 4;
            }
            else if (strstr(name.data, "Peep") != NULL)output->materials[i].region = 5;
            if (strstr(name.data, "VisibleMask") != NULL)output->materials[i].flags |= MATERIAL_IS_VISIBLE_MASK;
            else if (strstr(name.data, "Mask") != NULL)output->materials[i].flags |= MATERIAL_IS_MASK;
            if (strstr(name.data, "NoAO") != NULL)output->materials[i].flags |= MATERIAL_NO_AO;
            if (strstr(name.data, "Edge") != NULL)output->materials[i].flags |= MATERIAL_BACKGROUND_AA;
            if (strstr(name.data, "DarkEdge") != NULL)output->materials[i].flags |= MATERIAL_BACKGROUND_AA_DARK;
            if (strstr(name.data, "NoBleed") != NULL)output->materials[i].flags |= MATERIAL_NO_BLEED;
        }

        aiColor4D diffuse;
        aiString texture_path;
        if (aiGetMaterialString(mat, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), &texture_path) == AI_SUCCESS)
        {
            output->materials[i].flags |= MATERIAL_HAS_TEXTURE;
            if (texture_load_png(&(output->materials[i].texture), texture_path.data))
            {
                printf("Failed to load texture \"%s\"\r\n", texture_path.data);
                free(output->vertices);
                free(output->normals);
                free(output->faces);
                free(output->materials);
                //TODO free any textures already loaded	
                aiReleaseImport(scene);
                return 1;
            }
            //printf("%s\n",texture_path.data);
        }
        else if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == AI_SUCCESS)
        {
            output->materials[i].color = vector3(diffuse.r, diffuse.g, diffuse.b);
            //printf("%f,%f,%f\n",diffuse.r,diffuse.g,diffuse.b);
        }

        aiColor4D specular;
        if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &specular) == AI_SUCCESS)
        {
            output->materials[i].specular_color = vector3(specular.r, specular.g, specular.b);
            float specular_strength;
            if (aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS_STRENGTH, &specular_strength, NULL) == AI_SUCCESS)
            {
                output->materials[i].specular_color = vector3_mult(output->materials[i].specular_color, specular_strength);
            }
            //printf("%f,%f,%f\n",specular.r,specular.g,specular.b);
        }

        float specular_exponent;
        if (aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &specular_exponent, NULL) == AI_SUCCESS)
        {
            output->materials[i].specular_exponent = specular_exponent;
            //printf("%d\n",output->materials[i].specular_hardness);
        }
    }
    //Count vertices and faces in scene
    output->num_vertices = 0;
    output->num_faces = 0;

    for (size_t j = 0; j < scene->mNumMeshes; j++)
    {
        const aiMesh* mesh = scene->mMeshes[j];
        assert(mesh->mNormals);
        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            if (mesh->mFaces[i].mNumIndices < 3)continue;
            assert(mesh->mFaces[i].mNumIndices == 3);
            output->num_faces++;
        }
        output->num_vertices += scene->mMeshes[j]->mNumVertices;
    }

    printf("Loading model with %d vertices and %d faces\n", output->num_vertices, output->num_faces);

    //TODO detect if UVs are not used and do not load them
    output->vertices = (vector3_t*)malloc(output->num_vertices * sizeof(vector3_t));
    output->normals = (vector3_t*)malloc(output->num_vertices * sizeof(vector3_t));
    output->uvs = (vector2_t*)malloc(output->num_vertices * sizeof(vector2_t));
    output->faces = (face_t*)malloc(output->num_faces * sizeof(face_t));

    size_t mesh_start_vertex = 0;
    size_t mesh_start_face = 0;

    for (size_t j = 0; j < scene->mNumMeshes; j++)
    {
        const struct aiMesh* mesh = scene->mMeshes[j];
        assert(mesh->mNormals);
        for (size_t i = 0; i < mesh->mNumVertices; i++)
        {
            output->vertices[mesh_start_vertex + i] = matrix_vector(matrix, vector3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
            output->normals[mesh_start_vertex + i] = matrix_vector(matrix, vector3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
            if (mesh->mTextureCoords[0])
            {
                output->uvs[mesh_start_vertex + i] = vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }
            else output->uvs[mesh_start_vertex + i] = vector2(0.0, 0.0);
        }

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            if (mesh->mFaces[i].mNumIndices < 3)continue;
            assert(mesh->mFaces[i].mNumIndices == 3);
            output->faces[mesh_start_face].material = mesh->mMaterialIndex;
            for (uint32_t j = 0; j < 3; j++)output->faces[mesh_start_face].indices[j] = mesh_start_vertex + mesh->mFaces[i].mIndices[j];
            mesh_start_face++;
        }
        mesh_start_vertex += mesh->mNumVertices;
    }
    aiReleaseImport(scene);
    return 0;
}

void mesh_destroy(mesh_t* mesh)
{
    for (uint32_t i = 0; i < mesh->num_materials; i++)
    {
        if (mesh->materials[i].flags & MATERIAL_HAS_TEXTURE)
        {
            texture_destroy(&(mesh->materials[i].texture));
        }
    }

    free(mesh->vertices);
    free(mesh->normals);
    free(mesh->faces);
    free(mesh->materials);
}


int mesh_load(mesh_t* output, const char* filename)
{
    return mesh_load_transform(output, filename, matrix_identity());
}

