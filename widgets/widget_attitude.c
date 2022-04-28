#include "widget_attitude.h"

#include <string.h>
#include <math.h>
#include <stdio.h>

#include "../render.h"
#include "../font.h"

#define VERY_FAR 10000

static void draw(struct widget_attitude_indicator *widget) {
	char str[16];

	render_rotate(-widget->attitude_roll);
	render_translate(0, widget->attitude_pitch / M_PI * 180 * 10);

	render_set_color(0, 0.25, 0.75, 1);
	shape_draw(&widget->bg_half);

	render_set_color(0.5, 0.25, 0, 1);
	render_push_matrix();
	render_scale(1, -1);
	shape_draw(&widget->bg_half);
	render_pop_matrix();

	render_set_color(1, 1, 1, 1);
	for(int i = -9; i < 9; ++i) {
		if(i == 0) {
			continue;
		}

		snprintf(str, sizeof(str), "%d", abs(i * 10));

		render_push_matrix();
		render_translate(120, i * 100);
		font_render(str, FONT_ALIGN_V_CENTER | FONT_ALIGN_H_LEFT);
		render_pop_matrix();

		render_push_matrix();
		render_translate(-120, i * 100);
		font_render(str, FONT_ALIGN_V_CENTER | FONT_ALIGN_H_RIGHT);
		render_pop_matrix();
	}

	shape_draw(&widget->pitch_lines);
}

void widget_attitude_indicator_init(struct widget_attitude_indicator *widget) {
	widget->draw = draw;
	widget->attitude_pitch = 0;
	widget->attitude_roll = 0;

	shape_new(&widget->pitch_lines);
	shape_new(&widget->bg_half);

	shape_begin(&widget->bg_half, GL_TRIANGLE_STRIP);
	shape_vertex(&widget->bg_half, -VERY_FAR, 0);
	shape_vertex(&widget->bg_half, VERY_FAR, 0);
	shape_vertex(&widget->bg_half, -VERY_FAR, -VERY_FAR);
	shape_vertex(&widget->bg_half, VERY_FAR, -VERY_FAR);
	shape_end(&widget->bg_half);

	shape_begin(&widget->pitch_lines, GL_LINES);
	for(int i = -9; i <= 9; ++i) {
		if(i == 0) {
			shape_vertex(&widget->pitch_lines, -VERY_FAR, 0);
			shape_vertex(&widget->pitch_lines, VERY_FAR, 0);
			continue;
		}

		shape_vertex(&widget->pitch_lines, -100, i * 100);
		shape_vertex(&widget->pitch_lines, -80, i * 100);
		shape_vertex(&widget->pitch_lines, 100, i * 100);
		shape_vertex(&widget->pitch_lines, 80, i * 100);
	}
	shape_end(&widget->pitch_lines);
}
