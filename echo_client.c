#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024


void usage() {
    printf("syntax : echo-client <ip> <port>\n");
    printf("sample : echo-client 192.168.10.2 1234\n");
}


void ErrorHandling(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
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

    // 서버로부터 메시지를 수신하고 다시 전송
    char buffer[BUFFER_SIZE];
    while (1) {
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);
        printf("Message sent\n");

        // 서버로부터 응답을 받음
        if (recv(sock, buffer, BUFFER_SIZE, 0) == 0) {
            printf("Server disconnected\n");
            break;
        }
        printf("Server: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE); // 버퍼 초기화
    }

    close(sock); // 소켓 닫기
    return 0;
}