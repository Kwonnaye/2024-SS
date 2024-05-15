#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int sock;
    struct sockaddr address;
    int addr_len;
} connection_t;

int game_started = 0;
int num_clients = 0;
char last_word[BUFFER_SIZE] = {0};

pthread_mutex_t lock;
sem_t turn_semaphore;
int msgid;

// Message queue structure
struct mesg_buffer {
    long mesg_type;
    char mesg_text[BUFFER_SIZE];
} message;

void *client_thread(void *ptr) {
    connection_t *conn;
    char buffer[BUFFER_SIZE];
    int len;

    if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;

    // Game loop
    while ((len = read(conn->sock, buffer, sizeof(buffer) - 1)) > 0) {
        pthread_mutex_lock(&lock);
        buffer[len] = 0;

        // Check word validity
        if (game_started && buffer[0] == last_word[strlen(last_word) - 1]) {
            strcpy(last_word, buffer);
            printf("Valid word: %s\n", buffer);

            // Send to all clients via message queue
            message.mesg_type = 1;
            strcpy(message.mesg_text, buffer);
            msgsnd(msgid, &message, sizeof(message), 0);

            // Signal next client's turn
            sem_post(&turn_semaphore);
        } else {
            printf("Invalid word or not your turn: %s\n", buffer);
            strcpy(buffer, "Invalid word or not your turn");
            write(conn->sock, buffer, strlen(buffer));
        }
        pthread_mutex_unlock(&lock);
    }

    // Cleanup
    close(conn->sock);
    free(conn);
    pthread_exit(0);
}

int main(int argc, char **argv) {
    int sock = -1, client_sock = -1;
    struct sockaddr_in address;
    connection_t *connection;
    pthread_t thread;

    // Create message queue
    key_t key = ftok("progfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);

    // Create semaphore
    sem_init(&turn_semaphore, 0, 1);

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock <= 0) {
        perror("socket failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0) {
        perror("bind failed");
        return -1;
    }

    // Listen
    if (listen(sock, MAX_CLIENTS) < 0) {
        perror("listen failed");
        return -1;
    }

    printf("Server is listening on port %d\n", PORT);

    // Accept clients
    while (1) {
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(sock, &connection->address, &connection->addr_len);
        if (connection->sock <= 0) {
            free(connection);
        } else {
            pthread_create(&thread, 0, client_thread, (void *)connection);
            pthread_detach(thread);
        }
    }

    return 0;
}
