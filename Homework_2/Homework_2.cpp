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
        printf("����: %s <������ �̸�>\n", argv[0]);
        return 1;
    }

    // ���� �ʱ�ȭ
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup ����\n");
        return 1;
    }

    // ������ �̸����� ȣ��Ʈ ���� ���
    host = gethostbyname(argv[1]);
    if (host == NULL) {
        printf("ȣ��Ʈ ������ ������ �� �����ϴ�.\n");
        WSACleanup();
        return 1;
    }

    // ���� �̸� ���
    printf("���� ȣ��Ʈ �̸�: %s\n", host->h_name);

    // ���� ���
    printf("\n[���� ���]\n");
    for (alias = host->h_aliases; *alias != NULL; alias++) {
        printf("  %s\n", *alias);
    }

    // IPv4 �ּ� ���
    printf("\n[IPv4 �ּ� ���]\n");
    for (i = 0; host->h_addr_list[i] != NULL; i++) {
        addr.s_addr = *(u_long*)host->h_addr_list[i];
        printf("  %s\n", inet_ntoa(addr));
    }

    WSACleanup();
    return 0;
}