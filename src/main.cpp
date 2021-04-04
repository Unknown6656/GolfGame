#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <WinUser.h>
#include <gdiplusgraphics.h>


int paint(const HWND hwnd)
{
    const Graphics* graphics = Graphics::FromHWND(hwnd);


    return 0;
}

int main(const int argc, const char** argv)
{
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = { };

    wc.hInstance = nullptr;
    wc.lpszClassName = CLASS_NAME;
    wc.lpfnWndProc = [](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        switch (uMsg)
        {
            case WM_PAINT:
                return paint(hwnd);
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        return 0;
    };

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                           // Optional window styles.
        CLASS_NAME,                  // Window class
        L"Learn to Program Windows", // Window text
        WS_OVERLAPPEDWINDOW,         // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        nullptr, // Parent window    
        nullptr, // Menu
        nullptr, // Instance handle
        nullptr  // Additional application data
    );

    if (!hwnd)
        return -1;

    ShowWindow(hwnd, SW_SHOW);

    MSG msg = { };
    msg.wParam = 0;

    while (IsWindow(hwnd) && GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
