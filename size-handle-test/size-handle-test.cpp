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
HWND hTarget = NULL;
BOOL isPressedLB = FALSE;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    TargetProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// bitblt: set src to NULL and use WHITNESS or BLACKNESS to clear buffer
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

// if want to stop WM_ERASEBKGND from repainting, set hbrBackGround to 
// (HBRUSH)GetStockObject(HOLLOW_BRUSH);
// so that the WM_PAINT can use double-bufferring to refresh client area and avoid flikering
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
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW/*WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS*/,
      50, 50, 700, 400, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   hTarget = CreateWindowEx(0, szTargetLayerClass, szTargetTitle,
       WS_CAPTION | WS_CHILDWINDOW | WS_VISIBLE | WS_DISABLED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU,
       7, 7, 300, 250, hWnd, NULL, hInst, NULL);

   if (!hTarget)
   {
       return FALSE;
   }

   ShowWindow(hTarget, nCmdShow);
   UpdateWindow(hTarget);

   HDC hdc = GetDC(hWnd);
   DrawHandle(hdc, 0, 0, TRUE);
   ReleaseDC(hWnd, hdc);

   return TRUE;
}

// TODO: Mouse inside moving, post WM_NCHITTEST message to target, x, y subtract by handle size
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            CreateWindow(L"BUTTON", L"Move Left", WS_VISIBLE | WS_CHILD,
                310, 10, 100, 20, hWnd, (HMENU)1000, hInst, nullptr);
            break;
        }
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
            case 1000:
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_LBUTTONDOWN:
        {
            OutputDebugString(L"Parent Mouse Down\n");
            isPressedLB = TRUE;
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        case WM_LBUTTONUP:
        {
            OutputDebugString(L"Parent Mouse Up\n");
            isPressedLB = FALSE;
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        /*case WM_ERASEBKGND:
        {
            OutputDebugString(L"Parent Paint Erase\n");
            break;
        }*/
        case WM_PAINT:
        {
            OutputDebugString(L"Parent Paint\n");

            RECT rect, rcTarget;
            LONG width;
            LONG height;

            GetClientRect(hWnd, &rect);
            GetWindowRect(hTarget, &rcTarget);
            MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&rcTarget, 2);

            /*width = rect.right - rect.left;
            height = rect.bottom - rect.top;

            // Inflate invalid target window region
            InflateRect(&rcTarget, 10, 10);
            InvalidateRect(hWnd, NULL, FALSE);*/

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            /*HDC mem = CreateCompatibleDC(NULL);
            HBITMAP canvas = CreateCompatibleBitmap(hdc, width, height);
            HGDIOBJ old = SelectObject(mem, canvas);

            FillRect(mem, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
            DrawHandle(mem, rcTarget.left - 5, rcTarget.top - 5, TRUE);
            DrawHandle(mem, rcTarget.left, 0, TRUE);
            DrawHandle(mem, 0, rcTarget.top, FALSE);
            BitBlt(hdc, 0, 0, width, height, mem, 0, 0, SRCCOPY);

            SelectObject(mem, old);
            DeleteObject(canvas);
            DeleteDC(mem);*/

            EndPaint(hWnd, &ps);

            // post draw
            if (!isPressedLB) {
                InflateRect(&rcTarget, 6, 6);
                width = rcTarget.right - rcTarget.left;
                height = rcTarget.bottom - rcTarget.top;
                hdc = GetDC(hWnd);
                DrawHandle(hdc, rcTarget.left, rcTarget.top, TRUE);
                DrawHandle(hdc, rcTarget.left + width / 2 - 3, rcTarget.top, TRUE);
                DrawHandle(hdc, rcTarget.right - 6, rcTarget.top, TRUE);
                DrawHandle(hdc, rcTarget.left, rcTarget.top + height / 2 - 3, TRUE);
                //DrawHandle(hdc, rcTarget.left + width / 2 - 3, rcTarget.top + height / 2 - 3, TRUE);
                DrawHandle(hdc, rcTarget.right - 6, rcTarget.top + height / 2 - 3, TRUE);
                DrawHandle(hdc, rcTarget.left, rcTarget.bottom - 6, TRUE);
                DrawHandle(hdc, rcTarget.left + width / 2 - 3, rcTarget.bottom - 6, TRUE);
                DrawHandle(hdc, rcTarget.right - 6, rcTarget.bottom - 6, TRUE);
                ReleaseDC(hWnd, hdc);
            }

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
    case WM_NCHITTEST:
    {
        // if is a control: only return HTCAPTION (when inside) or HT(BORDER)
        /*LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
        if (hit != HTNOWHERE) {
            hit = HTCAPTION;
        }
        return hit;*/
        break;
    }
    case WM_LBUTTONDOWN:
    {
        OutputDebugString(L"Target Mouse Down\n");
        isPressedLB = TRUE;
        InvalidateRect(GetParent(hWnd), NULL, TRUE);
        //return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    case WM_LBUTTONUP:
    {
        OutputDebugString(L"Target Mouse Up\n");
        isPressedLB = FALSE;
        InvalidateRect(GetParent(hWnd), NULL, TRUE);
        return DefWindowProc(hWnd, message, wParam, lParam);
        //break;
    }
    case WM_ERASEBKGND:
    {
        OutputDebugString(L"Target Erase\n");
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    case WM_PAINT:
    {
        OutputDebugString(L"Target Paint\n");
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
