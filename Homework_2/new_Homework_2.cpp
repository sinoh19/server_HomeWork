#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    struct addrinfo hints, * res, * p;
    char ipstr[INET_ADDRSTRLEN]; // IPv4 주소 저장

    if (argc != 2) {
        printf("사용법: %s <도메인 이름>\n", argv[0]);
        return 1;
    }

    // 윈속 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup 실패\n");
        return 1;
    }

    // hints 구조 초기화
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4만
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // 도메인 이름으로 호스트 정보 얻기
    if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) {
        printf("호스트 정보를 가져올 수 없습니다.\n");
        WSACleanup();
        return 1;
    }

    printf("입력한 도메인: %s\n", argv[1]);
    printf("\n[IPv4 주소 목록]\n");

    for (p = res; p != NULL; p = p->ai_next) {
        struct sockaddr_in* addr = (struct sockaddr_in*)p->ai_addr;
        inet_ntop(AF_INET, &(addr->sin_addr), ipstr, sizeof(ipstr));
        printf("  %s\n", ipstr);
    }

    freeaddrinfo(res);
    WSACleanup();
    return 0;
}
