#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 4096
#define REQ_SIZE 16384

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_host_or_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *host = argv[1];
    const char *port = argv[2];

    struct addrinfo hints, *res, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int gai = getaddrinfo(host, port, &hints, &res);
    if (gai != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
        return EXIT_FAILURE;
    }

    int sockfd = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);

    if (sockfd == -1) {
        fprintf(stderr, "Could not connect to %s:%s\n", host, port);
        return EXIT_FAILURE;
    }

    printf("Connected to %s:%s\n", host, port);
    printf("Enter HTTP request lines, finish with empty line.\n");

    char request[REQ_SIZE];
    size_t used = 0;
    request[0] = '\0';

    while (1) {
        char line[1024];
        if (fgets(line, sizeof(line), stdin) == NULL) break;

        if (strcmp(line, "\n") == 0 || strcmp(line, "\r\n") == 0) {
            if (used + 2 >= sizeof(request)) {
                fprintf(stderr, "Request too large\n");
                close(sockfd);
                return EXIT_FAILURE;
            }
            strcat(request, "\r\n");
            used += 2;
            break;
        }

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        char with_crlf[1100];
        int n = snprintf(with_crlf, sizeof(with_crlf), "%s\r\n", line);
        if (n < 0 || (size_t)n >= sizeof(with_crlf)) {
            fprintf(stderr, "Line too long\n");
            close(sockfd);
            return EXIT_FAILURE;
        }

        if (used + (size_t)n >= sizeof(request)) {
            fprintf(stderr, "Request too large\n");
            close(sockfd);
            return EXIT_FAILURE;
        }

        strcat(request, with_crlf);
        used += (size_t)n;
    }

    if (used == 0) {
        fprintf(stderr, "No request entered\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    ssize_t sent = send(sockfd, request, used, 0);
    if (sent < 0) die("send");

    printf("\n--- Sent ---\n");
    fwrite(request, 1, used, stdout);
    printf("\n--- Response ---\n");

    char buf[BUF_SIZE];
    while (1) {
        ssize_t r = recv(sockfd, buf, sizeof(buf), 0);
        if (r < 0) die("recv");
        if (r == 0) break;
        fwrite(buf, 1, (size_t)r, stdout);
    }

    printf("\n--- Connection closed ---\n");
    close(sockfd);
    return EXIT_SUCCESS;
}
