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
mat_add(int mat_fd)
{
	struct mat *mat;

	mat = calloc(sizeof(*mat), 1);
	if (mat == NULL)
		err(1, "calloc");

	mat->mat_fd = mat_fd;

	TAILQ_INSERT_TAIL(&matlabs, mat, mat_next);
}

struct mat *
mat_find_by_fd(int fd)
{
	struct mat *mat;

	TAILQ_FOREACH(mat, &matlabs, mat_next) {
		if (fd == mat->mat_fd)
			return (mat);
	}

	return (NULL);
}

static void
mat_print(struct mat *mat, const char *fmt, ...)
{
	va_list ap;
	char *str;

	va_start(ap, fmt);
	vasprintf(&str, fmt, ap);
	va_end(ap);

	if (str == NULL)
		err(1, "vasprintf");


	write(mat->mat_fd, str, strlen(str));
	free(str);
}

static int
mat_atoi(struct mat *mat, const char *arg, const char *desc)
{
	int n;
	char *end;

	if (arg == NULL) {
		mat_print(mat, "Missing %s.\n", desc);
		return (-1);
	}

	n = strtol(arg, &end, 10);
	if (end[0] != '\0') {
		mat_print(mat, "\"%s\" is not a valid %s.\n", arg, desc);
		return (-1);
	}

	if (n < 0) {
		mat_print(mat, "The %s must be greater than zero.\n",  desc);
		return (-1);
	}

	return (n);
}

static void
mat_switches(struct mat *mat, const char *arg1, const char *arg2)
{
	struct array *a;
	struct ofswitch *ofs;
	char *str;

	a = a_alloc();
	TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
		a_add_int(a, ofs->ofs_number);
		a_add_int(a, ofs->ofs_datapath_id);
	}
	str = a_str(a);
	a_free(a);
	mat_print(mat, str);
	free(str);
}

static void
mat_topology_one(struct mat *mat, struct ofport *ofp)
{
	struct array *a;
	struct ofport *ofp2;

	TAILQ_FOREACH(ofp2, &ofp->ofp_link->ofl_ports, ofp_next_link) {
		a = a_alloc();
		a_add_int(a, ofp->ofp_switch->ofs_number);
		a_add_int(a, ofp->ofp_number);
		a_add_int(a, ofp2->ofp_switch->ofs_number);
		a_add_int(a, ofp2->ofp_number);
		mat_print(mat, a_str(a));
		a_free(a);
	}
}

static void
mat_topology(struct mat *mat, const char *arg1, const char *arg2)
{
	struct ofswitch *ofs;
	struct ofport *ofp;
	int switch_no, port_no;

	if (arg1 != NULL) {
		switch_no = mat_atoi(mat, arg1, "switch number");
		if (switch_no < 0)
			return;

		ofs = ofs_find_by_number(switch_no);
		if (ofs == NULL) {
			mat_print(mat, "Invalid switch number.\n");
			return;
		}

		if (arg2 != NULL) {
			port_no = mat_atoi(mat, arg1, "port number");
			if (port_no < 0)
				return;

			ofp = ofp_find_by_number(ofs, port_no);
			if (ofp == NULL) {
				mat_print(mat, "Invalid port number.\n");
				return;
			}

			mat_topology_one(mat, ofp);
		} else {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				mat_topology_one(mat, ofp);
		}
	} else {
		TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				mat_topology_one(mat, ofp);
		}
	}
}

static void
mat_status_one(struct mat *mat, struct ofport *ofp)
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
	mat_print(mat, a_str(a));
	a_free(a);
}

static void
mat_status(struct mat *mat, const char *arg1, const char *arg2)
{
	struct ofswitch *ofs;
	struct ofport *ofp;
	int switch_no, port_no;

	if (arg1 != NULL) {
		switch_no = mat_atoi(mat, arg1, "switch number");
		if (switch_no < 0)
			return;

		ofs = ofs_find_by_number(switch_no);
		if (ofs == NULL) {
			mat_print(mat, "Invalid switch number.\n");
			return;
		}

		if (arg2 != NULL) {
			port_no = mat_atoi(mat, arg1, "port number");
			if (port_no < 0)
				return;

			ofp = ofp_find_by_number(ofs, port_no);
			if (ofp == NULL) {
				mat_print(mat, "Invalid port number.\n");
				return;
			}

			mat_status_one(mat, ofp);
		} else {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				mat_status_one(mat, ofp);
		}
	} else {
		TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				mat_status_one(mat, ofp);
		}
	}
}

static void
mat_stats_one(struct mat *mat, struct ofport *ofp)
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
	mat_print(mat, a_str(a));
	a_free(a);
}

static void
mat_stats(struct mat *mat, const char *arg1, const char *arg2)
{
	struct ofswitch *ofs;
	struct ofport *ofp;
	int switch_no, port_no;

	if (arg1 != NULL) {
		switch_no = mat_atoi(mat, arg1, "switch number");
		if (switch_no < 0)
			return;

		ofs = ofs_find_by_number(switch_no);
		if (ofs == NULL) {
			mat_print(mat, "Invalid switch number.\n");
			return;
		}

		if (arg2 != NULL) {
			port_no = mat_atoi(mat, arg1, "port number");
			if (port_no < 0)
				return;

			ofp = ofp_find_by_number(ofs, port_no);
			if (ofp == NULL) {
				mat_print(mat, "Invalid port number.\n");
				return;
			}

			mat_stats_one(mat, ofp);
		} else {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				mat_stats_one(mat, ofp);
		}
	} else {
		TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
			TAILQ_FOREACH(ofp, &ofs->ofs_ports, ofp_next)
				mat_stats_one(mat, ofp);
		}
	}
}

static void
mat_port_up(struct mat *mat, const char *arg1, const char *arg2)
{
	int switch_no, port_no;
	struct ofswitch *ofs;
	struct ofport *ofp;

	switch_no = mat_atoi(mat, arg1, "switch number");
	port_no = mat_atoi(mat, arg2, "port number");
	if (switch_no < 0 || port_no < 0)
		return;

	ofs = ofs_find_by_number(switch_no);
	if (ofs == NULL) {
		mat_print(mat, "Invalid switch number.\n");
		return;
	}
	ofp = ofp_find_by_number(ofs, port_no);
	if (ofp == NULL) {
		mat_print(mat, "Invalid port number.\n");
		return;
	}
	control_port_up(ofp);
}

static void
mat_port_down(struct mat *mat, const char *arg1, const char *arg2)
{
	int switch_no, port_no;
	struct ofswitch *ofs;
	struct ofport *ofp;

	switch_no = mat_atoi(mat, arg1, "switch number");
	port_no = mat_atoi(mat, arg2, "port number");
	if (switch_no < 0 || port_no < 0)
		return;

	ofs = ofs_find_by_number(switch_no);
	if (ofs == NULL) {
		mat_print(mat, "Invalid switch number.\n");
		return;
	}
	ofp = ofp_find_by_number(ofs, port_no);
	if (ofp == NULL) {
		mat_print(mat, "Invalid port number.\n");
		return;
	}
	control_port_down(ofp);
}

static void
mat_poll(struct mat *mat, const char *arg1, const char *arg2)
{
}

static void
mat_help(struct mat *mat, const char *arg1, const char *arg2)
{

	mat_print(mat, "Available commands: switches, topology, status, stats, port-up, port-down, poll, help.\n");
}

static void
mat_unrecognized(struct mat *mat, const char *arg1, const char *arg2)
{

	mat_print(mat, "Unrecognized command; try 'help'.\n");
}

struct mat_cmd {
	const char	*mc_name;
	void		(*mc_f)(struct mat *, const char *arg1, const char *arg2);
};

static const struct mat_cmd mat_cmds[] = {
	{ "switches", mat_switches },
	{ "topology", mat_topology },
	{ "status", mat_status },
	{ "stats", mat_stats },
	{ "port-up", mat_port_up },
	{ "port-down", mat_port_down },
	{ "poll", mat_poll },
	{ "help", mat_help },
	{ NULL, NULL}};

void
mat_handle(struct mat *mat, int fd)
{
	struct packet *p;
	const struct mat_cmd *mc;
	int i;
	char ch;
	char *cmd, *arg1, *arg2;

	/*
	 * Gromadzimy kolejne znaki do momentu odebrania '\n'.
	 */
	if (mat->mat_p == NULL)
		mat->mat_p = p_alloc();
	p = mat->mat_p;
	p_read(p, fd, 1);
	ch = p->p_payload[p->p_payload_len - 1];
	if (ch != '\n' && ch != '\r')
		return;

	/*
	 * Wywalamy newline.
	 */
	p->p_payload[p->p_payload_len - 1] = '\0';

	cmd = p->p_payload;
	arg1 = arg2 = NULL;

	if (strlen(cmd) == 0) {
		p_free(mat->mat_p);
		mat->mat_p = NULL;
		return;
	}

	debug("got mat command \"%s\"\n", cmd);

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

	for (mc = mat_cmds; mc->mc_name != NULL; mc++) {
		if (strcasecmp(mc->mc_name, cmd) != 0)
			continue;
		mc->mc_f(mat, arg1, arg2);
		break;
	}

	if (mc->mc_name == NULL)
		mat_unrecognized(mat, arg1, arg2);

	p_free(mat->mat_p);
	mat->mat_p = NULL;
}
