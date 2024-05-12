#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define QUEUE_SIZE 5
#define MAX_CLIENT 2

bool echo_option = false;
bool broadcast_option = false;
int client_num = 0;
int client_sockets[MAX_CLIENT];


void usage() {
    printf("syntax : echo-server <port> [-e[-b]]\n");
    printf("sample : echo-server 1234 -e -b\n");
}


void ErrorHandling(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}


void *handle_client(void *arg) {
    int client_socket_local = *((int *)arg);
    int byte_received;
    char buffer[BUFFER_SIZE];

    while (1) {
        byte_received = recv(client_socket_local, buffer, sizeof(buffer), 0);
        if (byte_received <= 0) {
            perror("recv error");
            printf("Client Disconnected\n");
            break;
        }
        printf("received mesaage: %s\n", buffer);
        
        if (echo_option) {
            printf("echo: %s\n", buffer);
            send(client_socket_local, buffer, strlen(buffer), 0);
        }

        if (broadcast_option) {
            for (int i=0; i<=client_num; i++) {
                send(client_sockets[i], buffer, strlen(buffer), 0);
            }
        }

        printf("\n");
        memset(buffer, 0x00, BUFFER_SIZE); // 버퍼 초기화
    }

    close(client_socket_local);
    client_num --;

    return 0;
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

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        ErrorHandling("failed create socket");
    }

    printf("%s\n", argv[0]);
    printf("%s\n", argv[1]);

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0x00, sizeof(server_addr));
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

    socklen_t client_addr_len;
    pthread_t thread_id;
    int client_socket;
    while (1) {
        if (client_num > MAX_CLIENT) {
            printf("max client\n");
            continue;
        }

        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (client_socket < 0) {
            ErrorHandling("accept error");
        }

        client_sockets[client_num] = client_socket;

        if (pthread_create(&thread_id, NULL, handle_client, &client_socket) != 0) {
            perror("thread create failed");
            close(client_socket);
            continue;
        }
        pthread_detach(thread_id);
        
        client_num++; // 클라이언트 수 증가
        printf("%d\n", client_num);
    }

    close(server_socket); // 서버 소켓 닫기

    printf("server close\n");

    return 0;
}
