#include "widget_bank_indicator.h"

#include <string.h>
#include <math.h>

#include "../render.h"

static const int angle_marks[] = { 0, 10, 20, 30, 45, 60 };
static const int mark_size[] = { 2, 1, 1, 2, 1, 2 };

#define ARC_SEGMENTS 20
#define MARKS_SCALE 0.1
#define ARROW_SIZE 0.15

static void draw(struct widget_bank_indicator *widget) {
	render_set_color(1, 1, 1, 1);
	render_push_matrix();
	render_translate(0, -1);
	shape_draw(&widget->arrow);
	render_pop_matrix();

	render_rotate(-widget->bank_angle);
	shape_draw(&widget->arc);
	shape_draw(&widget->marks);
}

void widget_bank_indicator_init(struct widget_bank_indicator *widget) {
	widget->draw = draw;
	widget->bank_angle = 0;

	shape_new(&widget->arc);
	shape_new(&widget->marks);
	shape_new(&widget->arrow);

	int num_marks = sizeof(angle_marks) / sizeof(angle_marks[0]);
	float largest_angle = angle_marks[num_marks - 1] / 180.0 * M_PI;

	float step = largest_angle * 2 / ARC_SEGMENTS;

	shape_begin(&widget->arc, GL_LINE_STRIP);
	for (int i = 0; i < ARC_SEGMENTS + 1; i ++) {
		float angle = step * i - largest_angle - M_PI / 2;
		shape_vertex(&widget->arc, cosf(angle), sinf(angle));
	}
	shape_end(&widget->arc);

	shape_begin(&widget->marks, GL_LINES);
	for (int i = 0; i < num_marks; i ++) {
		float angle = angle_marks[i] / 180.0 * M_PI - M_PI / 2;
		float size = 1 + mark_size[i] * MARKS_SCALE;

		shape_vertex(&widget->marks, cosf(angle), sinf(angle));
		shape_vertex(&widget->marks, size*cosf(angle), size*sinf(angle));

		if (angle_marks[i] != 0) {
			angle = M_PI - angle;
			shape_vertex(&widget->marks, cosf(angle), sinf(angle));
			shape_vertex(&widget->marks, size*cosf(angle), size*sinf(angle));
		}
	}
	shape_end(&widget->marks);

	shape_begin(&widget->arrow, GL_TRIANGLES);
	shape_vertex(&widget->arrow, 0, 0);
	shape_vertex(&widget->arrow, -ARROW_SIZE/sqrtf(3), ARROW_SIZE);
	shape_vertex(&widget->arrow, ARROW_SIZE/sqrtf(3), ARROW_SIZE);
	shape_end(&widget->arrow);
}
