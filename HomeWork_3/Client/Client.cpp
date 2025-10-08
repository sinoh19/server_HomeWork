#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "ws2_32")

#define BUFSIZE 512

void err_quit(const char* msg) {
    printf("%s\n", msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc < 2 || argc > 3) {
        printf("사용법: %s [서버IP] 파일명\n", argv[0]);
        return 1;
    }

    const char* server_ip = "127.0.0.1";  // 기본값 로컬호스트
    char* filename;

    if (argc == 2) {
        filename = argv[1];  // 서버 IP 입력 안 하면 기본값 사용
    }
    else {
        server_ip = argv[1]; // 서버 IP 입력
        filename = argv[2];
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("소켓 생성 실패");

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
        err_quit("서버 연결 실패");

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("파일 열 수 없음: %s\n", filename);
        closesocket(sock);
        return 1;
    }

    // 파일 크기 전송
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    send(sock, (char*)&file_size, sizeof(file_size), 0);

    // 파일 전송
    char buf[BUFSIZE];
    size_t n;
    while ((n = fread(buf, 1, BUFSIZE, fp)) > 0) {
        send(sock, buf, n, 0);
    }

    fclose(fp);
    closesocket(sock);
    WSACleanup();

    printf("파일 전송 완료!\n");
    return 0;
}