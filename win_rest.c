#define _WIN32_WINNT 0x0601    
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")   

#define PORT 8080
#define BUFSZ 4096

void handle(int cfd) {
    char buf[BUFSZ+1] = {0};
    int n = recv(cfd, buf, BUFSZ, 0);
    if (n <= 0) { closesocket(cfd); return; }

    char method[8], path[256];
    if (sscanf(buf, "%7s %255s", method, path) != 2) {
        closesocket(cfd); return;
    }



    const char *body, *status;
    if (!strcmp(method, "GET") && !strcmp(path, "/hello")) {
        body = "{\"msg\":\"world\"}";
        status = "HTTP/1.1 200 OK\r\n";
        printf("GET /hello\n");
    } else {
        body = "{\"error\":\"not found\"}";
        status = "HTTP/1.1 404 Not Found\r\n";
    }

    char hdr[256];
    int len = snprintf(hdr, sizeof hdr,
            "%sContent-Type: application/json\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            status, strlen(body));
    send(cfd, hdr, len, 0);
    send(cfd, body, strlen(body), 0);
    closesocket(cfd);
}

int main(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa)) return 1;

    SOCKET sfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(sfd, (struct sockaddr*)&addr, sizeof addr);
    listen(sfd, 8);

    printf("Listening on :%d …\n", PORT);
    while (1) {
        SOCKET cfd = accept(sfd, NULL, NULL);
        if (cfd == INVALID_SOCKET) continue;
        handle(cfd);   /* single‑threaded demo */
    }
    WSACleanup();
}
