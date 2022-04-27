#include "render.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "matrix.h"

float render_current_transform[3][3] = {
	{1, 0, 0},
	{0, 1, 0},
	{0, 0, 1}
};

float render_current_color[4] = {
		0, 0, 0, 1
};

#define TRANSFORM_STACK_SIZE 16

static int transform_stack_size = 0;
static float transform_stack[TRANSFORM_STACK_SIZE][3][3];

void render_set_color(float r, float g, float b, float a) {
	render_current_color[0] = r;
	render_current_color[1] = g;
	render_current_color[2] = b;
	render_current_color[3] = a;
}

void render_push_matrix() {
	assert(transform_stack_size < TRANSFORM_STACK_SIZE && "Transform stack overflow");
	memcpy(transform_stack[transform_stack_size], render_current_transform, sizeof(render_current_transform));
	transform_stack_size++;
}

void render_pop_matrix() {
	assert(transform_stack_size > 0 && "Transform stack empty");
	memcpy(render_current_transform, transform_stack[transform_stack_size - 1], sizeof(render_current_transform));
	transform_stack_size--;
}

void render_load_identity() {
	memset(render_current_transform, 0, sizeof(render_current_transform));
	render_current_transform[0][0] = render_current_transform[1][1] = render_current_transform[2][2] = 1;
}

void render_transform(float transform[3][3]) {
	matrix_multiply(render_current_transform, transform, render_current_transform);
}

void render_translate(float x, float y) {
	matrix_translate_multiply(x, y, render_current_transform, render_current_transform);
}

void render_rotate(float rotate) {
	matrix_rotate_multiply(rotate, render_current_transform, render_current_transform);
}

void render_scale(float x, float y) {
	matrix_scale_multiply(x, y, render_current_transform, render_current_transform);
}

void render_ortho(float left, float right, float bottom, float top) {
	float transform[3][3] = {
			{2 / (right - left), 0, 0},
			{0, 2 / (top - bottom), 0},
			{-(right + left) / (right - left), -(top + bottom) / (top - bottom), 1}
	};
	render_transform(transform);
}
