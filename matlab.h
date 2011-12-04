#ifndef MATLAB_H
#define	MATLAB_H

#include <sys/queue.h>

struct packet;

struct matlab {
	TAILQ_ENTRY(matlab)	matlab_next;
	int			matlab_fd;
	struct packet		*matlab_p;
};

void	matlab_add(int matlab_fd);
struct matlab	*matlab_find(int fd);
void	matlab_handle(struct matlab *matlab, int fd);

TAILQ_HEAD(matlabs, matlab);
extern struct matlabs matlabs;

#endif /* !MATLAB_H */
