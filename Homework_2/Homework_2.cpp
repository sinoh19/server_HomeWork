#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    struct hostent* host;
    char** alias;
    struct in_addr addr;
    int i;

    if (argc != 2) {
        printf("사용법: %s <도메인 이름>\n", argv[0]);
        return 1;
    }

    // 윈속 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup 실패\n");
        return 1;
    }

    // 도메인 이름으로 호스트 정보 얻기
    host = gethostbyname(argv[1]);
    if (host == NULL) {
        printf("호스트 정보를 가져올 수 없습니다.\n");
        WSACleanup();
        return 1;
    }

    // 공식 이름 출력
    printf("공식 호스트 이름: %s\n", host->h_name);

    // 별명 출력
    printf("\n[별명 목록]\n");
    for (alias = host->h_aliases; *alias != NULL; alias++) {
        printf("  %s\n", *alias);
    }

    // IPv4 주소 출력
    printf("\n[IPv4 주소 목록]\n");
    for (i = 0; host->h_addr_list[i] != NULL; i++) {
        addr.s_addr = *(u_long*)host->h_addr_list[i];
        printf("  %s\n", inet_ntoa(addr));
    }

    WSACleanup();
    return 0;
}