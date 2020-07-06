// ToolBoxPane.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ToolBoxPane.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

#define ARRAY_LENGTH(arr, type) (sizeof(arr) / sizeof(type))
TOOLBOX_PANE toolbox;
VIEW_ITEM* pvi;
const TOOLBOX_CONTENT arrContent[] = {
    { L"Pointer", IDB_CTL_POINTER },
    { L"Button", IDB_CTL_BTN },
    { L"Check Box", IDB_CTL_CHECKBOX },
    { L"Edit Control", IDB_CTL_EDIT },
    { L"Combo Box", IDB_CTL_COMBOBOX },
    { L"List Box", IDB_CTL_LISTBOX },
    { L"Group Box", ID_NO_RES },
    { L"Radio Button", IDB_CTL_RADIO },
    { L"Static Text", ID_NO_RES },
    { L"Picture Control", IDB_CTL_PICTURE },
    { L"Horizontal Scroll Bar", IDB_CTL_HSCROLL },
    { L"Vertical Scroll Bar", IDB_CTL_VSCROLL },
    { L"Slider Control", ID_NO_RES },
    { L"Spin Control", ID_NO_RES },
    { L"Progress Control", ID_NO_RES },
    { L"Hot Key", ID_NO_RES },
    { L"List View", ID_NO_RES },
    { L"Tree View", ID_NO_RES },
    { L"Tab Control", ID_NO_RES },
    { L"Animation", ID_NO_RES },
    { L"Rich Edit 2.0", ID_NO_RES },
    { L"Date Time Picker", ID_NO_RES },
    { L"Calendar Control", ID_NO_RES },
    { L"IP Address Control", ID_NO_RES },
    { L"Extended Combo Box", ID_NO_RES },
    { L"SysLink Control", ID_NO_RES },
    { L"Split Button Control", ID_NO_RES },
    { L"Network Address Control", ID_NO_RES },
    { L"Command Button", ID_NO_RES },
    { L"Custom Control", ID_NO_RES }
};

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
    LoadStringW(hInstance, IDC_TOOLBOXPANE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TOOLBOXPANE));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TOOLBOXPANE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TOOLBOXPANE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
      CW_USEDEFAULT, 0, 400, 700, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            InitToolBoxPane(&toolbox, hWnd, hInst);

            // test list
            /*pvi = ItemAppend(&toolbox, NULL, ITEM_NO_PARENT, 0, L"Root 0", NULL);
            ItemAppend(&toolbox, pvi, ITEM_NO_PARENT, 1, L"Root 1", NULL);
            ItemAppend(&toolbox, pvi, ITEM_NO_PARENT, 2, L"Root 2", NULL);
            ItemAppend(&toolbox, pvi, ITEM_NO_PARENT, 3, L"Root 3", NULL);
            ItemAppend(&toolbox, pvi, ITEM_NO_PARENT, 4, L"Root 4", NULL);
            ItemAppend(&toolbox, pvi, 0, 5, L"Item 1", MAKEINTRESOURCE(IDB_CTL_BTN));
            ItemAppend(&toolbox, pvi, 0, 6, L"Item 2", MAKEINTRESOURCE(IDB_CTL_CHECKBOX));
            ItemAppend(&toolbox, pvi, 1, 7, L"Item 3", MAKEINTRESOURCE(IDB_CTL_BTN));
            ItemAppend(&toolbox, pvi, 2, 8, L"Item 4", MAKEINTRESOURCE(IDB_CTL_BTN));
            ItemAppend(&toolbox, pvi, 3, 9, L"Item 5", MAKEINTRESOURCE(IDB_CTL_BTN));
            ItemAppend(&toolbox, pvi, 3, 10, L"Item 6", MAKEINTRESOURCE(IDB_CTL_BTN));
            ItemAppend(&toolbox, pvi, 5, 11, L"Item 7", MAKEINTRESOURCE(IDB_CTL_BTN));
            ItemAppend(&toolbox, pvi, 11, 12, L"Item 8", NULL);*/

            // tool box list
            pvi = ItemAppend(&toolbox, NULL, ITEM_NO_PARENT, 0, L"General", NULL);
            for (int i = 0; i < ARRAY_LENGTH(arrContent, TOOLBOX_CONTENT); i++) {
                ItemAppend(&toolbox, pvi, 0, i + 1, arrContent[i].name, MAKEINTRESOURCE(arrContent[i].idRes));
            }

            ToolboxBindViewData(&toolbox, pvi, pvi);

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
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_ERASEBKGND:
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect((HDC)wParam, &rc, toolbox.brBackground);
            return 1;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            OnPaint(&toolbox, hdc, &ps.rcPaint, ps.fErase);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_SIZE:
        {
            OnSize(&toolbox, LOWORD(lParam), HIWORD(lParam));
            break;
        }
        case WM_VSCROLL:
        {
            OnScroll(&toolbox, LOWORD(wParam), 5);
            break;
        }
        case WM_MOUSEMOVE:
        {
            OnMouseMove(&toolbox, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            break;
        }
        case WM_MOUSELEAVE:
        {
            OnMouseLeave(&toolbox);
            break;
        }
        case WM_MOUSEHOVER:
        {
            toolbox.bMouseTrack = FALSE;
            break;
        }
        case WM_LBUTTONDOWN:
        {
            OnMouseLeftButtonPress(&toolbox, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            break;
        }
        case WM_DESTROY:
            ReleaseToolBoxPane(&toolbox);
            ReleaseItemList(pvi);
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
