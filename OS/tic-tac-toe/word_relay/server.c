#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <semaphore.h>

#define PORT 8888
#define MAX_CLIENTS 10
#define MSG_SIZE 100

typedef struct {
    char content[MSG_SIZE];
    int player;
    int size;
} Message;

typedef struct {
    int sock;
    int player_id;
} ClientInfo;

int client_count = 0;
pthread_mutex_t lock;
sem_t wr_sem;
ClientInfo client_data[MAX_CLIENTS];
Message current_message;

void initialize_game() {
    client_count = 0;
    current_message.content[0] = '\0';
    current_message.player = 1;
    current_message.size = 0;
    return;
}

void broadcast(char *client_msg) {
    char msg[MSG_SIZE];
    sprintf(msg, "%s", client_msg);

    for (int i = 1; i <= client_count; i++) {
        if (send(client_data[i].sock, msg, strlen(msg), 0) == -1) {
            perror("Send Failed");
            continue;
        }
    }
    printf("Server response: %s\n", msg);
    return;
}

bool check_word(char *client_msg, int size) {
    if (client_msg[0] == current_message.content[current_message.size-1]) {
        return true;
    } else {
        return false;
    }
    
}

void next_player() {
    if(++current_message.player > client_count) {
        current_message.player = 1;
    }
}

void *client_handler(void *client_info) {
    ClientInfo *info = (ClientInfo *)client_info;
    int sock = info->sock;
    int plyaer_id = info->player_id;
    char client_msg[MSG_SIZE], response[MSG_SIZE];
    int read_size = 0;

    while ((read_size = recv(sock, client_msg, MSG_SIZE, 0)) > 0) {
        client_msg[read_size] = '\0';
        
        printf("current player: %d\n", current_message.player);
        printf("recv msg: %s\n", client_msg);

        if (plyaer_id != current_message.player) {
            // 너 아니라고 알려주기
            char *not_ur_turn = "Not Your Turn!";
            if (send(info->sock, not_ur_turn, strlen(not_ur_turn), 0) == -1) {
                perror("send failed!");
            }
            continue;
        }

        sem_wait(&wr_sem); // LOCK

        if (current_message.content == "") {
            sprintf(current_message.content, "%s", client_msg);
            current_message.size = read_size;
        } else {
            if (check_word(client_msg, read_size)) {
                // 정답
                broadcast(client_msg);
            } else {
                // 오답
                broadcast(client_msg);
            }
        }
        next_player();

        sem_post(&wr_sem); // RELEASE
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("Recv Failed");
    }

    close(sock);
    info->sock = 0;

    return 0;
}

int main() {
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;
    Message message;
    pthread_t sniffer_thread;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Failed to create socket\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind Failed\n");
        return 1;
    }

    listen(socket_desc, MAX_CLIENTS);
    initialize_game();
    sem_init(&wr_sem, 0, 1);

    c = sizeof(struct sockaddr_in);

    while(1) {
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0) {
            perror("Accept Failed\n");
            continue;
        }

        if (client_count < MAX_CLIENTS) {
            // 연결
            client_count++;
            client_data[client_count].sock = client_sock;
            client_data[client_count].player_id = client_count;
            printf("%d\n", client_count);
            pthread_create(&sniffer_thread, NULL, client_handler, (void*)&client_data[client_count]);
        } else {
            char *msg = "Server Full\n";
            send(client_sock, msg, strlen(msg), 0);
            close(client_sock);
        }
    }

    return 0;
}