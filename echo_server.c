#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define QUEUE_SIZE 5

bool echo_option = false;
bool broadcast_option = false;


void usage() {
    printf("syntax : echo-server <port> [-e[-b]]\n");
    printf("sample : echo-server 1234 -e -b\n");
}


void ErrorHandling(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}


void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while (recv(client_socket, buffer, sizeof(buffer), 0) > 0) {
        printf("received mesaage: %s\n", buffer);
        if (echo_option) {
            send(client_socket, buffer, strlen(buffer), 0);
        }
    }
    close(client_socket);

    return NULL;
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        usage();
        return -1;
    }
    else {
        for (int i=0; i<argc; i++) {
            if (strcmp(argv[i], "-e") == 0) {
                echo_option = true;
            }
            else if (strcmp(argv[i], "-b") == 0) {
                broadcast_option = true;
            }
        }
    }
    // printf("%d\n", argc);
    // printf("%d\n", echo_option);
    // printf("%d\n", broadcast_option);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        ErrorHandling("failed create socket");
    }

    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ErrorHandling("bind error");
    }

    printf("server start, port num: %i\n", ntohs(server_addr.sin_port));

    if (listen(server_socket, QUEUE_SIZE) < 0) {
        ErrorHandling("listen error");
    }

    int client_socket;
    socklen_t client_addr_len;
    pthread_t thread_id;
    while (1) {
        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            ErrorHandling("accept error");
        }

        printf("Client connected: %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

        if (pthread_create(&thread_id, NULL, handle_client, &client_socket) != 0) {
            perror("thread create failed");
            close(client_socket);
            continue;
        }
    }

    close(server_socket); // 서버 소켓 닫기

    printf("server close\n");

    return 0;
}
