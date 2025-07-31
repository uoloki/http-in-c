#include "server.h"

#pragma comment(lib, "ws2_32.lib") // Link Winsock automatically

static SOCKET queue[QSZ];                 // ring buffer of client sockets
static LONG   head = 0, tail = 0;         // indices (atomic counters)

void q_push(SOCKET s)
{
    LONG t = _InterlockedIncrement(&tail) - 1;
    /*
    LONG WINAPI InterlockedIncrement(_Inout_ LONG volatile *Addend);
    Locks the operation and adds one increment, returns old+1
    */
    queue[t % QSZ] = s;                         
    /*
    the counter doesn't reset, so we reduce it into array bounds
    just in case
    */
    ReleaseSemaphore(sem, 1, NULL); // hand a permit back -> wake 1 worker
}

DWORD WINAPI worker_proc(void *unused)
{
    for (;;)
    {
        // Sleep until there’s at least 1 item in the queue
        WaitForSingleObject(sem, INFINITE);

        // Increment head so noone will touch the same socket
        LONG h = _InterlockedIncrement(&head) - 1;
        SOCKET s = queue[h % QSZ]; // keep the index inside queue

        // Handle the HTTP transaction
        handle(s);   // handle closes the socket
    }
    return 0;
}

const char *get_user_agent(const char *buf) { 
    char * start = strstr(buf, "User-Agent:");
    if (!start) return NULL;

    char * end   = strstr(start, "\r\n");
    if (!end) return NULL; 

    size_t len   = (size_t)(end - start);
    char *ua     = malloc(len + 1);
    if (!ua) return NULL;

    memcpy(ua, start, len);
    ua[len]      = '\0';

    return ua;  
} 

void handle(SOCKET cfd) {
    char buf[BUFSZ + 1] = {0};                     // +1 for NUL‑terminator
    size_t used = 0;
    while (used < BUFSZ) {
        int n = recv(cfd, buf+used, BUFSZ - used, 0);    
    /*  recv — pull raw bytes from the socket
        cfd   : client “file descriptor” (socket handle)
        buf   : destination buffer
        BUFSZ : max bytes to copy
        0     : default flags (blocking)
        n     : bytes actually read, up to 0 means EOF/error            */

        if (n <= 0) { closesocket(cfd); return; };
        used += n;
        
        if (used >= 4 && strstr(buf, HDR_END)) break; 
    };
    
    char * hdr_end = strstr(buf, HDR_END);
    if (!hdr_end) { closesocket(cfd); return; }

    size_t hdr_len = hdr_end + 4 - buf;    
    char * body_ptr = buf + hdr_len; // pointer arithmetic: buf points at the start + hdr_len gives us start of the body
    size_t body_have = used - hdr_len;

    size_t body_need = 0;
    {
        char * cl = strstr(buf, "Content-Length:");
        if (cl) body_need = strtoul(cl + 15, NULL, 10);
    }

    while (body_have < body_need && used < BUFSZ) {
        int n = recv(cfd, buf + used, BUFSZ - used, 0);
        if (n <= 0) { closesocket(cfd); return; }
        used += n;
        body_have += n;
    }

    char method[8], path[256];
    if (sscanf(buf, "%7s %255s", method, path) != 2) {
        /* sscanf parses the request line:
           %7s  -> method  (max 7 chars  + NUL)
           %255s-> path    (max 255 chars + NUL)                     */
        closesocket(cfd); return;
    }

    printf("Received request:\n%s\n", buf);

    const char *body, *status;
    if (!strcmp(method, "GET")  && !strcmp(path, "/hello")) {
        body   = "{\"msg\":\"world\"}";
        status = "HTTP/1.1 200 OK\r\n";
        printf("GET /hello\n");
    } else if (!strcmp(method, "POST") && !strcmp(path, "/echo")) {
        body   = buf;
        status = "HTTP/1.1 200 OK\r\n";
        printf("POST /echo\n");
    } else if (!strcmp(method, "POST") && !strcmp(path, "/user-agent")) {
        body   = get_user_agent(buf);
        status = "HTTP/1.1 200 OK\r\n";
        printf("POST /user-agent\n");
    } else {
        body   = "{\"error\":\"not found\"}";
        status = "HTTP/1.1 404 Not Found\r\n";
    }

    char hdr[256];
    int len = snprintf(
        hdr, sizeof hdr,
        "%s"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n",
        status, strlen(body));

    /*  snprintf — safe sprintf
        hdr   : output buffer
        sizeof: capacity (inc. NUL)
        returns bytes written (>= capacity -> truncated)             */
    send(cfd, hdr,  len,             0);           // send headers
    send(cfd, body, strlen(body),    0);           // then body
    closesocket(cfd);                              // done
}
