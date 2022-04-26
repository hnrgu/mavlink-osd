#include "font.h"

#include "shader.h"
#include "texture.h"

#include <GLES2/gl2.h>
#include <string.h>

static GLuint buffer_pos;
static GLuint buffer_uv;

static GLint a_pos;
static GLint a_uv;

static GLint u_texture;

static GLint u_screen_size;
static GLint u_color;
static GLint u_offset;
static GLint u_scale;

static struct program font_program;
static struct texture font_texture;

static float *buffer_uv_data;

static float r = 1, g = 1, b = 1, a = 1;
static float scale = 2;

#define TEXTURE_WIDTH 208
#define TEXTURE_HEIGHT 162

#define GLYPH_WIDTH  (TEXTURE_WIDTH / 16)
#define GLYPH_HEIGHT (TEXTURE_HEIGHT / 6)

#define MAX_STRING_LENGTH 256

#define BUF_ELEM_FLOATS 12
#define BUF_ELEM_BYTES (sizeof(GLfloat) * BUF_ELEM_FLOATS)

void font_init() {
	SHADER_LOAD(&font_program, _binary_shaders_font_vert, _binary_shaders_font_frag);
	shader_enable(&font_program);

	texture_init(&font_texture, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	TEXTURE_DATA_LOAD(&font_texture, _binary_fonts_font_rgba, TEXTURE_WIDTH, TEXTURE_HEIGHT);

	a_pos = glGetAttribLocation(font_program.id, "a_pos");
	a_uv = glGetAttribLocation(font_program.id, "a_uv");

	u_texture = glGetUniformLocation(font_program.id, "u_texture");
	u_screen_size = glGetUniformLocation(font_program.id, "u_screen_size");
	u_color = glGetUniformLocation(font_program.id, "u_color");
	u_offset = glGetUniformLocation(font_program.id, "u_offset");
	u_scale = glGetUniformLocation(font_program.id, "u_scale");

	GLint buffer_size = MAX_STRING_LENGTH * BUF_ELEM_BYTES;

	float *buffer_pos_data = malloc(buffer_size);
	glGenBuffers(1, &buffer_pos);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_pos);
	for(unsigned i = 0; i < MAX_STRING_LENGTH; i++) {
		buffer_pos_data[i * BUF_ELEM_FLOATS + 0] = i;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 1] = 0;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 2] = i;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 3] = 1;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 4] = i + 1;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 5] = 0;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 6] = i;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 7] = 1;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 8] = i + 1;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 9] = 0;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 10] = i + 1;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 11] = 1;
	}
	glBufferData(GL_ARRAY_BUFFER, buffer_size, buffer_pos_data, GL_STATIC_DRAW);

	buffer_uv_data = buffer_pos_data; // Buffer reuse;
	glGenBuffers(1, &buffer_uv);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);
	buffer_size = MAX_STRING_LENGTH * sizeof(GLfloat) * 8;
}

void font_window_size(GLfloat width, GLfloat height) {
	shader_enable(&font_program);
	glUniform2f(u_screen_size, width, height);
}

void font_color(float new_r, float new_g, float new_b, float new_a) {
	r = new_r;
	g = new_g;
	b = new_b;
	a = new_a;
}

void font_scale(float new_scale) {
	scale = new_scale;
}

void font_render(const char *text, float x, float y) {
	shader_enable(&font_program);
	size_t slen = strlen(text);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_pos);
	glVertexAttribPointer(a_pos, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(a_pos);

	for (size_t i = 0; i < slen; ++i) {
		int len = 0;
		for (; i < slen && len <= MAX_STRING_LENGTH; ++i, ++len) {
			char c = text[i];
			if(c == '\n') break;
			if(c < 32 || c > 126) {
				c = ' ';
			}
			c -= ' ';
			float u = c % 16;
			float v = c / 16;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 0] = u;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 1] = v;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 2] = u;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 3] = v + 1;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 4] = u + 1;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 5] = v;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 6] = u;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 7] = v + 1;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 8] = u + 1;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 9] = v;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 10] = u + 1;
			buffer_uv_data[len * BUF_ELEM_FLOATS + 11] = v + 1;
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);
		glBufferData(GL_ARRAY_BUFFER, len * BUF_ELEM_BYTES, buffer_uv_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(a_uv, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(a_uv);

		glUniform2f(u_offset, x, y);
		glUniform2f(u_scale, GLYPH_WIDTH * scale, GLYPH_HEIGHT * scale);

		shader_enable(&font_program);
		glUniform4f(u_color, r, g, b, a);

		texture_bind(&font_texture, u_texture);
		glDrawArrays(GL_TRIANGLES, 0, len * 6);

		y += GLYPH_HEIGHT;
	}
}
