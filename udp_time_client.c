#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 1024

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_host_or_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int gai = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (gai != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
        return EXIT_FAILURE;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) die("socket");

    const char *start = "START";
    if (sendto(sock, start, strlen(start), 0, res->ai_addr, res->ai_addrlen) < 0) {
        die("sendto");
    }

    while (1) {
        char buf[BUF_SIZE];
        ssize_t r = recvfrom(sock, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (r < 0) die("recvfrom");
        buf[r] = '\0';
        printf("%s", buf);
        fflush(stdout);
        if (strstr(buf, "Au revoir") != NULL) break;
    }

    freeaddrinfo(res);
    close(sock);
    return EXIT_SUCCESS;
}
