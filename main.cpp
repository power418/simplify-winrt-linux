#include <windows.h>
#include <commctrl.h>
#include <roapi.h>
#include <hstring.h>
#include <winstring.h>

#include "font.h"

#include <cwchar>
#include <string>

#define ID_INPUT 1001
#define ID_BUTTON 1002
#define ID_OUTPUT 1003

static HWND g_input = nullptr;
static HWND g_output = nullptr;

static BOOL g_winrt_initialized = false;
static HRESULT g_winrt_hr = E_FAIL;

static std::wstring HResultToText(HRESULT hr)
{
    wchar_t buff[64]{};
    wsprintfW(buff, L"0x%08lX", static_cast<unsigned long>(hr));
    return buff;
}

static void ShowInputAsWinRTHString(HWND hwnd)
{
    wchar_t input_buff[256]{};
    GetWindowTextW(g_input, input_buff, 256);

    if (input_buff[0] == L'\0')
    {
        SetWindowTextW(g_output, L"Input masih kosong.");
        MessageBoxW(hwnd, L"Input masih kosong.", L"Info", MB_OK | MB_ICONINFORMATION);
        return;
    }

    HSTRING hstr{};
    HRESULT hr = WindowsCreateString(
        input_buff,
        static_cast<UINT32>(wcslen(input_buff)),
        &hstr
    );

    if (FAILED(hr))
    {
        std::wstring err = L"WindowsCreateString failed: ";
        err += HResultToText(hr);

        SetWindowTextW(g_output, err.c_str());
        MessageBoxW(hwnd, err.c_str(), L"WinRT Error", MB_OK | MB_ICONERROR);
    }

    UINT32 len = 0;
    PCWSTR raw = WindowsGetStringRawBuffer(hstr, &len);

    std::wstring output = L"WinRT HSTRING value: ";
    output.append(raw, len);

    SetWindowTextW(g_output, output.c_str());
    MessageBoxW(g_output, output.c_str(), L"WinRT HSTRING", MB_OK | MB_ICONINFORMATION);

    WindowsDeleteString(hstr);
}

LRESULT CALLBACK WindProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            {
                CreateWindowExW(
                    0,
                    L"STATIC",
                    L"Simple MinGW + Raw WinRT + Wine",
                    WS_CHILD | WS_VISIBLE,
                    20, 20,
                    360, 24,
                    hwnd,
                    nullptr,
                    GetModuleHandleW(nullptr),
                    nullptr
                );

                CreateWindowExW(
                    0,
                    L"STATIC",
                    L"Enter text:",
                    WS_CHILD | WS_VISIBLE,
                    20, 60,
                    120, 24,
                    hwnd,
                    nullptr,
                    GetModuleHandleW(nullptr),
                    nullptr
                );

                g_input = CreateWindowExW(
                    WS_EX_CLIENTEDGE,
                    L"EDIT",
                    L"",
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                    20, 90,
                    320, 32,
                    hwnd,
                    reinterpret_cast<HMENU>(ID_INPUT),
                    GetModuleHandleW(nullptr),
                    nullptr
                );

                CreateWindowExW(
                    0,
                    L"BUTTON",
                    L"Convert to WinRT HSTRING",
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                    20, 140,
                    220, 36,
                    hwnd,
                    reinterpret_cast<HMENU>(ID_BUTTON),
                    GetModuleHandleW(nullptr),
                    nullptr
                );

                g_output = CreateWindowExW(
                    0,
                    L"STATIC",
                    L"Output will appear here.",
                    WS_CHILD | WS_VISIBLE,
                    20, 200,
                    600, 32,
                    hwnd,
                    reinterpret_cast<HMENU>(ID_OUTPUT),
                    GetModuleHandleW(nullptr),
                    nullptr
                );

                std::wstring status = L"WinRT init: ";
                status += SUCCEEDED(g_winrt_hr) ? L"OK" : L"FAILED ";
                if (FAILED(g_winrt_hr))
                {
                    status += HResultToText(g_winrt_hr);
                }

                CreateWindowExW(
                    0,
                    L"STATIC",
                    status.c_str(),
                    WS_CHILD | WS_VISIBLE,
                    20, 245,
                    600, 32,
                    hwnd,
                    nullptr,
                    GetModuleHandleW(nullptr),
                    nullptr
                );

                ApplyDefaultUiFont(hwnd);

                return 0;
            }
        case WM_COMMAND:
            {
                const int control_id = LOWORD(wParam);
                const int event_code = HIWORD(wParam);

                if (control_id == ID_BUTTON && event_code == BN_CLICKED)
                {
                    ShowInputAsWinRTHString(hwnd);
                    return 0;
                }
                break;
            }
        case WM_DESTROY:
            {
                DestroyDefaultUiFont();

                if (g_winrt_initialized)
                {
                    RoUninitialize();
                    g_winrt_initialized = false;
                }

                PostQuitMessage(0);
                return 0;
            }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR nCmdLine, INT nCmdShow)
{
    g_winrt_hr = RoInitialize(RO_INIT_SINGLETHREADED);
    g_winrt_initialized = SUCCEEDED(g_winrt_hr);

    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;

    InitCommonControlsEx(&icc);

    PCWSTR CLASS_NAME = L"MingGWRawWinRTwindowClass";

    WNDCLASSW wc{
        .style = 0,
        .lpfnWndProc = WindProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInst,
        .hIcon = nullptr,
        .hCursor = LoadCursorW(nullptr, IDC_ARROW),
        .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
        .lpszMenuName = nullptr,
        .lpszClassName = CLASS_NAME
    };

    if (!RegisterClassW(&wc))
    {
        MessageBoxW(nullptr, L"RegisterClassW failed.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    RECT rect{ 0, 0, 800, 640 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"MinGW Raw WinRT App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInst,
        nullptr
    );

    if (!hwnd)
    {
        MessageBoxW(nullptr, L"CreateWindowExW failed.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG message {};
    while (GetMessage(&message, nullptr, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return static_cast<int>(message.wParam);
}
