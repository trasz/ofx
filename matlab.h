#ifndef MATLAB_H
#define	MATLAB_H

#include <sys/queue.h>

struct matlab {
	TAILQ_ENTRY(matlab)	matlab_next;
	int			matlab_fd;
};

void	matlab_handle(struct matlab *matlab, int fd);

#endif /* !MATLAB_H */
