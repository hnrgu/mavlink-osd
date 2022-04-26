#ifndef FONT_H
#define FONT_H

#include <GLES2/gl2.h>

void font_init();
void font_window_size(GLfloat width, GLfloat height);
void font_color(float new_r, float new_g, float new_b, float new_a);
void font_scale(float new_scale);
void font_render(const char *text, float x, float y);

#endif
