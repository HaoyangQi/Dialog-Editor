#include "framework.h"
#include "sizebox.h"

static HINSTANCE appInstance;
static WCHAR szSizeBoxClass[MAX_LOADSTRING];
static HWND hwndMain, hwndTarget;
static HWND hStatic; // DEBUG ONLY
static size_t sizeControlPoint = 5;
static SIZE_BOX_DATA boxHandles[8];
static DRAG_HANDLE_POSITION positionType; // POS_XXX

ATOM RegisterSizeBoxClass(HINSTANCE hInstance);
LRESULT CALLBACK SizeBoxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL InitSizeBoxInstance(HINSTANCE hInstance, HWND hParent, int nCmdShow)
{
    appInstance = hInstance;

    LoadString(hInstance, IDC_SIZE_BOX, szSizeBoxClass, MAX_LOADSTRING);

    if (!RegisterSizeBoxClass(hInstance)) {
        return FALSE;
    }

    memset(boxHandles, 0, sizeof(SIZE_BOX_DATA) * 8);

    // handle set test
    boxHandles[POS_BOTTOM_CENTER_HANDLE].enable = TRUE;
    boxHandles[POS_MIDDLE_RIGHT_HANDLE].enable = TRUE;
    boxHandles[POS_TOP_LEFT_HANDLE].enable = TRUE;

    hwndMain = CreateWindowEx(WS_EX_TRANSPARENT, szSizeBoxClass, NULL,
        WS_CHILD,
        0, 0, 100, 100, hParent, NULL, appInstance, NULL);

    hwndTarget = NULL;
    //bBeginDrag = FALSE;

    if (!hwndMain) {
        return FALSE;
    }

    //SetLayeredWindowAttributes(hwndMain, RGB(255, 255, 255), 0, LWA_COLORKEY);
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    return TRUE;
}

ATOM RegisterSizeBoxClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = SizeBoxWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = GetStockBrush(WHITE_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szSizeBoxClass;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

LRESULT CALLBACK SizeBoxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        hStatic = CreateWindow(L"STATIC", L"?", WS_CHILD|WS_VISIBLE,
            10, 10, 70, 20, hWnd, nullptr, appInstance, nullptr);
        SetWindowText(hStatic, L"???");
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    case WM_NCHITTEST:
    {
        LRESULT hitResult = DefWindowProc(hWnd, WM_NCHITTEST, wParam, lParam);
        POINT ptCurrent;

        // fool the window to make it think a section is hit when actually hitting somewhere else
        if (hitResult == HTCLIENT) {
            positionType = POS_INSIDE;
            ptCurrent.x = GET_X_LPARAM(lParam);
            ptCurrent.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &ptCurrent);
            for (int i = 0; i < 8; i++) {
                if (PtInRect(&(boxHandles[i].handle), ptCurrent)) {
                    positionType = (DRAG_HANDLE_POSITION)i;
                }
            }

            switch (positionType) {
                case POS_TOP_LEFT_HANDLE:
                {
                    SetWindowText(hStatic, L"TL");
                    return HTTOPLEFT;
                }
                case POS_TOP_CENTER_HANDLE:
                {
                    SetWindowText(hStatic, L"TC");
                    return HTTOP;
                }
                case POS_TOP_RIGHT_HANDLE:
                {
                    SetWindowText(hStatic, L"TR");
                    return HTTOPRIGHT;
                }
                case POS_MIDDLE_LEFT_HANDLE:
                {
                    SetWindowText(hStatic, L"ML");
                    return HTLEFT;
                }
                case POS_MIDDLE_RIGHT_HANDLE:
                {
                    SetWindowText(hStatic, L"MR");
                    return HTRIGHT;
                }
                case POS_BOTTOM_LEFT_HANDLE:
                {
                    SetWindowText(hStatic, L"BL");
                    return HTBOTTOMLEFT;
                }
                case POS_BOTTOM_CENTER_HANDLE:
                {
                    SetWindowText(hStatic, L"BC");
                    return HTBOTTOM;
                }
                case POS_BOTTOM_RIGHT_HANDLE:
                {
                    SetWindowText(hStatic, L"BR");
                    return HTBOTTOMRIGHT;
                }
                default:
                {
                    SetWindowText(hStatic, L"INSIDE");
                    return HTCAPTION;
                }
            }
        }
        
        return hitResult;
    }
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
        GetClientRect(hWnd, &rect);
        int state = SaveDC(hdc);

        SelectObject(hdc, GetStockObject(DC_PEN));
        SelectObject(hdc, GetStockObject(DC_BRUSH));
        SetDCPenColor(hdc, RGB(0, 0, 0));
        SetDCBrushColor(hdc, RGB(255, 0, 0));
        
        long step_x = rect.right / 2;
        long step_y = rect.bottom / 2;
        int handle_left, handle_top;
        for (int y = 0, index = 0; y <= rect.bottom; y += step_y) {
            for (int x = 0; x <= rect.right; x += step_x) {
                // skip center
                if (x == step_x && y == step_y) {
                    continue;
                }

                handle_left = x - sizeControlPoint / 2;
                handle_top = y - sizeControlPoint / 2;

                // fix x, y
                if (handle_left < 0) {
                    handle_left = 0;
                }
                if (handle_top < 0) {
                    handle_top = 0;
                }
                if (rect.right - x < sizeControlPoint) {
                    handle_left = rect.right - sizeControlPoint;
                }
                if (rect.bottom - y < sizeControlPoint) {
                    handle_top = rect.bottom - sizeControlPoint;
                }

                if (!boxHandles[index].enable) {
                    SetDCBrushColor(hdc, RGB(0, 0, 0));
                }
                else {
                    SetDCBrushColor(hdc, RGB(255, 0, 0));
                }
                Rectangle(hdc, handle_left, handle_top,
                    handle_left + sizeControlPoint, handle_top + sizeControlPoint);

                // data set
                boxHandles[index].handle.left = handle_left;
                boxHandles[index].handle.top = handle_top;
                boxHandles[index].handle.right = handle_left + sizeControlPoint;
                boxHandles[index].handle.bottom = handle_top + sizeControlPoint;
                index++;
            }
        }

        RestoreDC(hdc, state);
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

void SizeBoxSetSnapTarget(HWND target)
{
    hwndTarget = target;
}

void SizeBoxSnap()
{
    if (hwndTarget) {
        ; // TODO: move window and resize it to match target's geometry
    }
}