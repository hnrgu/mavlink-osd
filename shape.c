#include "shape.h"

#include <stdlib.h>
#include <string.h>

#include "render.h"
#include "shader.h"

#define DEFAULT_SIZE 256

static struct program solid;
static int solid_u_color;
static int solid_u_transform;
static int solid_a_pos;

void shape_init() {
	SHADER_LOAD(&solid, shaders_solid_vert, shaders_solid_frag);

	solid_u_color = shader_get_uniform(&solid, "u_color");
	solid_u_transform = shader_get_uniform(&solid, "u_transform");
	solid_a_pos = shader_get_attrib(&solid, "a_pos");
}

void shape_new(struct shape_context *ctx) {
	memset(ctx, 0, sizeof(struct shape_context));
	glGenBuffers(1, &ctx->buffer_id);
	ctx->buffer_size = DEFAULT_SIZE;
	ctx->buffer = malloc(ctx->buffer_size * sizeof(float) * 2);
}

void shape_begin(struct shape_context *ctx, GLenum mode) {
	ctx->num_vertices = 0;
	ctx->mode = mode;
}

void shape_vertex(struct shape_context *ctx, float x, float y) {
	if(ctx->num_vertices >= ctx->buffer_size) {
		ctx->buffer_size *= 2;
		ctx->buffer = realloc(ctx->buffer, ctx->buffer_size * sizeof(float) * 2);
	}
	ctx->buffer[ctx->num_vertices * 2] = x;
	ctx->buffer[ctx->num_vertices * 2 + 1] = y;
	ctx->num_vertices++;
}

void shape_end(struct shape_context *ctx) {
	glBindBuffer(GL_ARRAY_BUFFER, ctx->buffer_id);
	glBufferData(GL_ARRAY_BUFFER, ctx->num_vertices * sizeof(float) * 2, ctx->buffer, GL_DYNAMIC_DRAW);
}

void shape_draw(struct shape_context *ctx) {
	shader_enable(&solid);

	glBindBuffer(GL_ARRAY_BUFFER, ctx->buffer_id);
	glVertexAttribPointer(solid_a_pos, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(solid_a_pos);

	glUniform4fv(solid_u_color, 1, render_current_color);
	glUniformMatrix3fv(solid_u_transform, 1, GL_FALSE, &render_current_transform[0][0]);
	glDrawArrays(ctx->mode, 0, ctx->num_vertices);
}
