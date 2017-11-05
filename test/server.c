/*
 * UDP Server for test (server.c)
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
	struct sockaddr_in server = {0};
	struct sockaddr_in client = {0};
	unsigned int len;
	char buf[1024];
	char str[16];
	uint16_t port = 12345;
	int sock;
	int n_recv;

	if ((sock = socket(AF_TEST, SOCK_DGRAM, 0)) < 0) {
		perror("[server] socket");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("[server] bind");
		exit(EXIT_FAILURE);
	}

	while (1) {
		memset(buf, 0, sizeof(buf));
		len = sizeof(client);
		if ((n_recv = recvfrom(sock, buf, sizeof(buf)-1, 0, (struct sockaddr *)&client, &len)) < 0) {
			perror("[server] recvfrom");
			exit(EXIT_FAILURE);
		}
		inet_ntop(AF_INET, &client.sin_addr, str, sizeof(str));

		printf("[server] recvfrom : %s, port=%d\n", str, ntohs(client.sin_port));
		printf("[server] Received : %s\n", buf);
	}

	close(sock);

	return 0;
}
