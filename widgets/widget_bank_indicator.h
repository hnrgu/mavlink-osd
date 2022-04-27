#ifndef WIDGET_BANK_INDICATOR_H
#define WIDGET_BANK_INDICATOR_H

#include "../shape.h"

struct widget_bank_indicator {
	void (*draw)(struct widget_bank_indicator *widget);
	float bank_angle;
	struct shape_context arc;
	struct shape_context marks;
	struct shape_context arrow;
};

void widget_bank_indicator_init(struct widget_bank_indicator *widget);

void widget_bank_indicator_set(struct widget_bank_indicator *widget, const char *text);

#endif
