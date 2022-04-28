#ifndef WIDGET_ATTITUDE_INDICATOR_H
#define WIDGET_ATTITUDE_INDICATOR_H

#include "../shape.h"

struct widget_attitude_indicator {
	void (*draw)(struct widget_attitude_indicator *widget);
	float attitude_roll;
	float attitude_pitch;
	struct shape_context bg_half;
	struct shape_context pitch_lines;
	struct shape_context horizon;
	struct shape_context center;
	struct shape_context stencil;
};

void widget_attitude_indicator_init(struct widget_attitude_indicator *widget);

#endif
