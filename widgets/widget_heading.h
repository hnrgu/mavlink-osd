#ifndef WIDGET_HEADING_INDICATOR_H
#define WIDGET_HEADING_INDICATOR_H

#include "../shape.h"

struct widget_heading_indicator {
	void (*draw)(struct widget_heading_indicator *widget);
	float heading;
	float direction;
	struct shape_context arc;
	struct shape_context arrow;
	struct shape_context needle;
	struct shape_context needle_arrow;
	struct shape_context marks;
};

void widget_heading_indicator_init(struct widget_heading_indicator *widget);

#endif
