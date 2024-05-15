#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MSG_SIZE 100

int main() {
    int sock;
    struct sockaddr_in server;
    char message[MSG_SIZE], server_reply[MSG_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed");
        return 1;
    }

    printf("Connected to the server\n");

    while (1) {
        printf("Enter row and column (e.g., 1 2): ");
        fgets(message, MSG_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline character

        if (send(sock, message, strlen(message), 0) < 0) {
            puts("Send failed");
            return 1;
        }

        // Clear the buffer
        memset(server_reply, 0, MSG_SIZE);
        if (recv(sock, server_reply, MSG_SIZE, 0) < 0) {
            puts("recv failed");
            break;
        }

        puts("Server reply :");
        puts(server_reply);
    }

    close(sock);
    return 0;
}
