#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define MSG_SIZE 100

struct message {
    long mtype;
    char mtext[MSG_SIZE];
};

int msgid;
int game_board[3][3];
sem_t board_sem;

void initialize_board() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            game_board[i][j] = 0;
        }
    }
}

void *client_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char client_msg[MSG_SIZE];
    struct message msg;
    int read_size;

    while ((read_size = recv(sock, client_msg, MSG_SIZE, 0)) > 0) {
        client_msg[read_size] = '\0';

        sem_wait(&board_sem); // Critical section 시작

        // 게임 로직 처리 (예: 사용자 입력을 보드에 적용)
        // 보드 업데이트, 승리 조건 체크 등

        sem_post(&board_sem); // Critical section 종료

        msg.mtype = 1;
        strcpy(msg.mtext, client_msg);
        msgsnd(msgid, &msg, sizeof(msg), 0);
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

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
    msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);

    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))) {
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

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

