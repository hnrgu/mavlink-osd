#include "widget_heading.h"

#include <string.h>
#include <math.h>
#include <stdio.h>

#include "../render.h"
#include "../font.h"

#define ARC_SEGMENTS 50
#define ARC_RADIUS 100

#define LARGE_TICK 10
#define SMALL_TICK 7

static void draw(struct widget_heading_indicator *widget) {
	char str[16];

	render_set_color(1, 1, 1, 1);
	shape_draw(&widget->arc);

	render_push_matrix();
	render_translate(0, -ARC_RADIUS - 20);
	snprintf(str, sizeof(str), "%03d", (int) round(widget->heading / M_PI * 180 + 360) % 360);
	font_render(str, FONT_ALIGN_V_CENTER | FONT_ALIGN_H_CENTER);
	render_pop_matrix();

	render_push_matrix();
	render_translate(0, -ARC_RADIUS);
	shape_draw(&widget->arrow);
	render_pop_matrix();

	render_rotate(-widget->heading);
	shape_draw(&widget->marks);
	for(int i = 0; i < 12; ++i) {
		float ang = i * 2 * M_PI / 12.0;
		render_push_matrix();
		render_rotate(ang);
		render_translate(0, -ARC_RADIUS + 10);

		if(i == 0) {
			snprintf(str, sizeof(str), "N");
		} else if(i == 6) {
			snprintf(str, sizeof(str), "S");
		} else if(i == 3) {
			snprintf(str, sizeof(str), "E");
		} else if(i == 9) {
			snprintf(str, sizeof(str), "W");
		} else {
			snprintf(str, sizeof(str), "%d", i * 3);
		}

		font_render(str, FONT_ALIGN_V_TOP | FONT_ALIGN_H_CENTER);
		render_pop_matrix();
	}

	render_set_color(0, 1, 0, 1);
	shape_draw(&widget->needle);
	shape_draw(&widget->needle_arrow);
}

void widget_heading_indicator_init(struct widget_heading_indicator *widget) {
	widget->draw = draw;
	widget->heading = 0;

	shape_new(&widget->arc);
	shape_new(&widget->arrow);
	shape_new(&widget->needle);
	shape_new(&widget->needle_arrow);
	shape_new(&widget->marks);

	shape_begin(&widget->arc, GL_LINE_LOOP);
	for(int i = 0; i < ARC_SEGMENTS; ++i) {
		float ang = i * 2 * M_PI / ARC_SEGMENTS;
		shape_vertex(&widget->arc, cosf(ang) * ARC_RADIUS, sinf(ang) * ARC_RADIUS);
	}
	shape_end(&widget->arc);

	shape_begin(&widget->arrow, GL_TRIANGLES);
	shape_vertex(&widget->arrow, -5, -10);
	shape_vertex(&widget->arrow, 5, -10);
	shape_vertex(&widget->arrow, 0, 0);
	shape_end(&widget->arrow);

#define NEEDLE_OFFSET 10

	shape_begin(&widget->needle, GL_LINES);
	shape_vertex(&widget->needle, 0, -ARC_RADIUS + NEEDLE_OFFSET);
	shape_vertex(&widget->needle, 0, ARC_RADIUS - NEEDLE_OFFSET);
	shape_end(&widget->needle);

	shape_begin(&widget->needle_arrow, GL_TRIANGLES);
	shape_vertex(&widget->needle_arrow, 0, -ARC_RADIUS + NEEDLE_OFFSET);
	shape_vertex(&widget->needle_arrow, -SMALL_TICK, -ARC_RADIUS + NEEDLE_OFFSET + 10);
	shape_vertex(&widget->needle_arrow, SMALL_TICK, -ARC_RADIUS + NEEDLE_OFFSET + 10);
	shape_end(&widget->needle_arrow);

	shape_begin(&widget->marks, GL_LINES);
	for(int i = 0; i < 36; ++i) {
		float ang = i * 2 * M_PI / 36.0;
		shape_vertex(&widget->marks, cosf(ang) * ARC_RADIUS, sinf(ang) * ARC_RADIUS);
		if(i % 3 == 0) {
			shape_vertex(&widget->marks, cosf(ang) * (ARC_RADIUS - LARGE_TICK), sinf(ang) * (ARC_RADIUS - LARGE_TICK));
		} else {
			shape_vertex(&widget->marks, cosf(ang) * (ARC_RADIUS - SMALL_TICK), sinf(ang) * (ARC_RADIUS - SMALL_TICK));
		}
	}
	shape_end(&widget->marks);
}
