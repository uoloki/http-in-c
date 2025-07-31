#ifndef SERVER_H
#define SERVER_H

#define _WIN32_WINNT 0x0601        // Target Windows 7+ APIs
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#define PORT 8080                  // Port the server listens on
#define BUFSZ 4096                 // Receive‑buffer size
#define HDR_END "\r\n\r\n"

#define WORKERS 8
#define QSZ     1024

static HANDLE sem;                        // kernel semaphore = #items in
static void q_push(SOCKET s);
DWORD WINAPI worker_proc(void *unused);
const char *get_user_agent(const char *buf);
void handle(SOCKET cfd);

#endif /* SERVER_H */
