#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define BUFSIZE 512

void err_quit(const char* msg)
{
    LPVOID IpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&IpMsgBuf, 0, NULL
    );
    MessageBoxA(NULL, (const char*)IpMsgBuf, msg, MB_ICONERROR);
    LocalFree(IpMsgBuf);
    exit(1);
}

void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char*)&lpMsgBuf, 0, NULL
    );
    printf("[%s] %s\n", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// === 클라이언트 스레드 함수 ===
DWORD WINAPI ClientThread(LPVOID arg)
{
    SOCKET client_sock = (SOCKET)arg;
    char buf[BUFSIZE];
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);

    getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);

    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
    int port = ntohs(clientaddr.sin_port);
    printf("\n클라이언트 접속: IP=%s, PORT=%d\n", addr, port);

    // ===== 파일 크기 수신 =====
    long file_size = 0;
    int ret = recv(client_sock, (char*)&file_size, sizeof(file_size), 0);
    if (ret != sizeof(file_size)) {
        printf("[%s:%d] 파일 크기 정보 수신 실패!\n", addr, port);
        closesocket(client_sock);
        return 1;
    }

    // 파일 이름 지정 (클라이언트마다 다르게)
    char filename[64];
    sprintf(filename, "received_%s_%d.dat", addr, port);
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        printf("[%s:%d] 파일 생성 실패!\n", addr, port);
        closesocket(client_sock);
        return 1;
    }

    // ===== 파일 수신 =====
    long total_bytes = 0;
    while ((ret = recv(client_sock, buf, BUFSIZE, 0)) > 0) {
        fwrite(buf, 1, ret, fp);
        total_bytes += ret;

        float percent = (float)total_bytes / file_size * 100;
        if (percent > 100) percent = 100;
        printf("\r[%s:%d] 수신 중: %.2f%%", addr, port, percent);
        fflush(stdout);
    }

    fclose(fp);
    printf("\n[%s:%d] 수신 완료: 100.00%% (총 %ld 바이트)\n", addr, port, total_bytes);

    closesocket(client_sock);
    printf("[%s:%d] 연결 종료\n", addr, port);
    return 0;
}

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("소켓 오류");
    printf("서버 소켓 생성됨\n");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    if (bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
        err_quit("bind() 오류");

    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR)
        err_quit("listen() 오류");

    printf("클라이언트 접속 대기 중...\n");

    while (1)
    {
        struct sockaddr_in clientaddr;
        int addrlen = sizeof(clientaddr);
        SOCKET client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            continue;
        }

        // 스레드 생성
        HANDLE hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) {
            err_display("CreateThread()");
            closesocket(client_sock);
        }
        else {
            CloseHandle(hThread); // 스레드 핸들은 닫아도 스레드는 백그라운드에서 실행됨
        }
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}