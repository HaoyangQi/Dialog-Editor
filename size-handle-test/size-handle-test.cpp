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

//BOOL bVisible = TRUE;

WINDOW_DESIGNER designerData;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    TargetProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void debugCheckError(int pos) {
    DWORD err = GetLastError();
    if (err != ERROR_SUCCESS) {
        wchar_t buf[100] = L"\0";
        swprintf_s(buf, 100, L"Error (at %d): %d\n", pos, err);
        OutputDebugString(buf);
        DebugBreak();
    }
}

void InitWindowDesigner(WINDOW_DESIGNER* pwd)
{
    BITMAP image;
    //HDC hdc;

    pwd->hwndMain = NULL;
    pwd->hwndTarget = NULL;
    pwd->bVisible = TRUE;

    pwd->hdcHandleDisable = CreateCompatibleDC(NULL);
    pwd->hdcHandleEnable = CreateCompatibleDC(NULL);
    pwd->hdcMask = CreateCompatibleDC(NULL);

    pwd->bmpHandleEnable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HANDLE_ENABLE));
    pwd->bmpHandleDisable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HANDLE_DISABLE));

    // Fetch image dimension
    GetObject(pwd->bmpHandleEnable, sizeof(BITMAP), &image);
    pwd->imgWidth = image.bmWidth;
    pwd->imgHeight = image.bmHeight;

    pwd->bmpMask = CreateBitmap(image.bmWidth, image.bmHeight, 1, 1, NULL);

    // Bind objects to DC and book-keep old object
    pwd->oldHandleDisable = SelectObject(pwd->hdcHandleDisable, pwd->bmpHandleDisable);
    pwd->oldHandleEnable = SelectObject(pwd->hdcHandleEnable, pwd->bmpHandleEnable);
    pwd->oldMask = SelectObject(pwd->hdcMask, pwd->bmpMask);
}

void ReleaseWindowDesigner(WINDOW_DESIGNER* pwd)
{
    // detach objects
    SelectObject(pwd->hdcHandleDisable, pwd->oldHandleDisable);
    SelectObject(pwd->hdcHandleEnable, pwd->oldHandleEnable);
    SelectObject(pwd->hdcMask, pwd->oldMask);

    // release objects
    DeleteObject(pwd->bmpHandleDisable);
    DeleteObject(pwd->bmpHandleEnable);
    DeleteObject(pwd->bmpMask);

    // release DCs
    DeleteDC(pwd->hdcHandleDisable);
    DeleteDC(pwd->hdcHandleEnable);
    DeleteDC(pwd->hdcMask);
}

BOOL DrawHandle(WINDOW_DESIGNER* pwd, HDC hdc, int x, int y, BOOL bEnable)
{
    BOOL ret = TRUE;
    HDC hdcHandle = bEnable ? pwd->hdcHandleEnable : pwd->hdcHandleDisable;

    // When copying to mask, SetBkColor will set a color that cannot be represented in mask 
    // to be converted to the default background color, which is white
    // As for other color cannot be represented, they will be converted to forground color by default
    SetBkColor(hdcHandle, RGB(255, 0, 255));

    ret &= BitBlt(pwd->hdcMask, 0, 0, pwd->imgWidth, pwd->imgHeight, hdcHandle, 0, 0, SRCCOPY);
    ret &= BitBlt(hdc, x, y, pwd->imgWidth, pwd->imgHeight, hdcHandle, 0, 0, SRCINVERT);
    ret &= BitBlt(hdc, x, y, pwd->imgWidth, pwd->imgHeight, pwd->hdcMask, 0, 0, SRCAND);
    ret &= BitBlt(hdc, x, y, pwd->imgWidth, pwd->imgHeight, hdcHandle, 0, 0, SRCINVERT);

    return ret;
}

BOOL DrawWindowHandles(WINDOW_DESIGNER* pwd, HDC hdc, HWND target, int dd, LONG flagEnableHandles)
{
    BOOL ret = TRUE;
    RECT rcTarget;
    LONG width, height, step_x, step_y;
    //HDC hdc;

    if (!IsChild(pwd->hwndMain, target)) {
        return FALSE;
    }

    GetWindowRect(target, &rcTarget);
    MapWindowPoints(HWND_DESKTOP, pwd->hwndMain, (LPPOINT)&rcTarget, 2);
    InflateRect(&rcTarget, dd, dd);

    width = rcTarget.right - rcTarget.left;
    height = rcTarget.bottom - rcTarget.top;
    step_x = (width - dd) / 2;
    step_y = (height - dd) / 2;

    //hdc = GetDC(parent);

    for (int y = rcTarget.top, index = 0; y <= height; y += step_y) {
        for (int x = rcTarget.left; x <= width; x += step_x) {
            // skip center
            if (x == rcTarget.left + step_x && y == rcTarget.top + step_y) {
                continue;
            }

            ret &= DrawHandle(pwd, hdc, x, y, flagEnableHandles & 1);
            flagEnableHandles >>= 1;
        }
    }
    
    //ReleaseDC(parent, hdc);

    return ret;
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

   InitWindowDesigner(&designerData);

   designerData.hwndMain = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW/*WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS*/,
      50, 50, 700, 400, nullptr, nullptr, hInstance, nullptr);

   if (!designerData.hwndMain)
   {
      return FALSE;
   }

   ShowWindow(designerData.hwndMain, nCmdShow);
   UpdateWindow(designerData.hwndMain);

   designerData.hwndTarget = CreateWindowEx(0, szTargetLayerClass, szTargetTitle,
       WS_CAPTION | WS_CHILDWINDOW | WS_VISIBLE | WS_DISABLED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU,
       7, 7, 300, 250, designerData.hwndMain, NULL, hInst, NULL);

   if (!designerData.hwndTarget)
   {
       return FALSE;
   }

   ShowWindow(designerData.hwndTarget, nCmdShow);
   UpdateWindow(designerData.hwndTarget);

   // Force refresh everything
   UpdateWindow(designerData.hwndMain);

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
            LONG x = GET_X_LPARAM(lParam);
            LONG y = GET_Y_LPARAM(lParam);

            designerData.bVisible = FALSE;
            designerData.ptTrackStart.x = x;
            designerData.ptTrackStart.y = y;
            SetRect(&designerData.rcTrackPrev, x, y, x, y);

            // TODO: SetCapture
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        case WM_LBUTTONUP:
        {
            designerData.bVisible = TRUE;
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        case WM_MOUSEMOVE:
        {
            // TODO: Mouse Drag:
            // 1. hit a control
            // 2. hit nothing
            // either case: lock target window update, move child BB (case 1) or a track rect (case 2)
            // final: unlock target window, invalidate base window
            // track rect: black rect and white frame in MEM DC, XOR with client DC
            // => black part no effect, white part will invert the color
            // => when another move come, use original mem XOR with DC again, then update mem
            if (wParam & MK_LBUTTON) {
                LONG x = GET_X_LPARAM(lParam);
                LONG y = GET_Y_LPARAM(lParam);
                LONG trackW = abs(designerData.ptTrackStart.x - x);
                LONG trackH = abs(designerData.ptTrackStart.y - y);

                x = min(designerData.ptTrackStart.x, x);
                y = min(designerData.ptTrackStart.y, y);

                HDC hdc = GetDC(hWnd);

                DrawFocusRect(hdc, &designerData.rcTrackPrev);
                SetRect(&designerData.rcTrackPrev, x, y, x + trackW, y + trackH);
                DrawFocusRect(hdc, &designerData.rcTrackPrev);

                ReleaseDC(hWnd, hdc);
            }

            POINT pt;
            GetCursorPos(&pt);
            LRESULT ret = DefWindowProc(designerData.hwndTarget, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
            if (ret != HTNOWHERE) {
                wchar_t buf[100] = L"\0";
                swprintf_s(buf, 100, L"hit %d\n", ret);
                OutputDebugString(buf);
            }

            break;
        }
        case WM_SIZING:
        {
            //OutputDebugString(L"Sizing\n");
            designerData.bVisible = FALSE;
            break;
        }
        case WM_SIZE:
        {
            designerData.bVisible = TRUE;
            if (wParam == SIZE_RESTORED) {
                ;// UpdateTrackBufferSize(&designerData, hWnd);
            }
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc;
            
            BeginPaint(hWnd, &ps);

            if (designerData.bVisible) {
                hdc = GetDC(hWnd);
                DrawWindowHandles(&designerData, hdc, designerData.hwndTarget, 6, ENABLE_RIGHTBOTTOM);
                ReleaseDC(hWnd, hdc);
            }

            EndPaint(hWnd, &ps);
            break;
        }
    case WM_DESTROY:
        ReleaseWindowDesigner(&designerData);
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
    /*case WM_NCHITTEST:
    {
        // if is a control: only return HTCAPTION (when inside) or HT(BORDER)
        LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
        if (hit != HTNOWHERE) {
            hit = HTCAPTION;
        }
        return hit;
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
    }*/
    case WM_PAINT:
    {
        //OutputDebugString(L"Target Paint\n");
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        
        if (!DrawHandle(&designerData, hdc, 0, 0, FALSE)) {
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
