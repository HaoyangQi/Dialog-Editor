#include "framework.h"
#include "windesign.h"

static HINSTANCE appInstance;
static WCHAR szDesignerWindowClass[MAX_LOADSTRING];
static WCHAR szTargetLayerClass[MAX_LOADSTRING];
static WCHAR szDesignLayerClass[MAX_LOADSTRING];
static WCHAR szTargetTitle[MAX_LOADSTRING];
static HWND hwndMain, hwndTarget, hwndDesign;

ATOM RegisterDesignWindowClass(HINSTANCE hInstance);
ATOM RegisterDesignTargetClass(HINSTANCE hInstance);
ATOM RegisterDesignLayerClass(HINSTANCE hInstance);
LRESULT CALLBACK DesignWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DesignTargetWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DesignLayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Three window: 
// 1. Bottom layer: the base of the control
// 2. Target layer: the demo child window
// 3. Design layer: a transparent layer, always on top-most
BOOL InitWindowDesignerInstance(HINSTANCE hInstance, HWND hParent, int nCmdShow)
{
    appInstance = hInstance;

    LoadString(hInstance, IDS_TARGET_LAYER_TITLE, szTargetTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_WINDOW_DESIGNER, szDesignerWindowClass, MAX_LOADSTRING);
    LoadString(hInstance, IDC_TARGET_LAYER, szTargetLayerClass, MAX_LOADSTRING);
    LoadString(hInstance, IDC_DESIGN_LAYER, szDesignLayerClass, MAX_LOADSTRING);

    if (!RegisterDesignWindowClass(hInstance)) {
        return FALSE;
    }

    if (!RegisterDesignTargetClass(hInstance)) {
        return FALSE;
    }

    if (!RegisterDesignLayerClass(hInstance)) {
        return FALSE;
    }

    hwndMain = CreateWindowEx(WS_EX_CLIENTEDGE, szDesignerWindowClass, NULL,
        WS_BORDER | WS_CHILD,
        0, 0, 400, 400, hParent, NULL, appInstance, NULL);

    hwndTarget = CreateWindow(szTargetLayerClass, szTargetTitle,
        WS_CAPTION | WS_BORDER | WS_CHILD | WS_DISABLED,
        0, 0, 300, 250, hwndMain, NULL, appInstance, NULL);

    // TODO: make size always same as main window
    hwndDesign = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT, szDesignLayerClass, NULL,
        WS_CHILD,
        0, 0, 400, 400, hwndMain, NULL, appInstance, NULL);

    if (!hwndMain || !hwndTarget || !hwndDesign) {
        return FALSE;
    }

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);
    ShowWindow(hwndTarget, nCmdShow);
    UpdateWindow(hwndTarget);
    ShowWindow(hwndDesign, nCmdShow);
    UpdateWindow(hwndDesign);

    // force redraw once along the hierachy to make sure everything is in correct order
    InvalidateRect(hwndMain, NULL, TRUE);

    return TRUE;
}

ATOM RegisterDesignWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
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

ATOM RegisterDesignLayerClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DesignLayerWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CUR_MAIN));
    wcex.hbrBackground = NULL; // empty background
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szDesignLayerClass;
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
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        SelectObject(hdc, GetStockObject(DC_PEN));
        SetDCPenColor(hdc, RGB(255, 0, 0));
        GetClientRect(hWnd, &rect);
        MoveToEx(hdc, rect.right, rect.top, NULL);
        LineTo(hdc, 0, rect.bottom - rect.top);

        EndPaint(hWnd, &ps);

        // update topmost layer to avoid any overlapping
        // InvalidateRect(hwndDesign, NULL, TRUE);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
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
    /*case WM_WINDOWPOSCHANGING:
    {
        LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
        RECT rect;

        GetDesignWindowClientPos(hWnd, &rect);

        // Prevent child window from being moved by user
        if (!isValidUpdate) {
            pos->x = rect.left;
            pos->y = rect.top;
            pos->cx = rect.right - rect.left;
            pos->cy = rect.bottom - rect.top;
        }
        else {
            isValidUpdate = FALSE;
        }
    }
    break;*/
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        SelectObject(hdc, GetStockObject(DC_PEN));
        SetDCPenColor(hdc, RGB(0, 255, 0));
        GetClientRect(hWnd, &rect);
        MoveToEx(hdc, rect.right, rect.top, NULL);
        LineTo(hdc, 0, rect.bottom - rect.top);

        EndPaint(hWnd, &ps);

        // update topmost layer to avoid any overlapping
        // InvalidateRect(hwndDesign, NULL, TRUE);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK DesignLayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        SelectObject(hdc, GetStockObject(DC_PEN));
        SetDCPenColor(hdc, RGB(0, 0, 255));
        GetClientRect(hWnd, &rect);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, rect.right - rect.left, rect.bottom - rect.top);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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
    InvalidateRect(hwndMain, NULL, TRUE);
}

void ScaleTargetWindow(int delta_width, int delta_height)
{
    RECT rect;
    GetWindowRect(hwndTarget, &rect);
    MapWindowPoints(HWND_DESKTOP, hwndMain, (LPPOINT)&rect, 2);

    rect.right += delta_width;
    rect.bottom += delta_height;
    MoveWindow(hwndTarget, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
    InvalidateRect(hwndMain, NULL, TRUE);
}
