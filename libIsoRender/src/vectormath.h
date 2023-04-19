#ifndef FIXED_H_INCLUDED
#define FIXED_H_INCLUDED
#include <stdint.h>

typedef struct
{
    float x, y, z;
}vector3_t;

typedef struct
{
    float x, y;
}vector2_t;

typedef struct
{
    float entries[9];
}matrix_t;

typedef struct
{
    matrix_t matrix;
    vector3_t translation;
}transform_t;

vector2_t vector2(float x, float y);
vector2_t vector2_add(vector2_t a, vector2_t b);
vector2_t vector2_sub(vector2_t a, vector2_t b);
vector2_t vector2_mult(vector2_t a, float b);
float vector2_norm(vector2_t);
float vector2_dot(vector2_t a, vector2_t b);

vector3_t vector3(float x, float y, float z);
vector3_t vector3_from_scalar(float a);
vector3_t vector3_add(vector3_t a, vector3_t b);
vector3_t vector3_sub(vector3_t a, vector3_t b);
vector3_t vector3_mult(vector3_t a, float b);
float vector3_norm(vector3_t a);
float vector3_dot(vector3_t a, vector3_t b);
vector3_t vector3_normalize(vector3_t a);
vector3_t vector3_cross(vector3_t a, vector3_t b);

#define MATRIX_INDEX(matrix,row,col) ((matrix).entries[3*(row)+(col)])
matrix_t matrix(float a, float b, float c, float d, float e, float f, float g, float h, float i);
matrix_t matrix_identity();
float matrix_determinant(matrix_t matrix);
matrix_t matrix_inverse(matrix_t matrix);
matrix_t matrix_transpose(matrix_t matrix);
matrix_t matrix_mult(matrix_t a, matrix_t b);
vector3_t matrix_vector(matrix_t matrix, vector3_t vector);

matrix_t rotate_x(float angle);
matrix_t rotate_y(float angle);
matrix_t rotate_z(float angle);

transform_t transform(matrix_t matrix, vector3_t vector);
transform_t transform_compose(transform_t transform, transform_t vector);
vector3_t transform_vector(transform_t transform, vector3_t vector);

#endif // FIXED_H_INCLUDED
