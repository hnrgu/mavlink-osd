#ifndef WIDGET_TAPE_H
#define WIDGET_TAPE_H

#include <stdint.h>

#include "../shape.h"

struct widget_tape {
	void (*draw)(struct widget_tape *widget);
	struct shape_context tape;
	struct shape_context indicator;
	struct shape_context stencil;
	struct shape_context line;
	int round_mult;
    int characters; // number of characters to display
	int decimals; // number of decimal points
	int num_marks; // number of visible markings on screen
	int mark_spacing; // spacing between marks in pixels
	int mark_length; // length of marks in pixels
	int mark_value; // the value of a mark multiplied by 10^decimals
	int label_period; // how often to label a mark, i.e. 2 is every other, 3 is every third
	int label_gap; // number of pixels between text and mark line
	int width; // width of tape in pixels
	int arrow_width; // width of arrow in pixels
	int arrow_height; // height of arrow in pixels
	int indicator_width; // width of indicator in pixels
	int indicator_height; // height of indicator in pixels
	int indicator_gap; // number of pixels between indicator and text
	int direction; // 0 for text on right, 1 for text on left
	float value;
};

// All struct members with comments above must be initialized
void widget_tape_init(struct widget_tape *widget);

#endif
