#ifndef OFSWITCH_H
#define	OFSWITCH_H

#include <sys/types.h>
#include <sys/queue.h>

struct ofport;

struct ofswitch {
	TAILQ_ENTRY(ofswitch)	ofs_next;
	int			ofs_switch_fd;
	int			ofs_controller_fd;
	int			ofs_number;
	int			ofs_nports;
	TAILQ_HEAD(, ofport)	ofs_ports;
};

void	ofs_add(int switch_fd, int controller_fd);
void	ofs_add_port(struct ofswitch *ofs, struct ofport *ofp);
void	ofs_delete_port(struct ofswitch *ofs, struct ofport *ofp);
void	ofs_modify_port(struct ofswitch *ofs, struct ofport *ofp);
struct ofswitch	*ofs_find_by_number(int number);
struct ofswitch	*ofs_find_by_fd(int fd);

TAILQ_HEAD(ofswitches, ofswitch);
extern struct ofswitches ofswitches;

#endif /* !OFSWITCH_H */
