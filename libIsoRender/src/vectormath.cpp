#include "vectormath.h"
#include <math.h>

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif 

vector2_t vector2(float x, float y)
{
    vector2_t result = { x,y };
    return result;
}
vector2_t vector2_add(vector2_t a, vector2_t b)
{
    return vector2(a.x + b.x, a.y + b.y);
}
vector2_t vector2_sub(vector2_t a, vector2_t b)
{
    return vector2(a.x - b.x, a.y - b.y);
}
vector2_t vector2_mult(vector2_t a, float b)
{
    return vector2(a.x * b, a.y * b);
}
float vector2_norm(vector2_t a)
{
    return sqrt(a.x * a.x + a.y * a.y);
}

vector3_t vector3(float x, float y, float z)
{
    vector3_t result = { x,y,z };
    return result;
}
vector3_t vector3_from_scalar(float a)
{
    return vector3(a, a, a);
}
vector3_t vector3_add(vector3_t a, vector3_t b)
{
    return vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
vector3_t vector3_sub(vector3_t a, vector3_t b)
{
    return vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
vector3_t vector3_mult(vector3_t a, float b)
{
    return vector3(a.x * b, a.y * b, a.z * b);
}
float vector3_dot(vector3_t a, vector3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
vector3_t vector3_cross(vector3_t a, vector3_t b)
{
    return vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
float vector3_norm(vector3_t a)
{
    return sqrt(vector3_dot(a, a));
}
vector3_t vector3_normalize(vector3_t a)
{
    return vector3_mult(a, 1.0 / vector3_norm(a));
}

matrix_t matrix(float a, float b, float c, float d, float e, float f, float g, float h, float i)
{
    matrix_t mat = { {a,b,c,d,e,f,g,h,i} };
    return mat;
}
matrix_t matrix_identity()
{
    return matrix(1, 0, 0, 0, 1, 0, 0, 0, 1);
}
float matrix_determinant(matrix_t mat)
{
    return MATRIX_INDEX(mat, 0, 0) * (MATRIX_INDEX(mat, 1, 1) * MATRIX_INDEX(mat, 2, 2) - MATRIX_INDEX(mat, 1, 2) * MATRIX_INDEX(mat, 2, 1)) -
        MATRIX_INDEX(mat, 0, 1) * (MATRIX_INDEX(mat, 1, 0) * MATRIX_INDEX(mat, 2, 2) - MATRIX_INDEX(mat, 1, 2) * MATRIX_INDEX(mat, 2, 0)) +
        MATRIX_INDEX(mat, 0, 2) * (MATRIX_INDEX(mat, 1, 0) * MATRIX_INDEX(mat, 2, 1) - MATRIX_INDEX(mat, 1, 1) * MATRIX_INDEX(mat, 2, 0));
}
matrix_t matrix_inverse(matrix_t mat)
{
    float determinant = matrix_determinant(mat);
    return matrix(
        (MATRIX_INDEX(mat, 1, 1) * MATRIX_INDEX(mat, 2, 2) - MATRIX_INDEX(mat, 1, 2) * MATRIX_INDEX(mat, 2, 1)) / determinant,
        (MATRIX_INDEX(mat, 0, 2) * MATRIX_INDEX(mat, 2, 1) - MATRIX_INDEX(mat, 2, 2) * MATRIX_INDEX(mat, 0, 1)) / determinant,
        (MATRIX_INDEX(mat, 0, 1) * MATRIX_INDEX(mat, 1, 2) - MATRIX_INDEX(mat, 1, 1) * MATRIX_INDEX(mat, 0, 2)) / determinant,

        (MATRIX_INDEX(mat, 1, 2) * MATRIX_INDEX(mat, 2, 0) - MATRIX_INDEX(mat, 2, 2) * MATRIX_INDEX(mat, 1, 0)) / determinant,
        (MATRIX_INDEX(mat, 0, 0) * MATRIX_INDEX(mat, 2, 2) - MATRIX_INDEX(mat, 2, 0) * MATRIX_INDEX(mat, 0, 2)) / determinant,
        (MATRIX_INDEX(mat, 0, 2) * MATRIX_INDEX(mat, 1, 0) - MATRIX_INDEX(mat, 1, 2) * MATRIX_INDEX(mat, 0, 0)) / determinant,

        (MATRIX_INDEX(mat, 1, 0) * MATRIX_INDEX(mat, 2, 1) - MATRIX_INDEX(mat, 2, 0) * MATRIX_INDEX(mat, 1, 1)) / determinant,
        (MATRIX_INDEX(mat, 0, 1) * MATRIX_INDEX(mat, 2, 0) - MATRIX_INDEX(mat, 2, 1) * MATRIX_INDEX(mat, 0, 0)) / determinant,
        (MATRIX_INDEX(mat, 0, 0) * MATRIX_INDEX(mat, 1, 1) - MATRIX_INDEX(mat, 1, 0) * MATRIX_INDEX(mat, 0, 1)) / determinant
    );
}

matrix_t matrix_transpose(matrix_t mat)
{
    return matrix(MATRIX_INDEX(mat, 0, 0), MATRIX_INDEX(mat, 1, 0), MATRIX_INDEX(mat, 2, 0), MATRIX_INDEX(mat, 0, 1), MATRIX_INDEX(mat, 1, 1), MATRIX_INDEX(mat, 2, 1), MATRIX_INDEX(mat, 0, 2), MATRIX_INDEX(mat, 1, 2), MATRIX_INDEX(mat, 2, 2));
}
matrix_t matrix_mult(matrix_t a, matrix_t b)
{
    matrix_t result;
    for (int col = 0; col < 3; col++)
        for (int row = 0; row < 3; row++)
        {
            MATRIX_INDEX(result, row, col) = MATRIX_INDEX(a, row, 0) * MATRIX_INDEX(b, 0, col) + MATRIX_INDEX(a, row, 1) * MATRIX_INDEX(b, 1, col) + MATRIX_INDEX(a, row, 2) * MATRIX_INDEX(b, 2, col);
        }
    return result;
}

vector3_t matrix_vector(matrix_t mat, vector3_t vector)
{
    vector3_t result;
    result.x = MATRIX_INDEX(mat, 0, 0) * vector.x + MATRIX_INDEX(mat, 0, 1) * vector.y + MATRIX_INDEX(mat, 0, 2) * vector.z;
    result.y = MATRIX_INDEX(mat, 1, 0) * vector.x + MATRIX_INDEX(mat, 1, 1) * vector.y + MATRIX_INDEX(mat, 1, 2) * vector.z;
    result.z = MATRIX_INDEX(mat, 2, 0) * vector.x + MATRIX_INDEX(mat, 2, 1) * vector.y + MATRIX_INDEX(mat, 2, 2) * vector.z;
    return result;
}

transform_t transform(matrix_t mat, vector3_t vec)
{
    transform_t trans = { mat,vec };
    return trans;
}
transform_t transform_compose(transform_t a, transform_t b)
{
    transform_t result;
    result.matrix = matrix_mult(a.matrix, b.matrix);
    result.translation = vector3_add(matrix_vector(a.matrix, b.translation), a.translation);
    return result;
}
vector3_t transform_vector(transform_t transform, vector3_t vector)
{
    return vector3_add(matrix_vector(transform.matrix, vector), transform.translation);
}


matrix_t rotate_x(float theta)
{
    return matrix(1, 0, 0, 0, cos(theta), -sin(theta), 0, sin(theta), cos(theta));
}
matrix_t rotate_y(float theta)
{
    return matrix(cos(theta), 0, sin(theta), 0, 1, 0, -sin(theta), 0, cos(theta));
}
matrix_t rotate_z(float theta)
{
    return matrix(cos(theta), -sin(theta), 0, sin(theta), cos(theta), 0, 0, 0, 1);
}
