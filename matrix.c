#include "matrix.h"

#include <math.h>
#include <string.h>

void matrix_multiply(const float m1[3][3], const float m2[3][3], float out[3][3]) {
	float result[3][3];
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			result[j][i] = 0;
			for (int k = 0; k < 3; ++k) {
				result[j][i] += m1[k][i] * m2[j][k];
			}
		}
	}
	memcpy(out, result, sizeof(result));
}

void matrix_identity(float transform[3][3]) {
	transform[0][0] = 1;
	transform[0][1] = 0;
	transform[0][2] = 0;
	transform[1][0] = 0;
	transform[1][1] = 1;
	transform[1][2] = 0;
	transform[2][0] = 0;
	transform[2][1] = 0;
	transform[2][2] = 1;
}

void matrix_translate(float x, float y, float transform[3][3]) {
	transform[0][0] = 1;
	transform[0][1] = 0;
	transform[0][2] = 0;
	transform[1][0] = 0;
	transform[1][1] = 1;
	transform[1][2] = 0;
	transform[2][0] = x;
	transform[2][1] = y;
	transform[2][2] = 1;
}

void matrix_rotate(float r, float transform[3][3]) {
	transform[0][0] = cosf(r);
	transform[0][1] = sinf(r);
	transform[0][2] = 0;
	transform[1][0] = -sinf(r);
	transform[1][1] = cosf(r);
	transform[1][2] = 0;
	transform[2][0] = 0;
	transform[2][1] = 0;
	transform[2][2] = 1;
}

void matrix_scale(float x, float y, float transform[3][3]) {
	transform[0][0] = x;
	transform[0][1] = 0;
	transform[0][2] = 0;
	transform[1][0] = 0;
	transform[1][1] = y;
	transform[1][2] = 0;
	transform[2][0] = 0;
	transform[2][1] = 0;
	transform[2][2] = 1;
}

void matrix_translate_multiply(float x, float y, const float transform[3][3], float out[3][3]) {
	float temp[3][3];
	matrix_translate(x, y, temp);
	matrix_multiply(transform, temp, out);
}

void matrix_rotate_multiply(float r, const float transform[3][3], float out[3][3]) {
	float temp[3][3];
	matrix_rotate(r, temp);
	matrix_multiply(transform, temp, out);
}

void matrix_scale_multiply(float x, float y, const float transform[3][3], float out[3][3]) {
	float temp[3][3];
	matrix_scale(x, y, temp);
	matrix_multiply(transform, temp, out);
}
