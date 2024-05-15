#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // Receive player id
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    printf("%s", buffer);

    while (1) {
        // Receive turn message
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        printf("%s", buffer);

        // If it's not the client's turn, skip input
        if (strstr(buffer, "Your turn") != NULL) {
            // Get user input
            printf("Enter a word: ");
            fgets(buffer, BUFFER_SIZE, stdin);
            send(client_socket, buffer, strlen(buffer), 0);

            // Check for game over
            if (strlen(buffer) == 0) {
                printf("Disconnected from server\n");
                break;
            }

            // Receive server response
            memset(buffer, 0, BUFFER_SIZE);
            recv(client_socket, buffer, BUFFER_SIZE, 0);
            printf("%s", buffer);
        }
    }

    close(client_socket);
    return 0;
}
