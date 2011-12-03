#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ofproto.h"
#include "matlab.h"

#define	OFP_PORT	6633
#define	MATLAB_PORT	3366

static int ofp_listening_socket, matlab_listening_socket;

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

static int
fd_add(int fd, fd_set *fdset, int nfds)
{
	FD_SET(fd, fdset);
	if (fd > nfds)
		nfds = fd;
	return (nfds);
}

static void
usage(void)
{

	printf("usage: ofx controller-ip [controller-port]");
	exit(0);
}

int
main(int argc, char **argv)
{
	fd_set fdset;
	int error, i, nfds, switch_fd, controller_fd, controller_port;
	const char *controller_ip;
	bool handled;

	if (argc < 1 || argc > 2)
		usage();

	controller_ip = argv[1];
	if (argc == 2) {
		controller_port = atoi(argv[2]);
		if (controller_port <= 0 || controller_port > 65535)
			errx(1, "invalid port number");
	} else
		controller_port = OFP_PORT;


	ofp_listening_socket = listen_on(OFP_PORT);
	matlab_listening_socket = listen_on(MATLAB_PORT);

	for (;;) {
		FD_ZERO(&fdset);
		nfds = 0;
		nfds = fd_add(ofp_listening_socket, &fdset, nfds);
		nfds = fd_add(matlab_listening_socket, &fdset, nfds);
		error = select(nfds + 1, &fdset, NULL, NULL, NULL);
		if (error <= 0)
			err(1, "select");

		if (FD_ISSET(ofp_listening_socket, &fdset)) {
			switch_fd = accept(ofp_listening_socket, NULL, 0);
			if (switch_fd < 0)
				err(1, "accept");
			controller_fd = connect_to(controller_ip, controller_port);
			ofproto_new_switch(switch_fd, controller_fd);
			continue;
		}

		if (FD_ISSET(matlab_listening_socket, &fdset)) {
			matlab_new_connection(matlab_listening_socket);
			continue;
		}

		for (i = 0; i < nfds; i++) {
			if (!FD_ISSET(i, &fdset))
				continue;
			handled = ofproto_handle(i);
			if (!handled)
				handled = matlab_handle(i);
			if (!handled)
				errx(1, "unknown fd");
		}
		if (i == nfds)
			errx(1, "fd not found");
	}

	return (0);
}

