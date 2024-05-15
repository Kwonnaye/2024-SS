#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 2
#define MSG_SIZE 100

struct message {
    long mtype;
    char mtext[MSG_SIZE];
};

int clients[MAX_CLIENTS] = {0};
int game_board[3][3];
sem_t board_sem;
int player_turn = 1;

void initialize_board() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            game_board[i][j] = 0;
        }
    }
}

int check_winner() {
    for (int i = 0; i < 3; i++) {
        if ((game_board[i][0] == game_board[i][1] && game_board[i][1] == game_board[i][2] && game_board[i][0] != 0) ||
            (game_board[0][i] == game_board[1][i] && game_board[1][i] == game_board[2][i] && game_board[0][i] != 0))
            return game_board[i][i];
    }
    if ((game_board[0][0] == game_board[1][1] && game_board[1][1] == game_board[2][2] && game_board[0][0] != 0) ||
        (game_board[0][2] == game_board[1][1] && game_board[1][1] == game_board[2][0] && game_board[0][2] != 0))
        return game_board[1][1];
    return 0;
}

void broadcast_game_state() {
    char buffer[MSG_SIZE];
    sprintf(buffer, "Board Update\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            char temp[4];
            sprintf(temp, "%d ", game_board[i][j]);
            strcat(buffer, temp);
        }
        strcat(buffer, "\n");
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != 0) {
            send(clients[i], buffer, strlen(buffer), 0);
        }
    }
}

void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    int read_size, row, col, winner, player_id;
    char client_msg[MSG_SIZE], response[MSG_SIZE];

    player_id = player_turn;

    while ((read_size = recv(sock, client_msg, MSG_SIZE, 0)) > 0) {
        client_msg[read_size] = '\0';
        sscanf(client_msg, "%d %d", &row, &col);

        sem_wait(&board_sem); // Enter critical section

        if (game_board[row][col] == 0 && player_id == player_turn) {
            game_board[row][col] = player_id;
            player_turn = player_turn % MAX_CLIENTS + 1; // Toggle between player 1 and 2
            broadcast_game_state();
        }

        winner = check_winner();
        if (winner > 0) {
            sprintf(response, "Player %d wins", winner);
            broadcast_game_state();
            break; // End game if there's a winner
        }

        sem_post(&board_sem); // Leave critical section
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    clients[player_id - 1] = 0; // Mark client as disconnected
    free(socket_desc);
    close(sock);
    return 0;
}

int main() {
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed");
        return 1;
    }

    listen(socket_desc, 3);
    initialize_board();
    sem_init(&board_sem, 0, 1);

    c = sizeof(struct sockaddr_in);
    int client_count = 0;
    while (client_count < MAX_CLIENTS && (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))) {
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        clients[client_count++] = client_sock;

        if (pthread_create(&sniffer_thread, NULL, client_handler, (void*) new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }
    }

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}
