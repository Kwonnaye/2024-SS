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
#define MAX_CLIENTS 10
#define MSG_SIZE 100
#define BOARD_SIZE 3

int current_turn = 1;  // 전역 변수로 선언
int game_board[BOARD_SIZE][BOARD_SIZE];
pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;

struct client_info {
    int sock;
    int player_id;
};

int game_board[3][3];
sem_t board_sem;
int player_turn = 1;
int connected_clients = 0;
struct client_info client_data[MAX_CLIENTS];

void initialize_board() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            game_board[i][j] = 0;
        }
    }
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
    for (int i = 0; i < connected_clients; i++) {
        if (client_data[i].sock != 0) {
            send(client_data[i].sock, buffer, strlen(buffer), 0);
        }
    }
}

void *client_handler(void *client_info) {
    struct client_info *info = (struct client_info *)client_info;
    int sock = info->sock;
    int player_id = info->player_id;
    char client_msg[MSG_SIZE], response[MSG_SIZE];
    int read_size, row, col, winner;

    while ((read_size = recv(sock, client_msg, MSG_SIZE, 0)) > 0) {
        client_msg[read_size] = '\0';
        sscanf(client_msg, "%d %d", &row, &col);

        sem_wait(&board_sem); // Enter critical section

        if (game_board[row][col] == 0 && player_id == player_turn) {
            game_board[row][col] = player_id;
            player_turn = player_turn % MAX_CLIENTS + 1;
            broadcast_game_state();
        }

        winner = check_winner();
        if (winner > 0) {
            sprintf(response, "Player %d wins", winner);
            broadcast_game_state();
            break;
        }

        sem_post(&board_sem); // Leave critical section
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    close(sock);
    info->sock = 0; // Mark this client as disconnected
    return 0;
}

int check_winner() {
    int player = current_turn;  // 현재 턴의 플레이어 (1 또는 2)

    // 각 행 검사
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (game_board[i][0] == player && game_board[i][1] == player && game_board[i][2] == player) {
            return 1;  // 플레이어가 한 행을 완성했음
        }
    }

    // 각 열 검사
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (game_board[0][i] == player && game_board[1][i] == player && game_board[2][i] == player) {
            return 1;  // 플레이어가 한 열을 완성했음
        }
    }

    // 대각선 검사 (왼쪽 상단에서 오른쪽 하단으로)
    if (game_board[0][0] == player && game_board[1][1] == player && game_board[2][2] == player) {
        return 1;  // 플레이어가 대각선을 완성했음
    }

    // 대각선 검사 (오른쪽 상단에서 왼쪽 하단으로)
    if (game_board[0][2] == player && game_board[1][1] == player && game_board[2][0] == player) {
        return 1;  // 플레이어가 대각선을 완성했음
    }

    return 0;  // 승리 조건을 만족하지 않음
}

int main() {
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;
    pthread_t sniffer_thread;

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

    listen(socket_desc, MAX_CLIENTS);
    initialize_board();
    sem_init(&board_sem, 0, 1);

    c = sizeof(struct sockaddr_in);

    while (1) {
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0) {
            perror("accept failed");
            continue;
        }

        if (connected_clients < MAX_CLIENTS) {
            client_data[connected_clients].sock = client_sock;
            client_data[connected_clients].player_id = connected_clients + 1;
            pthread_create(&sniffer_thread, NULL, client_handler, (void*)&client_data[connected_clients]);
            connected_clients++;
        } else {
            char *message = "Server full\n";
            send(client_sock, message, strlen(message), 0);
            close(client_sock);
        }
    }

    return 0;
}