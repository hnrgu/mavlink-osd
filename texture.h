#ifndef TEXTURE_H
#define TEXTURE_H

#include <GLES2/gl2.h>

#include "resource.h"

struct texture {
	GLuint id;
	GLint internal_format;
	GLenum format;
	GLenum type;
};

#define TEXTURE_DATA_LOAD(texture, rgba_sym, width, height) { \
    RESOURCE_DECL(rgba_sym);                                  \
    texture_data(texture, width, height, &rgba_sym##_start);  \
}

void texture_init(struct texture *texture, GLint internal_format, GLenum format, GLenum type);
void texture_param(struct texture *texture, GLenum option, GLint value);
void texture_data(struct texture *texture, int width, int height, const void *data);
void texture_bind(struct texture *texture, GLuint texture_unit);
void texture_unbind();

#endif
