#ifndef MATRIX_H
#define MATRIX_H

void matrix_multiply(const float m1[3][3], const float m2[3][3], float out[3][3]);

void matrix_identity(float transform[3][3]);

void matrix_translate(float x, float y, float transform[3][3]);

void matrix_rotate(float r, float transform[3][3]);

void matrix_scale(float x, float y, float transform[3][3]);

void matrix_translate_multiply(float x, float y, const float transform[3][3], float out[3][3]);

void matrix_rotate_multiply(float r, const float transform[3][3], float out[3][3]);

void matrix_scale_multiply(float x, float y, const float transform[3][3], float out[3][3]);

#endif
