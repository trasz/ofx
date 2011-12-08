#ifndef MATLAB_H
#define	MATLAB_H

#include <sys/queue.h>

struct packet;

struct mat {
	TAILQ_ENTRY(mat)	mat_next;
	int			mat_fd;
	struct packet		*mat_p;
};

void	mat_add(int mat_fd);
struct mat	*mat_find_by_fd(int fd);
void	mat_handle(struct mat *mat, int fd);

TAILQ_HEAD(matlabs, mat);
extern struct matlabs matlabs;

#endif /* !MATLAB_H */
