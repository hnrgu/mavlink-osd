#include "layout.h"

#include <string.h>
#include <stdlib.h>

#include "widget.h"
#include "widgets/widget_text.h"
#include "render.h"
#include "matrix.h"

struct layout_widget {
	struct widget *widget;
    float relative_x, relative_y;
    int offset_x, offset_y;
	float transform[3][3];
};

static struct layout_widget *widgets;
static size_t widget_size;
static size_t widget_capacity;
static int logical_width;
static int logical_height;

void layout_init() {
	widget_size = 0;
	widget_capacity = 16;
	widgets = malloc(widget_capacity * sizeof(struct layout_widget));
}

void layout_add(void *widget, float relative_x, float relative_y, int offset_x, int offset_y, float transform[3][3]) {
	if (widget_size + 1 >= widget_capacity) {
		widget_capacity *= 2;
		widgets = realloc(widgets, widget_capacity);
	}

	struct layout_widget *elem = &widgets[widget_size ++];
	elem->widget = widget;
    elem->relative_x = relative_x;
    elem->relative_y = relative_y;
    elem->offset_x = offset_x;
    elem->offset_y = offset_y;
	memcpy(&elem->transform, transform, 3 * 3 * sizeof(float));
}

void layout_set_logical_size(int width, int height) {
    logical_width = width;
    logical_height = height;
}

void layout_render() {
	for (int i = 0; i < widget_size; i ++) {
		render_push_matrix();
        render_translate(widgets[i].relative_x * logical_width + widgets[i].offset_x, widgets[i].relative_y * logical_height + widgets[i].offset_y);
        render_transform(widgets[i].transform);
		widgets[i].widget->draw(widgets[i].widget);
		render_pop_matrix();
		render_stencil_mode(RENDER_STENCIL_DISABLE);
	}
}
