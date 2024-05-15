#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    int sockfd;
    struct sockaddr address;
    int addr_len;
    int id;
    char last_char;
} client_t;

client_t *clients[2] = {NULL}; // 최대 2명의 플레이어
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    client_t *cli = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int len;

    while ((len = recv(cli->sockfd, buffer, sizeof(buffer), 0)) > 0) {
        buffer[len] = '\0'; // Null-terminate the buffer
        printf("Received from client %d: %s\n", cli->id, buffer);

        // Check if the word is valid
        if (cli->last_char != 0 && buffer[0] != cli->last_char) {
            char *msg = "Invalid word! It should start with the last character of the previous word.\n";
            send(cli->sockfd, msg, strlen(msg), 0);
            continue;
        }

        // Update the last character
        cli->last_char = buffer[strlen(buffer) - 1];

        // Broadcast the word to the other client
        for (int i = 0; i < 2; i++) {
            if (clients[i] && clients[i]->id != cli->id) {
                send(clients[i]->sockfd, buffer, len, 0);
            }
        }
    }

    close(cli->sockfd);
    pthread_mutex_lock(&clients_mutex);
    clients[cli->id] = NULL;
    client_count--;
    pthread_mutex_unlock(&clients_mutex);
    free(cli);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    pthread_t tid;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        if (client_count == 2) {
            printf("Max clients connected. Connection refused.\n");
            close(new_socket);
            continue;
        }

        client_t *cli = (client_t *)malloc(sizeof(client_t));
        if (!cli) {
            printf("Failed to allocate memory for new client.\n");
            close(new_socket);
            continue;
        }

        cli->sockfd = new_socket;
        cli->address = client_addr;
        cli->addr_len = addr_size;
        cli->id = client_count;
        cli->last_char = 0;

        pthread_mutex_lock(&clients_mutex);
        clients[client_count++] = cli;
        pthread_mutex_unlock(&clients_mutex);

        pthread_create(&tid, NULL, &handle_client, (void *)cli);
        printf("Client %d connected\n", cli->id);
    }

    close(server_fd);
    return 0;
}