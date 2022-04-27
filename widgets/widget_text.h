#ifndef WIDGET_TEXT_H
#define WIDGET_TEXT_H

#include <stdint.h>

struct widget_text {
	void (*draw)(struct widget_text *widget);
	char text[2048];
	uint8_t align;
	float color[4];
};

void widget_text_init(struct widget_text *widget);

void widget_text_set(struct widget_text *widget, const char *text);

#endif
