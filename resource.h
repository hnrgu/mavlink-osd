#ifndef RESOURCE_H
#define RESOURCE_H

#define RESOURCE_START(sym) _binary_ ## sym ## _start
#define RESOURCE_END(sym) _binary_ ## sym ## _end

#define RESOURCE_DECL(sym)   \
	extern char RESOURCE_START(sym); \
	extern char RESOURCE_END(sym)

#define RESOURCE_SIZE(sym) ((const char *) &RESOURCE_END(sym) - (const char *) &RESOURCE_START(sym))

#endif
