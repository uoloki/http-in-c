#include "server.h"

int main(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) return 1; // Init Winsock 2.2

    SOCKET sfd = socket(AF_INET, SOCK_STREAM, 0);
    /* Server File Descriptor, stays same for the lifetime of the program */

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,    // IPv4
        .sin_addr.s_addr = INADDR_ANY, // bind to all interfaces
        .sin_port        = htons(PORT) // host‑to‑network‑short
    };

    bind(sfd, (struct sockaddr*)&addr, sizeof addr); // attach to port
    listen(sfd, 8);                                  // queue up to 8 pending conns

    sem = CreateSemaphore(NULL, 0, QSZ, NULL); 
    /* 
    In C programming on Windows, a semaphore is a synchronization object used to 
    control access to a shared resource by multiple threads or processes. 
    It functions as a counter that tracks the number of available 
    "slots" or "permits" for that resource. 
    
    HANDLE hSemaphore = CreateSemaphore(
        NULL, // Default security attributes
        initialCount, // Initial count
        maximumCount, // Maximum count
        NULL // Unnamed semaphore
    ); - google 
    */

    for (int i = 0; i < WORKERS; ++i) CreateThread(NULL, 0, worker_proc, NULL, 0, NULL);

    printf("Listening on :%d …\n", PORT);
    while (1) {
        SOCKET cfd = accept(sfd, NULL, NULL);
        if (cfd == INVALID_SOCKET) continue;        // ignore failed accepts
        q_push(cfd);                                
    }

    WSACleanup();
}
