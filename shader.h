#ifndef SHADER_H
#define SHADER_H

#include <GLES2/gl2.h>
#include <stdlib.h>

#include "resource.h"

#define SHADER_LOAD(program, vert_sym, frag_sym) {                             \
    RESOURCE_DECL(vert_sym);                                                   \
    RESOURCE_DECL(frag_sym);                                                   \
    shader_load_program(program,                                               \
                        &vert_sym##_start, RESOURCE_SIZE(vert_sym), #vert_sym, \
                        &frag_sym##_start, RESOURCE_SIZE(frag_sym), #frag_sym);\
}

struct program {
	GLuint id;
	GLuint vertex_shader;
	GLuint fragment_shader;
};

void shader_load_program(struct program *program, const GLchar *vert, GLint vert_size, const char *vert_sym, const GLchar *frag, GLint frag_size, const char *frag_sym);
GLint shader_get_attrib(struct program *program, const GLchar *name);
GLint shader_get_uniform(struct program *program, const GLchar *name);
GLint shader_enable(struct program *program);
GLint shader_disable();

#endif
