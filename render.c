#include "render.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <GLES2/gl2.h>

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

static int stencil_counter;

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

void render_stencil_reset() {
	if(stencil_counter >= 255) {
		glClear(GL_STENCIL_BUFFER_BIT);
		stencil_counter = 0;
	}
	stencil_counter ++;
}

void render_stencil_begin(int mode) {
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	if(mode == RENDER_STENCIL_ADD) {
		glStencilFunc(GL_ALWAYS, stencil_counter, 0xFF);
	} else if(mode == RENDER_STENCIL_SUB) {
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
	} else {
		assert(0 && "Unknown stencil mode");
	}
}

void render_stencil_end() {
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void render_stencil_mode(int mode) {
	if(mode == RENDER_STENCIL_DISABLE) {
		glDisable(GL_STENCIL_TEST);
	} else {
		glEnable(GL_STENCIL_TEST);
		if(mode == RENDER_STENCIL_INSIDE) {
			glStencilFunc(GL_EQUAL, stencil_counter, 0xFF);
		} else if(mode == RENDER_STENCIL_OUTSIDE){
			glStencilFunc(GL_NOTEQUAL, stencil_counter, 0xFF);
		} else {
			assert(0 && "Unknown stencil mode");
		}
	}
}

