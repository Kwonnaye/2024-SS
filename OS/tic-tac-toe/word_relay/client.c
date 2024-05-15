#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8888
#define MSG_SIZE 100

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char msg[MSG_SIZE] = "";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket create error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("invalid address");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        return -1;
    }

    printf("Connected to server\n");

    // Game loop
    while (1) {
        printf("Enter a word: ");
        fgets(msg, MSG_SIZE, stdin);

        // Send word to server
        if (send(sock, msg, strlen(msg), 0) == -1) {
            perror("send failed");
            break;
        }

        // Receive response from server
        if (recv(sock, msg, MSG_SIZE, 0) == -1) {
            perror("recv failed");
            break;
        }

        printf("Server response: %s\n", msg);
    }

    close(sock);
    return 0;
}