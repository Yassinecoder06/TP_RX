#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define BACKLOG 5

static void reap_children(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

static void serve_client(int cli_fd) {
    for (int i = 1; i <= 60; i++) {
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_r(&now, &tm_now);

        char msg[160];
        int n = snprintf(msg, sizeof(msg),
                         "[pid=%d] Message %02d: Il est %02d:%02d:%02d !\n",
                         getpid(), i, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
        if (n > 0) send(cli_fd, msg, (size_t)n, 0);
        sleep(1);
    }
    send(cli_fd, "Au revoir\n", 10, 0);
    close(cli_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGCHLD, reap_children);

    int port = atoi(argv[1]);
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return EXIT_FAILURE;

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) return EXIT_FAILURE;
    if (listen(srv, BACKLOG) < 0) return EXIT_FAILURE;

    while (1) {
        int cli = accept(srv, NULL, NULL);
        if (cli < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            close(srv);
            serve_client(cli);
            _exit(0);
        }
        close(cli);
    }
}
