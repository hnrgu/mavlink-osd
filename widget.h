#ifndef WIDGET_H
#define WIDGET_H

struct widget {
	void (*draw)(struct widget *);
};

#endif
