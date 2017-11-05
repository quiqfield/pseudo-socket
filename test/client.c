/*
 * UDP Client for test (client.c)
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "test.h"

int main(int argc, char **argv)
{
	struct sockaddr_in addr = {0};
	char *dst_ip = "127.0.0.1";
	uint16_t port = 12345;
	int sock;

	/* without connect() */
	if ((sock = socket(AF_TEST, SOCK_DGRAM, 0)) < 0) {
		perror("[client1] socket");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(dst_ip);
	addr.sin_port = htons(port);

	if (sendto(sock, "HELLO", 5, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("[client1] sendto");
	}

	close(sock);


	/* with connect() */
	if ((sock = socket(AF_TEST, SOCK_DGRAM, 0)) < 0) {
		perror("[client2] socket");
		exit(EXIT_FAILURE);
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(dst_ip);
	addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("[client2] connect");
	}
	if (send(sock, "HELLO with connect()", 20, 0) < 0) {
		perror("[client2] send");
	}

	close(sock);

	return 0;
}
