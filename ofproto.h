#ifndef OFPROTO_H
#define	OFPROTO_H

#include <stdbool.h>

void	ofproto_new_switch(int switch_fd, int controller_fd);
bool	ofproto_handle(int fd);

#endif /* !OFPROTO_H */
