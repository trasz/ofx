#ifndef MATLAB_H
#define	MATLAB_H

#include <sys/queue.h>

struct packet;

struct matlab {
	TAILQ_ENTRY(matlab)	matlab_next;
	int			matlab_fd;
	struct packet		*matlab_p;
};

void	matlab_handle(struct matlab *matlab, int fd);

#endif /* !MATLAB_H */
