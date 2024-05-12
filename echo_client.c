#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFFER_SIZE 1024


void usage() {
    printf("syntax : echo-client <ip> <port>\n");
    printf("sample : echo-client 192.168.10.2 1234\n");
}


void ErrorHandling(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}


void *receive_broadcast(void *arg) {
    int sock = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0x00, BUFFER_SIZE); // 버퍼 초기화

        // 서버로부터 응답을 받음
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            ErrorHandling("recv error");
        }
        printf("\nBroadcast from server: %s\n", buffer);
        fflush(stdout);
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        usage();
        return -1;
    }

    // 소켓 파일 디스크립터 생성
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ErrorHandling("failed create socket");
    }

    // 서버 정보 설정
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    // IPv4 주소를 네트워크 바이트 순서로 변환
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) {
        ErrorHandling("Invalid address/ Address not supported");
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        ErrorHandling("Connection Failed");
    }

    // 브로드캐스트를 받는 쓰레드 생성
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, receive_broadcast, (void *)&sock) != 0) {
        ErrorHandling("Thread create failed");
    }

    // 서버로부터 메시지를 수신하고 다시 전송
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0x00, BUFFER_SIZE); // 버퍼 초기화
        printf("Enter message: ");
        fflush(stdout);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        send(sock, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit") == 0) {
            printf("disconnect\n");
            break;
        } 

        // 서버로부터 응답을 받음
        if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0) {
            ErrorHandling("recv error");
        }
        printf("Server: %s\n", buffer);
    }

    close(sock); // 소켓 닫기

    return 0;
}