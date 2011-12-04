#include <sys/types.h>
#include <sys/uio.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "matlab.h"
#include "packet.h"

struct matlabs matlabs;

void
matlab_add(int matlab_fd)
{
	struct matlab *matlab;

	matlab = calloc(sizeof(*matlab), 1);
	if (matlab == NULL)
		err(1, "calloc");

	matlab->matlab_fd = matlab_fd;

	TAILQ_INSERT_TAIL(&matlabs, matlab, matlab_next);
}

struct matlab *
matlab_find(int fd)
{
	struct matlab *matlab;

	TAILQ_FOREACH(matlab, &matlabs, matlab_next) {
		if (fd == matlab->matlab_fd)
			return (matlab);
	}

	return (NULL);
}


static void matlab_switches(struct matlab *matlab, const char *args)
{
}

static void matlab_topology(struct matlab *matlab, const char *args)
{
}

static void matlab_status(struct matlab *matlab, const char *args)
{
}

static void matlab_stats(struct matlab *matlab, const char *args)
{
}

static void matlab_poll(struct matlab *matlab, const char *args)
{
}

static void matlab_help(struct matlab *matlab, const char *args)
{
	const char msg[] = "Available commands: switches, topology, status, stats, poll, help.\n";

	write(matlab->matlab_fd, msg, sizeof(msg));

}

static void matlab_unrecognized(struct matlab *matlab, const char *args)
{
	const char msg[] = "Unrecognized command; try 'help'.\n";

	write(matlab->matlab_fd, msg, sizeof(msg));
}

struct matlab_cmd {
	const char	*mc_name;
	void		(*mc_f)(struct matlab *, const char *args);
};

static const struct matlab_cmd matlab_cmds[] = {
	{ "switches", matlab_switches },
	{ "topology", matlab_topology },
	{ "status", matlab_status },
	{ "stats", matlab_stats },
	{ "poll", matlab_poll },
	{ "help", matlab_help },
	{ NULL, NULL}};

void
matlab_handle(struct matlab *matlab, int fd)
{
	struct packet *p;
	const struct matlab_cmd *mc;
	int i;
	char ch;
	char *cmd, *args;

	/*
	 * Gromadzimy kolejne znaki do momentu odebrania '\n'.
	 */
	if (matlab->matlab_p == NULL)
		matlab->matlab_p = p_alloc();
	p = matlab->matlab_p;
	p_read(p, fd, 1);
	ch = p->p_payload[p->p_payload_len - 1];
	if (ch != '\n' && ch != '\r')
		return;

	/*
	 * Wywalamy newline.
	 */
	p->p_payload[p->p_payload_len - 1] = '\0';

	cmd = p->p_payload;
	args = NULL;

	/*
	 * Zmieniamy pierwszą spację na zero.
	 */
	for (i = 0; i < p->p_payload_len; i++) {
		if (isspace(p->p_payload[i])) {
			p->p_payload[i] = '\0';
			args = p->p_payload + i;
			break;
		}
	}

	for (mc = matlab_cmds; mc->mc_name != NULL; mc++) {
		if (strcasecmp(mc->mc_name, cmd) != 0)
			continue;
		mc->mc_f(matlab, args);
		break;
	}

	if (mc->mc_name == NULL)
		matlab_unrecognized(matlab, args);

	p_free(matlab->matlab_p);
	matlab->matlab_p = NULL;
}
