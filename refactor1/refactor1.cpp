#include "framework.h"
#include "refactor1.h"
#include "proc.h"
#include "designer.h"

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
WCHAR szTargetLayerClass[MAX_LOADSTRING] = L"TargetWindowTest";
WCHAR szTargetTitle[MAX_LOADSTRING] = L"Target Test Window";
WINDOW_DESIGNER designerData;

ATOM RegisterWindowDesignerClass(WINDOW_DESIGNER*, HINSTANCE);
BOOL InitInstance(WINDOW_DESIGNER*, HINSTANCE, int);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_REFACTOR1, szWindowClass, MAX_LOADSTRING);

    // Initialize control data
    // TODO: if pack the control, init should go to main WM_CREATE
    InitWindowDesigner(&designerData, hInstance);
    
    // Register class
    if (!RegisterWindowDesignerClass(&designerData, hInstance))
    {
        return FALSE;
    }

    // Fire up the window
    if (!InitInstance (&designerData, hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REFACTOR1));

    MSG msg;
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

ATOM RegisterWindowDesignerClass(WINDOW_DESIGNER* pwd, HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0; // TODO: set this and allocate data structure in WM_CREATE
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REFACTOR1));
    wcex.hCursor = NULL;// pwd->curDefault;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_REFACTOR1);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    if (!RegisterClassExW(&wcex)) {
        return 0;
    }

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = TargetProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = pwd->curDefault;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szTargetLayerClass;
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

// main startup code for this control
// TODO: if pack the control, this function is not necessary
BOOL InitInstance(WINDOW_DESIGNER* pwd, HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 700 * pwd->dpiScaleX, 400 * pwd->dpiScaleY, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
