#include "shader.h"

#include <stdio.h>
#include <assert.h>

void shader_load_shader(GLuint *shader, GLenum type, const GLchar *source, GLint len, const char *symbol) {
	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, &source, &len);
	glCompileShader(*shader);

	GLsizei log_len;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &log_len);
	if (log_len > 0) {
		char *buffer = malloc(log_len);
		glGetShaderInfoLog(*shader, log_len, NULL, buffer);
		fprintf(stderr, "Compiling %s\n%s\n", symbol, buffer);
		free(buffer);

		GLint success;
		glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			assert(0);
		}
	}
}

void shader_load_program(struct program *program, const GLchar *vert, GLint vert_size, const char *vert_sym, const GLchar *frag, GLint frag_size, const char *frag_sym) {
	program->id = glCreateProgram();
	shader_load_shader(&program->vertex_shader, GL_VERTEX_SHADER, vert, vert_size, vert_sym);
	shader_load_shader(&program->fragment_shader, GL_FRAGMENT_SHADER, frag, frag_size, frag_sym);

	glAttachShader(program->id, program->vertex_shader);
	glAttachShader(program->id, program->fragment_shader);

	glLinkProgram(program->id);

	GLsizei log_len;
	glGetProgramiv(program->id, GL_INFO_LOG_LENGTH, &log_len);
	if (log_len > 0) {
		char *buffer = malloc(log_len);
		glGetProgramInfoLog(program->id, log_len, NULL, buffer);
		fprintf(stderr, "Linking %s, %s\n%s\n", vert_sym, frag_sym, buffer);
		free(buffer);

		GLint success;
		glGetProgramiv(program->id, GL_LINK_STATUS, &success);
		if (!success) {
			assert(0);
		}
	}
}

GLint shader_get_attrib(struct program *program, const GLchar *name) {
	GLint loc = glGetAttribLocation(program->id, name);
	if(loc < 0) {
		fprintf(stderr, "location for attrib \"%s\" not found", name);
		assert(0);
	}
}

GLint shader_get_uniform(struct program *program, const GLchar *name) {
	GLint loc = glGetUniformLocation(program->id, name);
	if(loc < 0) {
		fprintf(stderr, "location for uniform \"%s\" not found", name);
		assert(0);
	}
}

GLint shader_enable(struct program *program) {
	glUseProgram(program->id);
}

GLint shader_disable() {
	glUseProgram(0);
}
