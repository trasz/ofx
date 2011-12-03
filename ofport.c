#include <err.h>
#include <stdlib.h>

#include "ofport.h"

struct ofport *
ofp_alloc(void)
{
	struct ofport *ofp;

	ofp = calloc(sizeof(*ofp), 1);
	if (ofp == NULL)
		err(1, "calloc");

	return (ofp);
}

