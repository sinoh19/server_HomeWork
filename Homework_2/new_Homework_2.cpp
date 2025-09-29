#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    struct addrinfo hints, * res, * p;
    char ipstr[INET_ADDRSTRLEN]; // IPv4 �ּ� ����

    if (argc != 2) {
        printf("����: %s <������ �̸�>\n", argv[0]);
        return 1;
    }

    // ���� �ʱ�ȭ
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup ����\n");
        return 1;
    }

    // hints ���� �ʱ�ȭ
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4��
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // ������ �̸����� ȣ��Ʈ ���� ���
    if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) {
        printf("ȣ��Ʈ ������ ������ �� �����ϴ�.\n");
        WSACleanup();
        return 1;
    }

    printf("�Է��� ������: %s\n", argv[1]);
    printf("\n[IPv4 �ּ� ���]\n");

    for (p = res; p != NULL; p = p->ai_next) {
        struct sockaddr_in* addr = (struct sockaddr_in*)p->ai_addr;
        inet_ntop(AF_INET, &(addr->sin_addr), ipstr, sizeof(ipstr));
        printf("  %s\n", ipstr);
    }

    freeaddrinfo(res);
    WSACleanup();
    return 0;
}
