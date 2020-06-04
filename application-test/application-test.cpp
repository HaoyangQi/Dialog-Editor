#include "framework.h"
#include "application-test.h"
#include "windesign.h"
#include "sizebox.h"

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
static HWND hDesignWindow;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_APPLICATIONTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    //RegisterDesignWindowClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_APPLICATIONTEST));

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



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATIONTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW-1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_APPLICATIONTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
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

   if (!InitWindowDesignerInstance(hInst, hWnd, nCmdShow)) {
       return FALSE;
   }

   InitSizeBoxInstance(hInstance, hWnd, nCmdShow);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{    
    switch (message)
    {
    case WM_CREATE:
    {
        CreateWindowW(L"BUTTON", L"Move Left", WS_VISIBLE | WS_CHILD,
            700, 0, 100, 20, hWnd, (HMENU)IDC_BTN_MOVE_LEFT, hInst, nullptr);
        CreateWindowW(L"BUTTON", L"Move Right", WS_VISIBLE | WS_CHILD,
            700, 20, 100, 20, hWnd, (HMENU)IDC_BTN_MOVE_RIGHT, hInst, nullptr);
        CreateWindowW(L"BUTTON", L"Move Up", WS_VISIBLE | WS_CHILD,
            700, 40, 100, 20, hWnd, (HMENU)IDC_BTN_MOVE_UP, hInst, nullptr);
        CreateWindowW(L"BUTTON", L"Move Down", WS_VISIBLE | WS_CHILD,
            700, 60, 100, 20, hWnd, (HMENU)IDC_BTN_MOVE_DOWN, hInst, nullptr);
        CreateWindowW(L"BUTTON", L"+ Width", WS_VISIBLE | WS_CHILD,
            700, 80, 100, 20, hWnd, (HMENU)IDC_BTN_INC_WIDTH, hInst, nullptr);
        CreateWindowW(L"BUTTON", L"- Width", WS_VISIBLE | WS_CHILD,
            700, 100, 100, 20, hWnd, (HMENU)IDC_BTN_DEC_WIDTH, hInst, nullptr);
        CreateWindowW(L"BUTTON", L"+ Height", WS_VISIBLE | WS_CHILD,
            700, 120, 100, 20, hWnd, (HMENU)IDC_BTN_INC_HEIGHT, hInst, nullptr);
        CreateWindowW(L"BUTTON", L"- Height", WS_VISIBLE | WS_CHILD,
            700, 140, 100, 20, hWnd, (HMENU)IDC_BTN_DEC_HEIGHT, hInst, nullptr);
        CreateWindowW(L"BUTTON", L"Ivalidate", WS_VISIBLE | WS_CHILD | WS_DISABLED,
            700, 160, 100, 20, hWnd, (HMENU)IDC_BTN_IVALIDATE, hInst, nullptr);
    }
    break;
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
        case IDC_BTN_MOVE_LEFT:
            MoveTargetWindow(-1, 0);
            break;
        case IDC_BTN_MOVE_RIGHT:
            MoveTargetWindow(1, 0);
            break;
        case IDC_BTN_MOVE_UP:
            MoveTargetWindow(0, -1);
            break;
        case IDC_BTN_MOVE_DOWN:
            MoveTargetWindow(0, 1);
            break;
        case IDC_BTN_INC_WIDTH:
            ScaleTargetWindow(1, 0);
            break;
        case IDC_BTN_DEC_WIDTH:
            ScaleTargetWindow(-1, 0);
            break;
        case IDC_BTN_INC_HEIGHT:
            ScaleTargetWindow(0, 1);
            break;
        case IDC_BTN_DEC_HEIGHT:
            ScaleTargetWindow(0, -1);
            break;
        case IDC_BTN_IVALIDATE:
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_LBUTTONDOWN:
    {
        //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rect;
        SetDCPenColor(hdc, RGB(0, 0, 0));
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
