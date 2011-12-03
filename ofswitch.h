#ifndef OFSWITCH_H
#define	OFSWITCH_H

#include <sys/queue.h>

struct ofport {
	int		ofp_number;
};

struct ofswitch {
	TAILQ_ENTRY(ofswitch)	ofs_next;
	int			ofs_switch_fd;
	int			ofs_controller_fd;
	int			ofs_number;
	int			ofs_nports;
	struct ofport		*ofs_ports;
};

#endif /* !OFSWITCH_H */
