#ifndef SHAPE_H
#define SHAPE_H

#include <GLES2/gl2.h>
#include <stdlib.h>

struct shape_context {
	GLuint buffer_id;
	GLenum mode;
	GLsizei num_vertices;
	GLsizei buffer_size;
	float *buffer;
};

void shape_init();
void shape_new(struct shape_context *ctx);
void shape_begin(struct shape_context *ctx, GLenum mode);
void shape_vertex(struct shape_context *ctx, float x, float y);
void shape_end(struct shape_context *ctx);
void shape_draw(struct shape_context *ctx);

#endif
