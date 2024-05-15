#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <semaphore.h>

#define PORT 8080
#define MAX_CLIENTS 2

struct message_buffer {
	    long msg_type;
	    char msg_text[100];
};

int game_board[3][3];
int client_count = 0;
pthread_mutex_t lock;
sem_t board_sem;
int msgid;

void initialize_game_board() {
	    memset(game_board, 0, sizeof(game_board));
}

void *client_handler(void *socket_desc) {
	int sock = *(int*)socket_desc;
	int read_size;
	char client_message[2000];
	struct message_buffer message;

	while((read_size = recv(sock, client_message, 2000, 0)) > 0) {
		client_message[read_size] = '\0';
		printf("Received: %s\n", client_message);

		pthread_mutex_lock(&lock);
		pthread_mutex_unlock(&lock);

		sem_wait(&board_sem);
		sprintf(message.msg_text, "Update: %s", client_message);
		message.msg_type = 1;
		msgsnd(msgid, &message, sizeof(message), 0);
		sem_post(&board_sem);

		memset(client_message, 0, sizeof(client_message));
	}

	if (read_size == 0) {
		puts("Client disconnected");
	} else if (read_size == -1) {
		perror("recv failed");
	}

	free(socket_desc);
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
		perror("bind failed. Error");
		return 1;
	}

	listen(socket_desc, 3);
	initialize_game_board();
	pthread_mutex_init(&lock, NULL);
	sem_init(&board_sem, 0, 1);
	msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);

	printf("Waiting for connections...\n");
	c = sizeof(struct sockaddr_in);
	while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))) {
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = client_sock;

		if (pthread_create(&sniffer_thread, NULL, client_handler, (void*) new_sock) < 0) {
			perror("could not create thread");
			return 1;
		    }

		pthread_mutex_lock(&lock);
		client_count++;
		pthread_mutex_unlock(&lock);
	}

	if (client_sock < 0) {
		perror("accept failed");
		return 1;
	}

	return 0;
}
