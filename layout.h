#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"

void layout_init();

void layout_add(struct widget *widget, float transform[3][3]);

void layout_update();

void layout_render();

#endif
