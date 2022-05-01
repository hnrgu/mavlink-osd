#include "font.h"

#include "shader.h"
#include "texture.h"
#include "render.h"

#include <GLES2/gl2.h>
#include <string.h>

static GLuint buffer_pos;
static GLuint buffer_uv;
static GLuint buffer_char;
static GLuint buffer_index;

static GLint a_pos;
static GLint a_char;
static GLint a_uv;

static GLint u_texture;
static GLint u_transform;
static GLint u_color;

static struct program font_program;
static struct texture font_texture;

static uint32_t *buffer_char_data;

#define TEXTURE_WIDTH 208
#define TEXTURE_HEIGHT 162

#define GLYPH_WIDTH  (TEXTURE_WIDTH / 16)
#define GLYPH_HEIGHT (TEXTURE_HEIGHT / 6)

#define MAX_STRING_LENGTH 256
#define MAX_STRING_LINES 64

#define BUF_ELEM_FLOATS 8

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void font_init() {
	SHADER_LOAD(&font_program, _binary_shaders_font_vert, _binary_shaders_font_frag);
	shader_enable(&font_program);

	texture_init(&font_texture, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	texture_param(&font_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture_param(&font_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	TEXTURE_DATA_LOAD(&font_texture, _binary_fonts_font_rgba, TEXTURE_WIDTH, TEXTURE_HEIGHT);

	a_pos = shader_get_attrib(&font_program, "a_pos");
	a_uv = shader_get_attrib(&font_program, "a_uv");
	a_char = shader_get_attrib(&font_program, "a_char");

	u_transform = shader_get_uniform(&font_program, "u_transform");
	u_texture = shader_get_uniform(&font_program, "u_texture");
	u_color = shader_get_uniform(&font_program, "u_color");

	GLint data_buffer_size = MAX_STRING_LENGTH * BUF_ELEM_FLOATS * sizeof(GLfloat);
	GLint index_buffer_size = MAX_STRING_LENGTH * 6 * sizeof(GLushort);

	float *data_buffer = malloc(MAX(data_buffer_size, index_buffer_size));

	float *buffer_pos_data = data_buffer;
	glGenBuffers(1, &buffer_pos);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_pos);
	for(unsigned i = 0; i < MAX_STRING_LENGTH; i++) {
		buffer_pos_data[i * BUF_ELEM_FLOATS + 0] = i;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 1] = 0;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 2] = i;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 3] = 1;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 4] = i + 1;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 5] = 0;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 6] = i + 1;
		buffer_pos_data[i * BUF_ELEM_FLOATS + 7] = 1;
	}
	glBufferData(GL_ARRAY_BUFFER, data_buffer_size, buffer_pos_data, GL_STATIC_DRAW);

	GLfloat *buffer_uv_data = data_buffer; // Buffer reuse
	glGenBuffers(1, &buffer_uv);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);
	for(size_t i = 0; i < MAX_STRING_LENGTH; ++i) {
		buffer_uv_data[i * BUF_ELEM_FLOATS + 0] = 0;
		buffer_uv_data[i * BUF_ELEM_FLOATS + 1] = 0;
		buffer_uv_data[i * BUF_ELEM_FLOATS + 2] = 0;
		buffer_uv_data[i * BUF_ELEM_FLOATS + 3] = 1;
		buffer_uv_data[i * BUF_ELEM_FLOATS + 4] = 1;
		buffer_uv_data[i * BUF_ELEM_FLOATS + 5] = 0;
		buffer_uv_data[i * BUF_ELEM_FLOATS + 6] = 1;
		buffer_uv_data[i * BUF_ELEM_FLOATS + 7] = 1;
	}
	glBufferData(GL_ARRAY_BUFFER, data_buffer_size, buffer_uv_data, GL_STATIC_DRAW);

	GLushort *buffer_index_data = data_buffer; // Buffer reuse
	glGenBuffers(1, &buffer_index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_index);
	for(unsigned i = 0; i < MAX_STRING_LENGTH; i++) {
		buffer_index_data[i * 6 + 0] = i * 4 + 0;
		buffer_index_data[i * 6 + 1] = i * 4 + 1;
		buffer_index_data[i * 6 + 2] = i * 4 + 2;
		buffer_index_data[i * 6 + 3] = i * 4 + 1;
		buffer_index_data[i * 6 + 4] = i * 4 + 2;
		buffer_index_data[i * 6 + 5] = i * 4 + 3;
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, buffer_index_data, GL_STATIC_DRAW);

	buffer_char_data = data_buffer; // Buffer reuse
	glGenBuffers(1, &buffer_char);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_char);
}

void font_render_line(const char *text) {
	shader_enable(&font_program);
	size_t len = strlen(text);
	if(len > MAX_STRING_LENGTH) {
		len = MAX_STRING_LENGTH;
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer_pos);
	glVertexAttribPointer(a_pos, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(a_pos);

	for(size_t i = 0; i < len; ++i) {
		char c = text[i];
		if(c < 32 || c > 126) {
			c = ' ';
		}
		buffer_char_data[i] = c << 24 | c << 16 | c << 8 | c;
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer_char);
	glBufferData(GL_ARRAY_BUFFER, len * 4, buffer_char_data, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(a_char, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(a_char);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);
	glVertexAttribPointer(a_uv, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(a_uv);

	render_push_matrix();
	render_scale(GLYPH_WIDTH, GLYPH_HEIGHT);
	glUniformMatrix3fv(u_transform, 1, GL_FALSE, &render_current_transform);
	render_pop_matrix();

	shader_enable(&font_program);
	glUniform4fv(u_color, 1, &render_current_color);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_index);
	texture_bind(&font_texture, u_texture);
	glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_SHORT, NULL);
}

void font_render(const char *text, uint8_t align) {
	int len = strlen(text);
	char buf[len + 1];
	strcpy(buf, text);
	buf[len] = 0;

	const char *lines[MAX_STRING_LINES];
	int line_count = 0;
	int max_len = 0;

	char *token = strtok(buf, "\n");
	while (token) {
		int line_len = strlen(token);
		if (line_len > max_len) max_len = line_len;

		lines[line_count ++] = token;
		if (line_count >= MAX_STRING_LINES) break;
		token = strtok(NULL, "\n");
	}

	render_push_matrix();
	if (align & FONT_ALIGN_H_CENTER) {
		render_translate(-max_len * GLYPH_WIDTH / 2.0, 0);
	} else if (align & FONT_ALIGN_H_RIGHT) {
		render_translate(-max_len * GLYPH_WIDTH, 0);
	}

	if (align & FONT_ALIGN_V_CENTER) {
		render_translate(0, -line_count * GLYPH_HEIGHT / 2.0);
	} else if (align & FONT_ALIGN_V_BOTTOM) {
		render_translate(0, -line_count * GLYPH_HEIGHT);
	}

	for (int i = 0; i < line_count; i ++) {
		if (align & FONT_ALIGN_H_CENTER) {
			render_push_matrix();
			render_translate((max_len - strlen(lines[i])) * GLYPH_WIDTH / 2.0, 0);
		} else if (align & FONT_ALIGN_H_RIGHT) {
			render_push_matrix();
			render_translate((max_len - strlen(lines[i])) * GLYPH_WIDTH, 0);
		}

		font_render_line(lines[i]);

		if (align & (FONT_ALIGN_H_CENTER | FONT_ALIGN_H_RIGHT)) {
			render_pop_matrix();
		}

		render_translate(0, GLYPH_HEIGHT);
	}
	render_pop_matrix();
}
