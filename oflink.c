#include <err.h>
#include <stdlib.h>

#include "debug.h"
#include "oflink.h"
#include "ofport.h"
#include "ofswitch.h"

void
ofl_link(struct ofport *ofp1, struct ofport *ofp2)
{
	struct oflink *ofl;

	if (ofp1->ofp_link != NULL && ofp1->ofp_link == ofp2->ofp_link)
		return;

	if (ofp1->ofp_link != NULL && ofp2->ofp_link != NULL) {
		debug("ports %d:%d and %d:%d already linked to something; unlinking both.\n",
		    ofp1->ofp_switch->ofs_number, ofp1->ofp_number, ofp2->ofp_switch->ofs_number, ofp2->ofp_number);
		ofl_unlink(ofp1);
		ofl_unlink(ofp2);
	}

	if (ofp1->ofp_link != NULL) {
		ofl = ofp1->ofp_link;
		ofp2->ofp_link = ofl;
		TAILQ_INSERT_TAIL(&ofl->ofl_ports, ofp2, ofp_next_link);
	} else if (ofp2->ofp_link != NULL) {
		ofl = ofp2->ofp_link;
		ofp1->ofp_link = ofl;
		TAILQ_INSERT_TAIL(&ofl->ofl_ports, ofp1, ofp_next_link);
	} else {
		ofl = calloc(sizeof(*ofl), 1);
		if (ofl == NULL)
			err(1, "calloc");
		ofp1->ofp_link = ofl;
		TAILQ_INSERT_TAIL(&ofl->ofl_ports, ofp1, ofp_next_link);
		ofp2->ofp_link = ofl;
		TAILQ_INSERT_TAIL(&ofl->ofl_ports, ofp2, ofp_next_link);
	}
}

void
ofl_unlink(struct ofport *ofp)
{
	struct oflink *ofl;

	if (ofp->ofp_link == NULL)
		return;

	ofl = ofp->ofp_link;
	TAILQ_REMOVE(&ofl->ofl_ports, ofp, ofp_next_link);
	if (TAILQ_EMPTY(&ofl->ofl_ports))
		free(ofl);

	ofp->ofp_link = NULL;
}
