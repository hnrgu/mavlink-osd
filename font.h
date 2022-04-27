#ifndef FONT_H
#define FONT_H

#include <GLES2/gl2.h>

#define FONT_ALIGN_H_LEFT 0
#define FONT_ALIGN_H_CENTER 1
#define FONT_ALIGN_H_RIGHT 2
#define FONT_ALIGN_V_TOP 0
#define FONT_ALIGN_V_CENTER 4
#define FONT_ALIGN_V_BOTTOM 8

#define FONT_ALIGN_DEFAULT FONT_ALIGN_H_LEFT | FONT_ALIGN_V_TOP

void font_init();
void font_render(const char *text, uint8_t align);

#endif
