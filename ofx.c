#include <sys/types.h>
#include <sys/queue.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "ofproto.h"
#include "ofport.h"
#include "ofswitch.h"
#include "matlab.h"

#define	OFP_PORT	6633
#define	MATLAB_PORT	3366

static int ofp_listening_socket, matlab_listening_socket;

TAILQ_HEAD(, ofswitch)		ofswitches;
TAILQ_HEAD(, matlab)		matlabs;

static int
listen_on(int port)
{
	struct sockaddr_in sin;
	int sock, error;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		err(1, "socket");

	sin.sin_family = AF_INET;
	sin.sin_port = port;
	sin.sin_addr.s_addr = INADDR_ANY;

	error = bind(sock, (struct sockaddr *)&sin, sizeof(sin));
	if (error != 0)
		err(1, "bind");

	error = listen(sock, 42);
	if (error != 0)
		err(1, "listen");

	return (sock);
}

static int
connect_to(const char *ip, int port)
{
	struct sockaddr_in sin;
	int sock, error;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		err(1, "socket");

	sin.sin_family = AF_INET;
	sin.sin_port = port;
	sin.sin_addr.s_addr = INADDR_ANY;

	if (inet_pton(AF_INET, ip, &sin.sin_addr) != 1)
		err(1, "inet_pton");

	error = connect(sock, (struct sockaddr *)&sin, sizeof(sin));
	if (error != 0)
		err(1, "connect");

	return (sock);
}

static void
ofs_add(int switch_fd, int controller_fd)
{
	struct ofswitch *ofs;

	ofs = calloc(sizeof(*ofs), 1);
	if (ofs == NULL)
		err(1, "calloc");

	ofs->ofs_switch_fd = switch_fd;
	ofs->ofs_controller_fd = controller_fd;

	TAILQ_INSERT_TAIL(&ofswitches, ofs, ofs_next);
}

static struct ofswitch *
ofs_find(int fd)
{
	struct ofswitch *ofs;

	TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
		if (fd == ofs->ofs_switch_fd || fd == ofs->ofs_controller_fd)
			return (ofs);
	}

	return (NULL);
}

void
ofs_add_port(struct ofswitch *ofs, struct ofport *ofp)
{

	TAILQ_INSERT_TAIL(&ofs->ofs_ports, ofp, ofp_next);
}

static int
fd_add(int fd, fd_set *fdset, int nfds)
{
	FD_SET(fd, fdset);
	if (fd > nfds)
		nfds = fd;
	return (nfds);
}

static void
matlab_add(int matlab_fd)
{
	struct matlab *matlab;

	matlab = calloc(sizeof(*matlab), 1);
	if (matlab == NULL)
		err(1, "calloc");

	matlab->matlab_fd = matlab_fd;

	TAILQ_INSERT_TAIL(&matlabs, matlab, matlab_next);
}

static struct matlab *
matlab_find(int fd)
{
	struct matlab *matlab;

	TAILQ_FOREACH(matlab, &matlabs, matlab_next) {
		if (fd == matlab->matlab_fd)
			return (matlab);
	}

	return (NULL);
}

static void
usage(void)
{

	printf("usage: ofx controller-ip [controller-port]\n");
	exit(0);
}

int
main(int argc, char **argv)
{
	fd_set fdset;
	int error, i, nfds, switch_fd, controller_fd, matlab_fd, controller_port;
	const char *controller_ip;
	struct ofswitch *ofs;
	struct matlab *matlab;

	if (argc < 2 || argc > 3)
		usage();

	controller_ip = argv[1];
	if (inet_addr(controller_ip) == INADDR_NONE)
		errx(1, "invalid ip address");
	if (argc == 3) {
		controller_port = atoi(argv[2]);
		if (controller_port <= 0 || controller_port > 65535)
			errx(1, "invalid port number");
	} else
		controller_port = OFP_PORT;

	ofp_listening_socket = listen_on(OFP_PORT);
	matlab_listening_socket = listen_on(MATLAB_PORT);

	debug("listening for switches on port %d; "
	    "connecting to controller at %s:%d\n",
	    OFP_PORT, controller_ip, controller_port);
	debug("listening for MATLAB at port %d\n", MATLAB_PORT);

	for (;;) {
		FD_ZERO(&fdset);
		nfds = 0;
		nfds = fd_add(ofp_listening_socket, &fdset, nfds);
		nfds = fd_add(matlab_listening_socket, &fdset, nfds);
		TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
			nfds = fd_add(ofs->ofs_switch_fd, &fdset, nfds);
			nfds = fd_add(ofs->ofs_controller_fd, &fdset, nfds);
		}
		error = select(nfds + 1, &fdset, NULL, NULL, NULL);
		if (error <= 0)
			err(1, "select");

		if (FD_ISSET(ofp_listening_socket, &fdset)) {
			switch_fd = accept(ofp_listening_socket, NULL, 0);
			if (switch_fd < 0)
				err(1, "accept");
			debug("fd %d: got new switch\n", switch_fd);
			controller_fd = connect_to(controller_ip,
			    controller_port);
			ofs_add(switch_fd, controller_fd);
			debug("fd %d: connected to the controller\n",
			    controller_fd);
			continue;
		}

		if (FD_ISSET(matlab_listening_socket, &fdset)) {
			matlab_fd = accept(matlab_listening_socket, NULL, 0);
			debug("fd %d: got new MATLAB client\n", matlab_fd);
			if (matlab_fd < 0)
				err(1, "accept");
			matlab_add(matlab_fd);
			continue;
		}

		for (i = 0; i < nfds; i++) {
			if (!FD_ISSET(i, &fdset))
				continue;

			ofs = ofs_find(i);
			if (ofs != NULL) {
				ofproto_handle(ofs, i);
				continue;
			}

			matlab = matlab_find(i);
			if (matlab != NULL) {
				matlab_handle(matlab, i);
				continue;
			}
			errx(1, "unknown fd");
		}
		if (i == nfds)
			errx(1, "fd not found");
	}

	return (0);
}

