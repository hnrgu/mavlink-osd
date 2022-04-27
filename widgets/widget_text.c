#include "widget_text.h"

#include <string.h>

#include "../font.h"
#include "../render.h"

static void draw(struct widget_text *widget) {
	render_set_color(widget->color[0], widget->color[1], widget->color[2], widget->color[3]);
	font_render(widget->text, widget->align);
}

void widget_text_init(struct widget_text *widget) {
	widget->draw = draw;
	widget->text[0] = 0;
	widget->align = FONT_ALIGN_DEFAULT;

	widget->color[0] = 1;
	widget->color[1] = 1;
	widget->color[2] = 1;
	widget->color[3] = 1;
}

void widget_text_set(struct widget_text *widget, const char *text) {
	strncpy(widget->text, text, 2047);
}
