#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MSG_SIZE 1024

void display_board(char *board) {
    int index = 0;
    printf("Current Board State:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            char symbol = '.';
            if (board[index] == '1') symbol = 'X';
            else if (board[index] == '2') symbol = 'O';
            printf(" %c ", symbol);
            index += 2;
            if (j < 2) printf("|");
        }
        printf("\n");
        if (i < 2) printf("---+---+---\n");
    }
}

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

    // Initial board state received
    if (recv(sock, server_reply, MSG_SIZE, 0) < 0) {
        puts("recv failed");
        return 1;
    }

    display_board(server_reply);

    while (1) {
        printf("Enter your move (row col, e.g., 1 2): ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline character

        if (send(sock, message, strlen(message), 0) < 0) {
            puts("Send failed");
            return 1;
        }

        memset(server_reply, 0, sizeof(server_reply));
        if (recv(sock, server_reply, MSG_SIZE, 0) < 0) {
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