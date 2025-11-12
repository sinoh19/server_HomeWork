// client_gui.c
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>   // 반드시 windows.h보다 먼저
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <stdarg.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

#define ID_BTN_BROWSE 1001
#define ID_BTN_SEND   1002
#define ID_PROGRESS   1003
#define ID_STATICFILE 1004
#define ID_LISTBOX    1005

#define SERVER_PORT 9000
#define SERVER_IP "192.168.0.9"
#define BUFSIZE 512

HWND hProgress, hFileStatic, hListBox;
char g_selectedFile[MAX_PATH] = { 0 };
volatile BOOL g_sending = FALSE;

// 리스트박스에 로그 추가
void LogAdd(HWND hwnd, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[512];
    vsprintf(buf, fmt, ap);
    va_end(ap);
    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)buf);
    SendMessage(hListBox, LB_SETCURSEL, (WPARAM)(SendMessage(hListBox, LB_GETCOUNT, 0, 0) - 1), 0);
}

// 파일 전송 스레드
DWORD WINAPI SendThread(LPVOID arg) {
    if (g_sending) return 0;
    g_sending = TRUE;

    if (g_selectedFile[0] == 0) {
        LogAdd(NULL, "전송할 파일을 먼저 선택하세요.");
        g_sending = FALSE;
        return 0;
    }

    FILE* fp = fopen(g_selectedFile, "rb");
    if (!fp) {
        LogAdd(NULL, "파일 열기 실패: %s", g_selectedFile);
        g_sending = FALSE;
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        LogAdd(NULL, "WSAStartup 실패");
        fclose(fp);
        g_sending = FALSE;
        return 0;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        LogAdd(NULL, "소켓 생성 실패");
        WSACleanup();
        fclose(fp);
        g_sending = FALSE;
        return 0;
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    LogAdd(NULL, "서버(%s:%d) 연결 시도...", SERVER_IP, SERVER_PORT);
    if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        LogAdd(NULL, "서버 연결 실패: %d", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        fclose(fp);
        g_sending = FALSE;
        return 0;
    }
    LogAdd(NULL, "서버 연결됨.");

    send(sock, (char*)&file_size, sizeof(file_size), 0);

    char buf[BUFSIZE];
    long total = 0;
    SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
    SendMessage(hProgress, PBM_SETPOS, 0, 0);

    size_t n;
    while ((n = fread(buf, 1, BUFSIZE, fp)) > 0) {
        int sent = 0;
        while (sent < (int)n) {
            int ret = send(sock, buf + sent, (int)n - sent, 0);
            if (ret == SOCKET_ERROR) {
                LogAdd(NULL, "전송 오류: %d", WSAGetLastError());
                closesocket(sock);
                WSACleanup();
                fclose(fp);
                g_sending = FALSE;
                return 0;
            }
            sent += ret;
            total += ret;
            int pos = (int)((double)total / (double)file_size * 1000.0);
            SendMessage(hProgress, PBM_SETPOS, pos, 0);
        }
    }

    LogAdd(NULL, "전송 완료: 총 %ld 바이트", total);
    SendMessage(hProgress, PBM_SETPOS, 1000, 0);
    closesocket(sock);
    WSACleanup();
    fclose(fp);
    g_sending = FALSE;
    return 0;
}

// 윈도우 메시지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowEx(0, "STATIC", "선택 파일:", WS_CHILD | WS_VISIBLE, 10, 10, 80, 20, hWnd, NULL, NULL, NULL);
        hFileStatic = CreateWindowEx(0, "STATIC", "(없음)", WS_CHILD | WS_VISIBLE | SS_LEFT, 100, 10, 360, 20, hWnd, (HMENU)ID_STATICFILE, NULL, NULL);
        CreateWindowEx(0, "BUTTON", "파일 선택", WS_CHILD | WS_VISIBLE, 470, 8, 100, 24, hWnd, (HMENU)ID_BTN_BROWSE, NULL, NULL);
        CreateWindowEx(0, "BUTTON", "전송", WS_CHILD | WS_VISIBLE, 470, 40, 100, 28, hWnd, (HMENU)ID_BTN_SEND, NULL, NULL);
        hProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE, 10, 40, 440, 28, hWnd, (HMENU)ID_PROGRESS, NULL, NULL);
        hListBox = CreateWindowEx(0, WC_LISTBOX, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL, 10, 80, 560, 300, hWnd, (HMENU)ID_LISTBOX, NULL, NULL);
        LogAdd(NULL, "클라이언트 준비 완료 (서버: %s:%d)", SERVER_IP, SERVER_PORT);
        break;
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_BTN_BROWSE) {
            OPENFILENAME ofn;
            char szFile[MAX_PATH] = { 0 };
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "모든 파일\0*.*\0\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            if (GetOpenFileName(&ofn)) {
                strcpy(g_selectedFile, szFile);
                SetWindowText(hFileStatic, g_selectedFile);
                LogAdd(NULL, "파일 선택: %s", g_selectedFile);
            }
        }
        else if (id == ID_BTN_SEND) {
            if (g_selectedFile[0] == 0) {
                LogAdd(NULL, "전송할 파일을 선택하세요.");
            }
            else if (!g_sending) {
                HANDLE h = CreateThread(NULL, 0, SendThread, NULL, 0, NULL);
                if (h) CloseHandle(h);
            }
            else {
                LogAdd(NULL, "이미 전송 중입니다.");
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// WinMain: GUI 애플리케이션 엔트리
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MyClientWndClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(0, wc.lpszClassName, "파일 전송 클라이언트",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 430, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, nShowCmd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

