#define UNICODE
#define _UNICODE
#include <windows.h>
#include <hidsdi.h>
#include <strsafe.h>

// Global state variables for two independent cursors
POINT cursorPos1 = {0, 0};
POINT cursorPos2 = {0, 0};

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Función helper para debug
void DebugLog(const wchar_t* format, ...) {
    wchar_t buffer[1024];
    va_list args;
    va_start(args, format);
    StringCchVPrintfW(buffer, 1024, format, args);
    va_end(args);
    OutputDebugStringW(buffer);
}

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "hid.lib")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    DebugLog(L"Iniciando aplicación...\n");
    
    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DualCursorOverlay";
    
    if (!RegisterClassEx(&wc)) {
        DebugLog(L"Error: Window Registration Failed!\n");
        MessageBox(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    DebugLog(L"Ventana registrada exitosamente\n");

    // Create window
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT,
        L"DualCursorOverlay",
        L"Dual Cursor Window",
        WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        DebugLog(L"Error: Window Creation Failed!\n");
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    DebugLog(L"Ventana creada exitosamente\n");

    // Make window transparent
    SetLayeredWindowAttributes(hwnd, 0, 1, LWA_ALPHA);
    ShowWindow(hwnd, nCmdShow);

    // Register Raw Input devices
    RAWINPUTDEVICE rid[1];
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = hwnd;

    if (!RegisterRawInputDevices(rid, 1, sizeof(rid[0]))) {
        MessageBox(NULL, L"Raw Input Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INPUT: {
        DebugLog(L"Recibiendo entrada de mouse. Pos1: (%d,%d), Pos2: (%d,%d)\n", 
                 cursorPos1.x, cursorPos1.y, cursorPos2.x, cursorPos2.y);
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        LPBYTE lpb = new BYTE[dwSize];
        if (lpb == NULL) { break; }
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
        {
            delete[] lpb;
            break;
        }
        RAWINPUT* raw = (RAWINPUT*)lpb;
        if(raw->header.dwType == RIM_TYPEMOUSE) {
            static bool assignToFirst = true;
            if(assignToFirst) {
                cursorPos1.x += raw->data.mouse.lLastX;
                cursorPos1.y += raw->data.mouse.lLastY;
            } else {
                cursorPos2.x += raw->data.mouse.lLastX;
                cursorPos2.y += raw->data.mouse.lLastY;
            }
            assignToFirst = !assignToFirst;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        delete[] lpb;
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
} 