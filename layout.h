#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"

void layout_init();

void layout_add(void *widget, float relative_x, float relative_y, int offset_x, int offset_y, float transform[3][3]);

void layout_update();

void layout_set_logical_size(int width, int height);

void layout_render();

#endif
