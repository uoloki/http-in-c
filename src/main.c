#include "server.h"

int main(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) return 1; // Init Winsock 2.2

    SOCKET sfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,    // IPv4
        .sin_addr.s_addr = INADDR_ANY, // bind to all interfaces
        .sin_port        = htons(PORT) // host‑to‑network‑short
    };

    bind(sfd, (struct sockaddr*)&addr, sizeof addr); // attach to port
    listen(sfd, 8);                                  // queue up to 8 pending conns

    printf("Listening on :%d …\n", PORT);
    while (1) {
        SOCKET cfd = accept(sfd, NULL, NULL);
        if (cfd == INVALID_SOCKET) continue;        // ignore failed accepts
        handle(cfd);                                // single‑threaded demo
    }

    WSACleanup();
}
