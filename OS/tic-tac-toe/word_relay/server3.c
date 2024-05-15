#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Global variables
int client_count = 0;
int current_player = 1;
char word[BUFFER_SIZE] = {0};
pthread_mutex_t mutex;
sem_t turn_semaphore;

void *client_handler(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE] = {0};
    int player_id;

    pthread_mutex_lock(&mutex);
    player_id = ++client_count;
    pthread_mutex_unlock(&mutex);

    sprintf(buffer, "You are player %d\n", player_id);
    send(client_socket, buffer, strlen(buffer), 0);

    while (1) {
        if (player_id != current_player) {
            sprintf(buffer, "Waiting for other player's turn...\n");
            send(client_socket, buffer, strlen(buffer), 0);
            sleep(1);
            continue;
        }

        sprintf(buffer, "Your turn. Current word is: %s\n", word);
        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (strlen(buffer) == 0) {
            printf("Player %d disconnected\n", player_id);
            break;
        }

        if (buffer[0] != word[strlen(word) - 1]) {
            sprintf(buffer, "Invalid word. Word should start with %c\n", word[strlen(word) - 1]);
            send(client_socket, buffer, strlen(buffer), 0);
            continue;
        }

        pthread_mutex_lock(&mutex);
        strcpy(word, buffer);
        current_player = (current_player % client_count) + 1;
        pthread_mutex_unlock(&mutex);

        printf("Player %d: %s\n", player_id, word);
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;
    
    // Initialize mutex and semaphore
    pthread_mutex_init(&mutex, NULL);
    sem_init(&turn_semaphore, 0, 1);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind server socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for clients...\n");

    while (1) {
        socklen_t client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Acceptance failed");
            continue;
        }

        pthread_create(&tid, NULL, client_handler, (void *)&client_socket);
        pthread_detach(tid);
    }

    close(server_socket);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&turn_semaphore);

    return 0;
}
