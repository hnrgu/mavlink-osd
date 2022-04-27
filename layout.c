#include "layout.h"

#include <string.h>
#include <stdlib.h>

#include "widget.h"
#include "widgets/widget_text.h"
#include "render.h"

struct layout_widget {
	struct widget *widget;
	float transform[3][3];
};

static struct layout_widget *widgets;
static size_t widget_size;
static size_t widget_capacity;

void layout_init() {
	widget_size = 0;
	widget_capacity = 16;
	widgets = malloc(widget_capacity * sizeof(struct layout_widget));
}

void layout_add(struct widget *widget, float transform[3][3]) {
	if (widget_size + 1 >= widget_capacity) {
		widget_capacity *= 2;
		widgets = realloc(widgets, widget_capacity);
	}

	struct layout_widget *elem = &widgets[widget_size ++];
	elem->widget = widget;
	memcpy(&elem->transform, transform, 3 * 3 * sizeof(float));
}

void layout_render() {
	for (int i = 0; i < widget_size; i ++) {
		render_push_matrix();
		render_transform(widgets[i].transform);
		widgets[i].widget->draw(widgets[i].widget);
		render_pop_matrix();
	}
}
