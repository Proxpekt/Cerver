#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096

// Send all bytes in buf (handles partial sends)
static int send_all(SOCKET s, const char *buf, int len) {
    int total = 0;
    while (total < len) {
        int sent = send(s, buf + total, len - total, 0);
        if (sent == SOCKET_ERROR) return SOCKET_ERROR;
        if (sent == 0) break;
        total += sent;
    }
    return total;
}

// Read & discard the HTTP request (basic drain)
static void drain_http_request(SOCKET client) {
    char tmp[BUFFER_SIZE];
    // Read until we see \r\n\r\n or the peer closes
    // Keep it simple for a toy server
    int got = 0;
    int header_end = 0;
    char window[4] = {0,0,0,0};
    do {
        got = recv(client, tmp, sizeof(tmp), 0);
        if (got <= 0) break;
        for (int i = 0; i < got; ++i) {
            window[0] = window[1];
            window[1] = window[2];
            window[2] = window[3];
            window[3] = tmp[i];
            if (window[0] == '\r' && window[1] == '\n' &&
                window[2] == '\r' && window[3] == '\n') {
                header_end = 1;
                break;
            }
        }
    } while (!header_end);
}

static void send_404(SOCKET client, const char *msg) {
    char body[256];
    if (!msg) msg = "404 Not Found";
    int body_len = snprintf(body, sizeof(body),
                            "<html><body><h1>%s</h1></body></html>", msg);

    char hdr[256];
    int hdr_len = snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        body_len);
    send_all(client, hdr, hdr_len);
    send_all(client, body, body_len);
}

static void send_file(SOCKET client, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("[!] html open error: %s\n", path);
        send_404(client, "404 Not Found");
        return;
    }

    // compute file size
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        send_404(client, "File read error");
        return;
    }
    long filesize = ftell(f);
    if (filesize < 0) {
        fclose(f);
        send_404(client, "File read error");
        return;
    }
    fseek(f, 0, SEEK_SET);

    // send HTTP header with Content-Length and Connection: close
    char header[256];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n", filesize);

    if (send_all(client, header, header_len) == SOCKET_ERROR) {
        fclose(f);
        return;
    }

    // send file contents
    char buf[BUFFER_SIZE];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, f)) > 0) {
        if (send_all(client, buf, (int)n) == SOCKET_ERROR) {
            break;
        }
    }
    fclose(f);
}

int main(void) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // SO_REUSEADDR expects char* on Windows
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof addr) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, 8) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Listening on http://localhost:%d\n", PORT);

    for (;;) {
        struct sockaddr_in cli;
        int len = sizeof cli;
        SOCKET client = accept(server_fd, (struct sockaddr*)&cli, &len);
        if (client == INVALID_SOCKET) {
            printf("Accept failed: %d\n", WSAGetLastError());
            continue;
        }

        printf("Client connected\n");

        // Read & discard the request (we don’t route yet)
        drain_http_request(client);

        // Serve index.html (must be next to the exe)
        send_file(client, "index.html");

        printf("Disconnected\n");
        closesocket(client);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
