#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MSG_SIZE 100

void display_board(char *board) {
    int offset = 0;
    printf("Current Board State:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            switch (board[offset]) {
                case '1': printf(" X "); break;
                case '2': printf(" O "); break;
                default: printf(" . "); break;
            }
            if (j < 2) printf("|");
            offset += 2;
        }
        printf("\n");
        if (i < 2) printf("-----------\n");
    }
}

int main() {
    int sock;
    struct sockaddr_in server;
    char message[MSG_SIZE], server_reply[MSG_SIZE * 5];

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

    // Receive initial game state
    if (recv(sock, server_reply, MSG_SIZE * 5, 0) < 0) {
        puts("recv failed");
        return 1;
    }

    display_board(server_reply);

    while (1) {
        printf("Enter your move (row col, e.g., 1 2): ");
        fgets(message, MSG_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline character

        if (send(sock, message, strlen(message), 0) < 0) {
            puts("Send failed");
            return 1;
        }

        // Clear the buffer
        memset(server_reply, 0, sizeof(server_reply));
        if (recv(sock, server_reply, MSG_SIZE * 5, 0) < 0) {
            puts("recv failed");
            break;
        }

        if (strncmp(server_reply, "Player", 6) == 0) {
            printf("%s\n", server_reply);
            break; // Game over
        }

        display_board(server_reply);
    }

    close(sock);
    return 0;
}
