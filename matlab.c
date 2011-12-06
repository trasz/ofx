#include <sys/types.h>
#include <sys/uio.h>
#include <ctype.h>
#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "array.h"
#include "control.h"
#include "debug.h"
#include "matlab.h"
#include "oflink.h"
#include "ofport.h"
#include "ofswitch.h"
#include "packet.h"

struct matlabs matlabs = TAILQ_HEAD_INITIALIZER(matlabs);

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
matlab_find_by_fd(int fd)
{
	struct matlab *matlab;

	TAILQ_FOREACH(matlab, &matlabs, matlab_next) {
		if (fd == matlab->matlab_fd)
			return (matlab);
	}

	return (NULL);
}

static void
matlab_print(struct matlab *matlab, const char *fmt, ...)
{
	va_list ap;
	char *str;

	va_start(ap, fmt);
	vasprintf(&str, fmt, ap);
	va_end(ap);

	if (str == NULL)
		err(1, "vasprintf");


	write(matlab->matlab_fd, str, strlen(str));
	free(str);
}

static int
matlab_atoi(struct matlab *matlab, const char *arg, const char *desc)
{
	int n;
	char *end;

	if (arg == NULL) {
		matlab_print(matlab, "Missing %s.\n", desc);
		return (-1);
	}

	n = strtol(arg, &end, 10);
	if (end[0] != '\0') {
		matlab_print(matlab, "\"%s\" is not a valid %s.\n", arg, desc);
		return (-1);
	}

	if (n < 0) {
		matlab_print(matlab, "The %s must be greater than zero.\n",  desc);
		return (-1);
	}

	return (n);
}

static void
matlab_switches(struct matlab *matlab, const char *arg1, const char *arg2)
{
	struct array *a;
	struct ofswitch *ofs;
	char *str;

	a = a_alloc();
	TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
		a_add_int(a, ofs->ofs_number);
	}
	str = a_str(a);
	a_free(a);
	matlab_print(matlab, str);
	free(str);
}

static void
matlab_topology_one(struct matlab *matlab, struct ofport *ofp)
{
	struct array *a;
	struct ofport *ofp2;

	TAILQ_FOREACH(ofp2, &ofp->ofp_link->ofl_ports, ofp_next_link) {
		a = a_alloc();
		a_add_int(a, ofp->ofp_switch->ofs_number);
		a_add_int(a, ofp->ofp_number);
		a_add_int(a, ofp2->ofp_switch->ofs_number);
		a_add_int(a, ofp2->ofp_number);
		matlab_print(matlab, a_str(a));
		a_free(a);
	}
}

static void
matlab_topology(struct matlab *matlab, const char *arg1, const char *arg2)
{
	struct ofswitch *ofs;
	struct ofport *ofp;
	int switch_no, port_no;

	if (arg1 != NULL) {
		switch_no = matlab_atoi(matlab, arg1, "switch number");
		if (switch_no < 0)
			return;

		ofs = ofs_find_by_number(switch_no);
		if (ofs == NULL) {
			matlab_print(matlab, "Invalid switch number.\n");
			return;
		}

		if (arg2 != NULL) {
			port_no = matlab_atoi(matlab, arg1, "port number");
			if (port_no < 0)
				return;

			ofp = ofp_find_by_number(ofs, port_no);
			if (ofp == NULL) {
				matlab_print(matlab, "Invalid port number.\n");
				return;
			}

			matlab_topology_one(matlab, ofp);
		} else {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				matlab_topology_one(matlab, ofp);
		}
	} else {
		TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				matlab_topology_one(matlab, ofp);
		}
	}
}

static void
matlab_status_one(struct matlab *matlab, struct ofport *ofp)
{
	struct array *a;

	a = a_alloc();
	a_add_int(a, ofp->ofp_switch->ofs_number);
	a_add_int(a, ofp->ofp_number);
	a_add_int(a, ofp->ofp_config);
	a_add_int(a, ofp->ofp_state);
	a_add_int(a, ofp->ofp_curr);
	a_add_int(a, ofp->ofp_advertised);
	a_add_int(a, ofp->ofp_supported);
	a_add_int(a, ofp->ofp_peer);
	matlab_print(matlab, a_str(a));
	a_free(a);
}

static void
matlab_status(struct matlab *matlab, const char *arg1, const char *arg2)
{
	struct ofswitch *ofs;
	struct ofport *ofp;
	int switch_no, port_no;

	if (arg1 != NULL) {
		switch_no = matlab_atoi(matlab, arg1, "switch number");
		if (switch_no < 0)
			return;

		ofs = ofs_find_by_number(switch_no);
		if (ofs == NULL) {
			matlab_print(matlab, "Invalid switch number.\n");
			return;
		}

		if (arg2 != NULL) {
			port_no = matlab_atoi(matlab, arg1, "port number");
			if (port_no < 0)
				return;

			ofp = ofp_find_by_number(ofs, port_no);
			if (ofp == NULL) {
				matlab_print(matlab, "Invalid port number.\n");
				return;
			}

			matlab_status_one(matlab, ofp);
		} else {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				matlab_status_one(matlab, ofp);
		}
	} else {
		TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				matlab_status_one(matlab, ofp);
		}
	}
}

static void
matlab_stats_one(struct matlab *matlab, struct ofport *ofp)
{
	struct array *a;

	a = a_alloc();
	a_add_int(a, ofp->ofp_switch->ofs_number);
	a_add_int(a, ofp->ofp_number);
	a_add_int(a, ofp->ofp_rx_packets);
	a_add_int(a, ofp->ofp_tx_packets);
	a_add_int(a, ofp->ofp_rx_bytes);
	a_add_int(a, ofp->ofp_tx_bytes);
	a_add_int(a, ofp->ofp_rx_dropped);
	a_add_int(a, ofp->ofp_tx_dropped);
	a_add_int(a, ofp->ofp_rx_errors);
	a_add_int(a, ofp->ofp_tx_errors);
	a_add_int(a, ofp->ofp_rx_frame_err);
	a_add_int(a, ofp->ofp_rx_over_err);
	a_add_int(a, ofp->ofp_rx_crc_err);
	a_add_int(a, ofp->ofp_collisions);
	matlab_print(matlab, a_str(a));
	a_free(a);
}

static void
matlab_stats(struct matlab *matlab, const char *arg1, const char *arg2)
{
	struct ofswitch *ofs;
	struct ofport *ofp;
	int switch_no, port_no;

	if (arg1 != NULL) {
		switch_no = matlab_atoi(matlab, arg1, "switch number");
		if (switch_no < 0)
			return;

		ofs = ofs_find_by_number(switch_no);
		if (ofs == NULL) {
			matlab_print(matlab, "Invalid switch number.\n");
			return;
		}

		if (arg2 != NULL) {
			port_no = matlab_atoi(matlab, arg1, "port number");
			if (port_no < 0)
				return;

			ofp = ofp_find_by_number(ofs, port_no);
			if (ofp == NULL) {
				matlab_print(matlab, "Invalid port number.\n");
				return;
			}

			matlab_stats_one(matlab, ofp);
		} else {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				matlab_stats_one(matlab, ofp);
		}
	} else {
		TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				matlab_stats_one(matlab, ofp);
		}
	}
}

static void
matlab_port_up(struct matlab *matlab, const char *arg1, const char *arg2)
{
	int switch_no, port_no;
	struct ofswitch *ofs;
	struct ofport *ofp;

	switch_no = matlab_atoi(matlab, arg1, "switch number");
	port_no = matlab_atoi(matlab, arg2, "port number");
	if (switch_no < 0 || port_no < 0)
		return;

	ofs = ofs_find_by_number(switch_no);
	if (ofs == NULL) {
		matlab_print(matlab, "Invalid switch number.\n");
		return;
	}
	ofp = ofp_find_by_number(ofs, port_no);
	if (ofp == NULL) {
		matlab_print(matlab, "Invalid port number.\n");
		return;
	}
	control_port_up(ofp);
}

static void
matlab_port_down(struct matlab *matlab, const char *arg1, const char *arg2)
{
	int switch_no, port_no;
	struct ofswitch *ofs;
	struct ofport *ofp;

	switch_no = matlab_atoi(matlab, arg1, "switch number");
	port_no = matlab_atoi(matlab, arg2, "port number");
	if (switch_no < 0 || port_no < 0)
		return;

	ofs = ofs_find_by_number(switch_no);
	if (ofs == NULL) {
		matlab_print(matlab, "Invalid switch number.\n");
		return;
	}
	ofp = ofp_find_by_number(ofs, port_no);
	if (ofp == NULL) {
		matlab_print(matlab, "Invalid port number.\n");
		return;
	}
	control_port_down(ofp);
}

static void
matlab_poll(struct matlab *matlab, const char *arg1, const char *arg2)
{
}

static void
matlab_help(struct matlab *matlab, const char *arg1, const char *arg2)
{

	matlab_print(matlab, "Available commands: switches, topology, status, stats, poll, help.\n");
}

static void
matlab_unrecognized(struct matlab *matlab, const char *arg1, const char *arg2)
{

	matlab_print(matlab, "Unrecognized command; try 'help'.\n");
}

struct matlab_cmd {
	const char	*mc_name;
	void		(*mc_f)(struct matlab *, const char *arg1, const char *arg2);
};

static const struct matlab_cmd matlab_cmds[] = {
	{ "switches", matlab_switches },
	{ "topology", matlab_topology },
	{ "status", matlab_status },
	{ "stats", matlab_stats },
	{ "port-up", matlab_port_up },
	{ "port-down", matlab_port_down },
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
	char *cmd, *arg1, *arg2;

	/*
	 * Gromadzimy kolejne znaki do momentu odebrania '\n'.
	 */
	if (matlab->matlab_p == NULL)
		matlab->matlab_p = p_alloc();
	p = matlab->matlab_p;
	p_read(p, fd, 1);
	ch = p->p_payload[p->p_payload_len - 1];
	if (ch != '\n' && ch != '\r') {
		debug("char %c, line not finished", ch);
		return;
	}

	/*
	 * Wywalamy newline.
	 */
	p->p_payload[p->p_payload_len - 1] = '\0';

	debug("got matlab command \"%s\"\n", p->p_payload);

	cmd = p->p_payload;
	arg1 = arg2 = NULL;

	/*
	 * Zmieniamy pierwszą spację na zero.
	 */
	for (i = 0; i < p->p_payload_len; i++) {
		if (isspace(p->p_payload[i])) {
			p->p_payload[i] = '\0';
			arg1 = p->p_payload + i;
			break;
		}
	}

	for (mc = matlab_cmds; mc->mc_name != NULL; mc++) {
		if (strcasecmp(mc->mc_name, cmd) != 0)
			continue;
		mc->mc_f(matlab, arg1, arg2);
		break;
	}

	if (mc->mc_name == NULL)
		matlab_unrecognized(matlab, arg1, arg2);

	p_free(matlab->matlab_p);
	matlab->matlab_p = NULL;
}
