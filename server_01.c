#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET server_fd, client_fd;
    struct sockaddr_in server, client;
    int c, recv_size;
    char buffer[2048];

    // Initialize Winsock
    WSAStartup(MAKEWORD(2,2), &wsa);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Prepare sockaddr_in
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    // Bind
    bind(server_fd, (struct sockaddr *)&server, sizeof(server));

    // Listen
    listen(server_fd, 3);

    printf("Listening on http://localhost:8080\n");

    while (1) {
        c = sizeof(struct sockaddr_in);
        client_fd = accept(server_fd, (struct sockaddr *)&client, &c);

        // Receive request
        recv_size = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        buffer[recv_size] = '\0';
        printf("Client says:\n%s\n", buffer);

        // Send HTTP response
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>Hello from C Server on Windows! 🎉</h1></body></html>";

        send(client_fd, response, (int)strlen(response), 0);

        closesocket(client_fd);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
