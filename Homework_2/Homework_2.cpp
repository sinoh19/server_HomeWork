#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    struct hostent* hostInfo;
    char** aliasName;
    struct in_addr ipAddr;
    int ipIndex;

    if (argc != 2) {
        printf("����: %s <������ �̸�>\n", argv[0]);
        return 1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    hostInfo = gethostbyname(argv[1]);
    if (!hostInfo) { WSACleanup(); return 1; }

    printf("���� ȣ��Ʈ �̸�: %s\n\n[���� ���]\n", hostInfo->h_name);
    for (aliasName = hostInfo->h_aliases; *aliasName; aliasName++) printf("  %s\n", *aliasName);

    printf("\n[IPv4 �ּ� ���]\n");
    for (ipIndex = 0; hostInfo->h_addr_list[ipIndex]; ipIndex++) {
        ipAddr.s_addr = *(u_long*)hostInfo->h_addr_list[ipIndex];
        printf("  %s\n", inet_ntoa(ipAddr));
    }

    WSACleanup();
    return 0;
}