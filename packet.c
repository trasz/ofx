#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "packet.h"

struct packet *
p_alloc(void)
{
	struct packet *p;

	p = calloc(sizeof(*p), 1);
	if (p == NULL)
		err(1, "calloc");

	return (p);
}

void
p_free(struct packet *p)
{

	if (p->p_payload != NULL)
		free(p->p_payload);
	memset(p, 0, sizeof(*p));
	free(p);
}

char *
p_extend(struct packet *p, size_t diff)
{
	char *buf;

	if (p->p_payload_len + diff >= p->p_allocated_len) {
		p->p_allocated_len *= 4;
		if (p->p_payload_len + diff >= p->p_allocated_len)
			p->p_allocated_len = p->p_payload_len + diff;
		p->p_payload = realloc(p->p_payload, p->p_allocated_len);
		if (p->p_payload == NULL)
			err(1, "realloc");
	}

	buf = p->p_payload + p->p_payload_len;

	p->p_payload_len += diff;
	return (buf);
}

char *
p_read(struct packet *p, int fd, size_t len)
{
	char *buf;
	ssize_t received;

	/*
	 * XXX: Przydałby się mechanizm, żeby czytać tyle, ile jest w sockecie,
	 * 	gdzieś to buforować i zwracać przy kolejnym p_read().
	 */

	buf = p_extend(p, len);
	received = read(fd, buf, len);
	if (received != len)
		err(1, "read from fd %d", fd);

	return (buf);
}

char *
p_peek(struct packet *p, int fd, size_t len)
{
	char *buf;
	ssize_t received;

	/*
	 * XXX: Przydałby się mechanizm, żeby czytać tyle, ile jest w sockecie,
	 * 	gdzieś to buforować i zwracać przy kolejnym p_read().
	 */

	buf = p_extend(p, len);
	received = recv(fd, buf, len, MSG_PEEK);
	if (received < 0 && errno != EAGAIN)
		err(1, "peek from fd %d", fd);

	/*
	 * Undo part of p_extend().
	 */
	p->p_payload_len -= len;

	if ((size_t)received == len)
		return (buf);

	return (NULL);
}

void
p_write(struct packet *p, int fd)
{
	ssize_t ret;

	ret = send(fd, p->p_payload, p->p_payload_len, 0);
	if (ret < 0 || (size_t)ret != p->p_payload_len)
		err(1, "send to fd %d", fd);
}

