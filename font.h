#pragma once

// #include <windows.h>
//
// static HFONT g_default_ui_font = nullptr;
//
// static HFONT CreateDefaultUiFont(HWND hwnd)
// {
//     NONCLIENTMETRICSW metrics{};
//     metrics.cbSize = sizeof(metrics);
//
//     LOGFONTW font{};
//     if (SystemParametersInfoW(
//             SPI_GETNONCLIENTMETRICS,
//             metrics.cbSize,
//             &metrics,
//             0))
//     {
//         font = metrics.lfMessageFont;
//     }
//     else
//     {
//         HDC dc = GetDC(hwnd);
//         const int dpi_y = dc ? GetDeviceCaps(dc, LOGPIXELSY) : 96;
//         if (dc)
//         {
//             ReleaseDC(hwnd, dc);
//         }
//
//         // This logical face maps to the platform's native dialog font.
//         font.lfHeight = -MulDiv(9, dpi_y, 72);
//         font.lfWeight = FW_NORMAL;
//         font.lfCharSet = DEFAULT_CHARSET;
//         lstrcpynW(font.lfFaceName, L"MS Shell Dlg 2", LF_FACESIZE);
//     }
//
//     // Grayscale antialiasing behaves consistently on Windows and Wine.
//     font.lfQuality = ANTIALIASED_QUALITY;
//     return CreateFontIndirectW(&font);
// }
//
// static BOOL CALLBACK ApplyDefaultUiFontToChild(HWND child, LPARAM font)
// {
//     SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(font), TRUE);
//     return TRUE;
// }
//
// static void ApplyDefaultUiFont(HWND parent)
// {
//     if (!g_default_ui_font)
//     {
//         g_default_ui_font = CreateDefaultUiFont(parent);
//     }
//
//     if (g_default_ui_font)
//     {
//         EnumChildWindows(
//             parent,
//             ApplyDefaultUiFontToChild,
//             reinterpret_cast<LPARAM>(g_default_ui_font)
//         );
//     }
// }
//
// static void DestroyDefaultUiFont()
// {
//     if (g_default_ui_font)
//     {
//         DeleteObject(g_default_ui_font);
//         g_default_ui_font = nullptr;
//     }
// }

#include <windows.h>

static HFONT g_default_ui_font = nullptr;

static HFONT CreateDefaultUiFont(HWND hwnd)
{
    NONCLIENTMETRICSW metrics{};
    metrics.cbSize = sizeof(metrics);

    if (SystemParametersInfoW(
            SPI_GETNONCLIENTMETRICS,
            sizeof(metrics),
            &metrics,
            0))
    {
        return CreateFontIndirectW(&metrics.lfMessageFont);
    }

    HDC hdc = GetDC(hwnd);
    const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(hwnd, hdc);

    return CreateFontW(
        -MulDiv(9, dpiY, 72),
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
}

static void ApplyFont(HWND hwnd, HFONT font)
{
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

static void ApplyDefaultUiFont(HWND parent)
{
    if (!g_default_ui_font)
    {
        g_default_ui_font = CreateDefaultUiFont(parent);
    }

    ApplyFont(parent, g_default_ui_font);

    EnumChildWindows(parent, [](HWND child, LPARAM fontParam) -> BOOL
    {
        auto font = reinterpret_cast<HFONT>(fontParam);
        SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        return TRUE;
    }, reinterpret_cast<LPARAM>(g_default_ui_font));
}

static void DestroyDefaultUiFont()
{
    if (g_default_ui_font)
    {
        DeleteObject(g_default_ui_font);
        g_default_ui_font = nullptr;
    }
}
