#ifndef RESOURCE_H
#define RESOURCE_H

#define RESOURCE_DECL(sym)   \
	extern char sym##_start; \
	extern char sym##_end

#define RESOURCE_SIZE(sym) ((const char *) &sym##_end - (const char *) &sym##_start)

#endif
