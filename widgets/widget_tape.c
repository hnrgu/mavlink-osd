#include "widget_tape.h"

#include <string.h>
#include <math.h>
#include <stdio.h>

#include "../font.h"
#include "../render.h"

static void draw(struct widget_tape *widget) {
	float dist_per_val = widget->mark_spacing / widget->mark_value;

	float s_val = widget->value * widget->round_mult + 0.5;
	float d_val = s_val / widget->mark_value;
	d_val -= floor(d_val);

	shape_draw(&widget->line);

	render_stencil_begin();
	shape_draw(&widget->stencil);
	render_stencil_end();

	render_stencil_mode(RENDER_STENCIL_INSIDE);

	render_set_color(1, 1, 1, 1);
	render_push_matrix();
	render_translate(0, d_val * dist_per_val);

	render_push_matrix();
	render_translate(0, -widget->mark_spacing * widget->num_marks / 2.0);
	shape_draw(&widget->tape);
	render_pop_matrix();

	render_translate(widget->mark_length + widget->label_gap, -widget->mark_spacing * (widget->num_marks / 2.0));
	for (int i = 0; i < widget->num_marks; i ++) {
		int ind = (-i + widget->num_marks / 2 + floor(s_val / widget->mark_value));

		if (ind % widget->label_period == 0) {
			float val = widget->mark_value * (float) ind / widget->round_mult;
			char buf[16];
			snprintf(buf, sizeof(buf), "%.*f", widget->decimals, val);
			font_render(buf, FONT_ALIGN_H_LEFT | FONT_ALIGN_V_CENTER);
		}

		render_translate(0, widget->mark_spacing);
	}
	render_pop_matrix();

	render_stencil_mode(RENDER_STENCIL_DISABLE);

	render_set_color(0, 0, 0, 1);
	widget->indicator.mode = GL_TRIANGLE_FAN;
	shape_draw(&widget->indicator);

	widget->indicator.mode = GL_LINE_LOOP;
	render_set_color(1, 1, 1, 1);
	shape_draw(&widget->indicator);

	render_translate(widget->mark_length + widget->arrow_width + widget->indicator_gap, 0);
	char buf[16];
	snprintf(buf, sizeof(buf), "%.*f", widget->decimals, widget->value);
	font_render(buf, FONT_ALIGN_H_LEFT | FONT_ALIGN_V_CENTER);
}

void widget_tape_init(struct widget_tape *widget) {
	widget->draw = draw;

	widget->value = 0;

	widget->round_mult = 1;
	for (int i = 0; i < widget->decimals; i ++) {
		widget->round_mult *= 10;
	}

	shape_new(&widget->tape);
	shape_begin(&widget->tape, GL_LINES);
	for (int i = 0; i < widget->num_marks; i ++) {
		shape_vertex(&widget->tape, 0, i * widget->mark_spacing);
		shape_vertex(&widget->tape, widget->mark_length, i * widget->mark_spacing);
	}
	shape_end(&widget->tape);

	shape_new(&widget->indicator);
	shape_begin(&widget->indicator, GL_TRIANGLE_FAN);
	shape_vertex(&widget->indicator, widget->mark_length + widget->arrow_width, -widget->arrow_height/2);
	shape_vertex(&widget->indicator, widget->mark_length, 0);
	shape_vertex(&widget->indicator, widget->mark_length + widget->arrow_width, widget->arrow_height/2);
	shape_vertex(&widget->indicator, widget->mark_length + widget->arrow_width, widget->indicator_height/2);
	shape_vertex(&widget->indicator, widget->mark_length + widget->arrow_width + widget->indicator_width, widget->indicator_height/2);
	shape_vertex(&widget->indicator, widget->mark_length + widget->arrow_width + widget->indicator_width, -widget->indicator_height/2);
	shape_vertex(&widget->indicator, widget->mark_length + widget->arrow_width, -widget->indicator_height/2);
	shape_end(&widget->indicator);

	shape_new(&widget->stencil);
	shape_begin(&widget->stencil, GL_TRIANGLE_FAN);
	shape_vertex(&widget->stencil, 0, -widget->mark_spacing * widget->num_marks / 2.0);
	shape_vertex(&widget->stencil, 100, -widget->mark_spacing * widget->num_marks / 2.0);
	shape_vertex(&widget->stencil, 100, +widget->mark_spacing * widget->num_marks / 2.0);
	shape_vertex(&widget->stencil, 0, +widget->mark_spacing * widget->num_marks / 2.0);
	shape_end(&widget->stencil);

	shape_new(&widget->line);
	shape_begin(&widget->line, GL_LINES);
	shape_vertex(&widget->line, 0, -widget->mark_spacing * widget->num_marks / 2.0);
	shape_vertex(&widget->line, 0, +widget->mark_spacing * widget->num_marks / 2.0);
	shape_end(&widget->line);
}
