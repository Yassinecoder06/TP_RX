#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1024

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
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) die("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) die("bind");

    struct sockaddr_in cliaddr;
    socklen_t cli_len = sizeof(cliaddr);
    char buf[BUF_SIZE];

    ssize_t r = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr *)&cliaddr, &cli_len);
    if (r < 0) die("recvfrom");

    for (int i = 1; i <= 60; i++) {
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_r(&now, &tm_now);

        char msg[128];
        int n = snprintf(msg, sizeof(msg), "Message %02d: Il est %02d:%02d:%02d !\n",
                         i, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
        if (n > 0) {
            sendto(sock, msg, (size_t)n, 0, (struct sockaddr *)&cliaddr, cli_len);
        }
        sleep(1);
    }

    sendto(sock, "Au revoir\n", 10, 0, (struct sockaddr *)&cliaddr, cli_len);
    close(sock);
    return EXIT_SUCCESS;
}
