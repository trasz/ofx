#ifndef DEBUG_H
#define	DEBUG_H

#include <stdio.h>

#define debug(FMT, ...)	fprintf(stderr, FMT, __VA_ARGS__)

#endif /* !DEBUG_H */
