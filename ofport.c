#include <assert.h>
#include <err.h>
#include <stdlib.h>

#include "ofport.h"
#include "ofswitch.h"

struct ofport *
ofp_alloc(void)
{
	struct ofport *ofp;

	ofp = calloc(sizeof(*ofp), 1);
	if (ofp == NULL)
		err(1, "calloc");

	return (ofp);
}

void
ofp_free(struct ofport *ofp)
{

	free(ofp);
}

struct ofport *
ofp_find_by_number(struct ofswitch *ofs, int number)
{
	struct ofport *ofp;

	TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next) {
		assert(ofp->ofp_switch == ofs);
		if (ofp->ofp_number == number)
			return (ofp);
	}

	return (NULL);
}

