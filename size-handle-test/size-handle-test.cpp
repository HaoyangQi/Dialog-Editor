// size-handle-test.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "size-handle-test.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR szTargetLayerClass[MAX_LOADSTRING] = L"TargetWindowTest";
WCHAR szTargetTitle[MAX_LOADSTRING] = L"Target Test Window";

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    TargetProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

BOOL DrawHandle(HDC hdc, int x, int y, BOOL bEnable)
{
    HDC hdcHandle, hdcMask; //, hdcMem
    HBITMAP bmpHandleEnable, bmpHandleDisable, bmpMask; //, bmpMem
    HGDIOBJ bmpHandleOld, bmpMaskOld; //, bmpMemOld
    BITMAP image;
    LONG width, height;
    
    //hdcMem = CreateCompatibleDC(NULL);
    hdcMask = CreateCompatibleDC(NULL);
    hdcHandle = CreateCompatibleDC(NULL);

    bmpHandleEnable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HANDLE_ENABLE));
    bmpHandleDisable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HANDLE_DISABLE));

    GetObject(bmpHandleEnable, sizeof(BITMAP), &image);
    width = image.bmWidth;
    height = image.bmHeight;
    bmpMask = CreateBitmap(width, height, 1, 1, NULL);
    //bmpMem = CreateCompatibleBitmap(hdc, width, height);

    //bmpMemOld = SelectObject(hdcMem, bmpMem);
    bmpHandleOld = SelectObject(hdcHandle, bEnable ? bmpHandleEnable : bmpHandleDisable);
    bmpMaskOld = SelectObject(hdcMask, bmpMask);

    // When copying to mask, SetBkColor will set a color that cannot be represented in mask 
    // to be converted to the default background color, which is white
    // As for other color cannot be represented, they will be converted to forground color by default
    SetBkColor(hdcHandle, RGB(255, 0, 255));
    BitBlt(hdcMask, 0, 0, width, height, hdcHandle, 0, 0, SRCCOPY);
/*
    // Method 1, 1 access to DC
    BitBlt(hdcMem, 0, 0, width, height, hdc, x, y, SRCCOPY);
    BitBlt(hdcMem, 0, 0, width, height, hdcBlank, 0, 0, SRCINVERT);
    BitBlt(hdcMem, 0, 0, width, height, hdcMask, 0, 0, SRCAND);
    BitBlt(hdcMem, 0, 0, width, height, hdcHandle, 0, 0, SRCINVERT);
    BitBlt(hdc, x, y, width, height, hdcMem, 0, 0, SRCCOPY);
*/

    // Method 2, 3 access to DC
    BitBlt(hdc, x, y, width, height, hdcHandle, 0, 0, SRCINVERT);
    BitBlt(hdc, x, y, width, height, hdcMask, 0, 0, SRCAND);
    BitBlt(hdc, x, y, width, height, hdcHandle, 0, 0, SRCINVERT);

    //SelectObject(hdcMem, bmpMemOld);
    SelectObject(hdcHandle, bmpHandleOld);
    SelectObject(hdcMask, bmpMaskOld);
    //DeleteObject(bmpMemOld);
    DeleteObject(bmpHandleOld);
    DeleteObject(bmpMaskOld);
    //DeleteDC(hdcMem);
    DeleteDC(hdcHandle);
    DeleteDC(hdcMask);

    return TRUE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SIZEHANDLETEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIZEHANDLETEST));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIZEHANDLETEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SIZEHANDLETEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassExW(&wcex)) {
        return 0;
    }

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = TargetProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szTargetLayerClass;
    wcex.hIconSm        = NULL;

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   HWND hTarget = CreateWindowEx(0, szTargetLayerClass, szTargetTitle,
       WS_CAPTION | WS_CHILDWINDOW | WS_VISIBLE | WS_DISABLED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU,
       5, 5, 300, 250, hWnd, NULL, hInst, NULL);

   if (!hTarget)
   {
       return FALSE;
   }

   ShowWindow(hTarget, nCmdShow);
   UpdateWindow(hTarget);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            if (!DrawHandle(hdc, 0, 0, TRUE)) {
                OutputDebugString(L"Error drawing handle\n");
            }

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

LRESULT CALLBACK TargetProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        
        if (!DrawHandle(hdc, 0, 0, FALSE)) {
            OutputDebugString(L"Error drawing handle\n");
        }

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

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
