#include "framework.h"
#include "windesign.h"
//#include "sizebox.h"

static HINSTANCE appInstance;
static WCHAR szDesignerWindowClass[MAX_LOADSTRING];
static WCHAR szTargetLayerClass[MAX_LOADSTRING];
static WCHAR szTargetTitle[MAX_LOADSTRING];
static HWND hwndMain, hwndTarget;
//static HDC hdc, hdcMem;

ATOM RegisterDesignWindowClass(HINSTANCE hInstance);
ATOM RegisterDesignTargetClass(HINSTANCE hInstance);
LRESULT CALLBACK DesignWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DesignTargetWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Two Layers: 
// 1. Bottom layer: the base of the control
// 2. Target layer: the demo child window
BOOL InitWindowDesignerInstance(HINSTANCE hInstance, HWND hParent, int nCmdShow)
{
    appInstance = hInstance;

    LoadString(hInstance, IDS_TARGET_LAYER_TITLE, szTargetTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_WINDOW_DESIGNER, szDesignerWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDC_TARGET_LAYER, szTargetLayerClass, MAX_LOADSTRING);
    //LoadString(hInstance, IDC_DESIGN_LAYER, szDesignLayerClass, MAX_LOADSTRING);

    if (!RegisterDesignWindowClass(hInstance)) {
        return FALSE;
    }

    if (!RegisterDesignTargetClass(hInstance)) {
        return FALSE;
    }

    // this part needs to go WM_CREATE message
    // bottom base pane should not have WS_CLIPCHILDREN so that all drawings are above children
    hwndMain = CreateWindowEx(WS_EX_TRANSPARENT, szDesignerWindowClass, NULL,
        WS_VISIBLE | WS_CHILDWINDOW | WS_CLIPSIBLINGS,
        0, 0, 400, 400, hParent, NULL, appInstance, NULL);

    // target window should be a standard window but only disabled
    hwndTarget = CreateWindowEx(0, szTargetLayerClass, szTargetTitle,
        WS_CAPTION | WS_CHILDWINDOW | WS_VISIBLE | 0/*WS_DISABLED*/ | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU,
        0, 0, 300, 250, hwndMain, NULL, appInstance, NULL);

    if (!hwndMain || !hwndTarget) {
        return FALSE;
    }

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);
    ShowWindow(hwndTarget, nCmdShow);
    UpdateWindow(hwndTarget);

    return TRUE;
}

ATOM RegisterDesignWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = DesignWindowWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szDesignerWindowClass;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

ATOM RegisterDesignTargetClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = DesignTargetWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szTargetLayerClass;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

LRESULT CALLBACK DesignWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        OutputDebugString(L"Main Draw call\n");
        // Use BeginPaint to fetch DC will set clip region no matter what
        // But using GetDC is dangerous as it cannot validate region and pop WM_PAINT request (high cpu)
        // So simple trade-off: use BeginPaint/EndPaint as a guard, additionally GetDC, then draw
        // further optimization: double-buffering
        PAINTSTRUCT ps;
        HDC hdc;
        hdc = BeginPaint(hWnd, &ps);
        //hdc = GetWindowDC(hWnd); // DC of entire window
        //hdc = GetDC(hWnd);

        /* Double befuerring template code
        
        // Create an off-screen DC for double-buffering
        hdcMem = CreateCompatibleDC(hdc);
        hbmMem = CreateCompatibleBitmap(hdc, win_width, win_height);

        hOld = SelectObject(hdcMem, hbmMem);

        // Draw into hdcMem here

        // Transfer the off-screen DC to the screen
        BitBlt(hdc, 0, 0, win_width, win_height, hdcMem, 0, 0, SRCCOPY);

        // Free-up the off-screen DC
        SelectObject(hdcMem, hOld);

        DeleteObject(hbmMem);
        DeleteDC (hdcMem);

        */

        RECT rect;
        SelectObject(hdc, GetStockObject(DC_PEN));
        SetDCPenColor(hdc, RGB(255, 0, 0));
        GetClientRect(hWnd, &rect);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, rect.right, rect.bottom);

        //WindowDesignerRefreshEx(hdc);

        ReleaseDC(hWnd, hdc);
        EndPaint(hWnd, &ps);
        //WindowDesignerRefresh();
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    /*if (message == WM_PAINT) {
        OutputDebugString(L"Override\n");
        WindowDesignerRefresh();
    }*/

    return 0;
}

LRESULT CALLBACK DesignTargetWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        OutputDebugString(L"Target Draw call\n");
        EndPaint(hWnd, &ps);
        //WindowDesignerRefresh();
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void WindowDesignerRefresh()
{
    HDC hdc, hdcMem;
    HBITMAP bitmapMem;
    //HGDIOBJ objOld;
    RECT rect;
    LONG clientWidth, clientHeight;

    RedrawWindow(hwndMain, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    GetClientRect(hwndMain, &rect);

    OutputDebugString(L"Override\n");

    clientWidth = rect.right - rect.left;
    clientHeight = rect.bottom - rect.top;
    hdc = GetDC(hwndMain);
    hdcMem = CreateCompatibleDC(hdc);
    bitmapMem = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
    //objOld = SelectObject(hdcMem, bitmapMemMask);

    SelectObject(hdcMem, GetStockObject(DC_PEN));

    SetDCPenColor(hdcMem, RGB(0, 0, 255));
    MoveToEx(hdcMem, 0, 0, NULL);
    LineTo(hdcMem, rect.right, rect.bottom);

    TransparentBlt(hdc, 0, 0, clientWidth, clientHeight, hdcMem, 0, 0, clientWidth, clientHeight, RGB(0, 0, 0));

    //SelectObject(hdcMem, objOld);
    DeleteObject(bitmapMem);
    DeleteDC(hdcMem);
    ReleaseDC(hwndMain, hdc);
}

void WindowDesignerRefresh2()
{
    HDC hdc, hdcMem, hdcMask;
    HBITMAP bitmapMem, bitmapMemMask;
    //HGDIOBJ objOld;
    RECT rect;
    LONG clientWidth, clientHeight;

    RedrawWindow(hwndMain, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    GetClientRect(hwndMain, &rect);

    OutputDebugString(L"Override\n");

    clientWidth = rect.right - rect.left;
    clientHeight = rect.bottom - rect.top;
    hdc = GetDC(hwndMain);
    hdcMem = CreateCompatibleDC(hdc);
    hdcMask = CreateCompatibleDC(hdc);
    bitmapMem = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
    bitmapMemMask = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
    //objOld = SelectObject(hdcMem, bitmapMemMask);

    SelectObject(hdcMask, bitmapMem);
    SelectObject(hdcMask, GetStockObject(DC_PEN));
    SetDCPenColor(hdcMask, RGB(0, 0, 0));
    SelectObject(hdcMem, bitmapMemMask);
    SelectObject(hdcMem, GetStockObject(DC_PEN));
    FillRect(hdcMask, &rect, GetStockBrush(WHITE_BRUSH));

    MoveToEx(hdcMask, 0, 0, NULL);
    LineTo(hdcMask, rect.right, rect.bottom);

    SetDCPenColor(hdcMem, RGB(0, 0, 255));
    MoveToEx(hdcMem, 0, 0, NULL);
    LineTo(hdcMem, rect.right, rect.bottom);

    BitBlt(hdc, 0, 0, clientWidth, clientHeight, hdcMask, 0, 0, SRCAND);
    BitBlt(hdc, 0, 0, clientWidth, clientHeight, hdcMem, 0, 0, SRCPAINT);

    //SelectObject(hdcMem, objOld);
    DeleteObject(bitmapMem);
    DeleteObject(bitmapMemMask);
    DeleteDC(hdcMem);
    DeleteDC(hdcMask);
    ReleaseDC(hwndMain, hdc);
}

void WindowDesignerRefreshEx(HDC hdcMain)
{
    OutputDebugString(L"Override\n");
    HDC hdcMemMain, hdcTarget;
    HBITMAP bitmapMem;
    HGDIOBJ objOld;
    RECT rect, rectTargetWindow;
    LONG clientWidth, clientHeight;

    InvalidateRect(hwndTarget, NULL, TRUE);

    GetClientRect(hwndMain, &rect);
    GetWindowRect(hwndTarget, &rectTargetWindow);
    MapWindowPoints(HWND_DESKTOP, hwndMain, (LPPOINT)&rectTargetWindow, 2);

    clientWidth = rect.right - rect.left;
    clientHeight = rect.bottom - rect.top;
    hdcTarget = GetWindowDC(hwndTarget);
    hdcMemMain = CreateCompatibleDC(hdcMain);
    bitmapMem = CreateCompatibleBitmap(hdcMain, clientWidth, clientHeight);
    objOld = SelectObject(hdcMemMain, bitmapMem);

    SelectObject(hdcMemMain, GetStockObject(DC_PEN));
    SetDCPenColor(hdcMemMain, RGB(0, 255, 255));
    MoveToEx(hdcMemMain, rect.right, rect.top, NULL);
    LineTo(hdcMemMain, rect.left, rect.bottom);

    BitBlt(hdcMain, 0, 0, clientWidth, clientHeight, hdcMemMain, 0, 0, SRCPAINT);
    BitBlt(hdcTarget, 0, 0, rectTargetWindow.right-rectTargetWindow.left, rectTargetWindow.bottom-rectTargetWindow.top, hdcMemMain, rectTargetWindow.left, rectTargetWindow.top, SRCCOPY);

    SelectObject(hdcMemMain, objOld);
    DeleteObject(bitmapMem);
    DeleteDC(hdcMemMain);
    ReleaseDC(hwndTarget, hdcTarget);
}

// DEBUG ONLY
void MoveTargetWindow(int delta_horizontal, int delta_vertical)
{
    RECT rect;
    GetWindowRect(hwndTarget, &rect);
    MapWindowPoints(HWND_DESKTOP, hwndMain, (LPPOINT)&rect, 2);

    rect.left += delta_horizontal;
    rect.top += delta_vertical;
    rect.right += delta_horizontal;
    rect.bottom += delta_vertical;
    MoveWindow(hwndTarget, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
    //InvalidateRect(hwndMain, NULL, TRUE);
    //WindowDesignerRefresh();
}

void ScaleTargetWindow(int delta_width, int delta_height)
{
    RECT rect;
    GetWindowRect(hwndTarget, &rect);
    MapWindowPoints(HWND_DESKTOP, hwndMain, (LPPOINT)&rect, 2);

    rect.right += delta_width;
    rect.bottom += delta_height;
    MoveWindow(hwndTarget, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
    //OutputDebugString(L"Invalidate main area\n");
    //InvalidateRect(hwndMain, NULL, TRUE);
}
