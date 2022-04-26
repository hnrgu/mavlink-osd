#include "texture.h"

void texture_init(struct texture *texture, GLint internal_format, GLenum format, GLenum type) {
	texture->internal_format = internal_format;
	texture->format = format;
	texture->type = type;

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void texture_param(struct texture *texture, GLenum option, GLint value) {
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexParameteri(GL_TEXTURE_2D, option, value);
}

void texture_data(struct texture *texture, int width, int height, const void *data) {
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexImage2D(GL_TEXTURE_2D, 0, texture->internal_format, width, height, 0, texture->format, texture->type, data);
}

void texture_bind(struct texture *texture, GLuint texture_unit) {
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture->id);
}

void texture_unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}