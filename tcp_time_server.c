#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 5

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) die("socket");

    int opt = 1;
    if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        die("setsockopt");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);

    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(srv, BACKLOG) < 0) die("listen");

    printf("TCP server listening on port %d\n", port);

    struct sockaddr_in cliaddr;
    socklen_t cli_len = sizeof(cliaddr);
    int cli = accept(srv, (struct sockaddr *)&cliaddr, &cli_len);
    if (cli < 0) die("accept");

    for (int i = 1; i <= 60; i++) {
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_r(&now, &tm_now);

        char msg[128];
        int n = snprintf(msg, sizeof(msg), "Message %02d: Il est %02d:%02d:%02d !\n",
                         i, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
        if (n > 0) {
            if (send(cli, msg, (size_t)n, 0) < 0) {
                perror("send");
                break;
            }
        }
        sleep(1);
    }

    send(cli, "Au revoir\n", 10, 0);
    close(cli);
    close(srv);
    return EXIT_SUCCESS;
}
