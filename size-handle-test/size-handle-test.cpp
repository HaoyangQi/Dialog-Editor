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

HWND btnMain = NULL;
HWND staticMain, staticMainFocus, staticMainCur;
HWND focus = NULL;
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
    HDC hdc;

    // fill DPI scale factor
    hdc = GetDC(NULL);
    if (hdc) {
        pwd->dpiScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
        pwd->dpiScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
    }
    else {
        pwd->dpiScaleX = 1.0f;
        pwd->dpiScaleY = 1.0f;
    }
    ReleaseDC(NULL, hdc);

    pwd->hwndMain = NULL;
    pwd->hwndTarget = NULL;
    pwd->bVisible = TRUE;

    pwd->hdcHandleDisable = CreateCompatibleDC(NULL);
    pwd->hdcHandleEnable = CreateCompatibleDC(NULL);
    pwd->hdcMask = CreateCompatibleDC(NULL);

    pwd->bmpHandleEnable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HANDLE_ENABLE));
    pwd->bmpHandleDisable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HANDLE_DISABLE));
    pwd->curDefault = LoadCursor(NULL, IDC_ARROW);
    pwd->curMove = LoadCursor(hInst, MAKEINTRESOURCE(IDC_MOVE));
    pwd->curLR = LoadCursor(hInst, MAKEINTRESOURCE(IDC_RESIZE_H));
    pwd->curUD = LoadCursor(hInst, MAKEINTRESOURCE(IDC_RESIZE_V));
    pwd->curNE = LoadCursor(hInst, MAKEINTRESOURCE(IDC_RESIZE_NE));
    pwd->curSE = LoadCursor(hInst, MAKEINTRESOURCE(IDC_RESIZE_SE));
    pwd->curCurrent = pwd->curDefault;

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
    // destroy cursors
    DestroyCursor(pwd->curDefault);
    DestroyCursor(pwd->curMove);
    DestroyCursor(pwd->curLR);
    DestroyCursor(pwd->curUD);
    DestroyCursor(pwd->curNE);
    DestroyCursor(pwd->curSE);

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

BOOL DrawWindowHandles(WINDOW_DESIGNER* pwd, HWND target, int dd, LONG flagEnableHandles)
{
    BOOL ret = TRUE;
    RECT rcTarget;
    LONG width, height, step_x, step_y;
    HDC hdc;

    if (!target) {
        return FALSE;
    }

    /*if (!IsChild(parent, target)) {
        return FALSE;
    }*/

    GetWindowRect(target, &rcTarget);
    MapWindowPoints(HWND_DESKTOP, pwd->hwndMain, (LPPOINT)&rcTarget, 2);
    InflateRect(&rcTarget, dd, dd);

    width = rcTarget.right - rcTarget.left;
    height = rcTarget.bottom - rcTarget.top;
    step_x = (width - dd) / 2;
    step_y = (height - dd) / 2;

    hdc = GetDC(pwd->hwndMain);

    if (!hdc) {
        OutputDebugString(L"Error: DC not available\n");
        return FALSE;
    }

    for (int y = rcTarget.top; y <= rcTarget.bottom; y += step_y) {
        for (int x = rcTarget.left; x <= rcTarget.right; x += step_x) {
            // skip center
            if (x == rcTarget.left + step_x && y == rcTarget.top + step_y) {
                continue;
            }

            // TODO: fix x, y for a percise bound?

            ret &= DrawHandle(pwd, hdc, x, y, flagEnableHandles & 1);
            flagEnableHandles >>= 1;
        }
    }

    ReleaseDC(pwd->hwndMain, hdc);

    return ret;
}

BOOL IsSubRect(const RECT* target, const RECT* bound)
{
    return target->left >= bound->left &&
        target->top >= bound->top &&
        target->right <= bound->right &&
        target->bottom <= bound->bottom;
}

// point in base window coordinates
// -1 nowhere, 0 inside, otherwise a handle
LONG IsHoveringOnHandles(WINDOW_DESIGNER* pwd, POINT pt, int dd)
{
    RECT rcInflate, rcSquare, rcTmp;
    LONG handle = HANDLE_TOP_LEFT;// , config = ENABLE_ALL;
    LONG width, height, step_x, step_y;

    // TODO: rcTmp is only used to test, actually the focus rect
    GetWindowRect(focus, &rcTmp);
    MapWindowPoints(HWND_DESKTOP, pwd->hwndMain, (LPPOINT)&rcTmp, 2);

    if (PtInRect(&rcTmp, pt)) {
        return 0;
    }

    CopyRect(&rcInflate, &rcTmp);
    InflateRect(&rcInflate, 6, 6);

    width = rcInflate.right - rcInflate.left;
    height = rcInflate.bottom - rcInflate.top;
    step_x = (width - dd) / 2;
    step_y = (height - dd) / 2;

    wchar_t b1[100] = L"\0", b2[100] = L"\0";
    swprintf_s(b1, 100, L"(%d, %d) - (%d, %d)", rcInflate.left, rcInflate.top, rcInflate.right, rcInflate.bottom);
    swprintf_s(b2, 100, L"(%d, %d)", pt.x, pt.y);
    SetWindowText(staticMainFocus, b1);
    SetWindowText(staticMainCur, b2);

    /*if (target == pwd->hwndTarget) {
        config = ENABLE_RIGHTBOTTOM;
    }*/

    if (PtInRect(&rcInflate, pt)) {
        // TODO: in region of interest, iterate over 8 squares, skip center and disabled ones
        // OffsetRect
        for (int y = rcInflate.top; y <= rcInflate.bottom; y += step_y) {
            for (int x = rcInflate.left; x <= rcInflate.right; x += step_x) {
                // skip center
                if (x == rcInflate.left + step_x && y == rcInflate.top + step_y) {
                    continue;
                }

                // TODO: skip disabled handles?

                // handle is 6x6 by default
                SetRect(&rcSquare, x, y, x + dd, y + dd);
                if (PtInRect(&rcSquare, pt)) {
                    return handle;
                }

                handle <<= 1;
            }
        }
    }

    return -1;
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
    wcex.hCursor        = NULL;
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

    // debug-only settings
    LONG tmpMainWidth = 700 * designerData.dpiScaleX;
    LONG tmpMainHeight = 400 * designerData.dpiScaleY;
    LONG tmpTargetWidth = 300 * designerData.dpiScaleX;
    LONG tmpTargetHeight = 250 * designerData.dpiScaleY;
    LONG tmpControlWidth = 100 * designerData.dpiScaleX;
    LONG tmpControlHeight = 20 * designerData.dpiScaleY;
    // --------------------

	designerData.hwndMain = CreateWindowW(szWindowClass, szTitle, 
        WS_OVERLAPPEDWINDOW/*WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS*/,
		50, 50, 
        tmpMainWidth,
        tmpMainHeight,
        nullptr, nullptr, hInstance, nullptr);

	if (!designerData.hwndMain)
	{
		return FALSE;
	}

	ShowWindow(designerData.hwndMain, nCmdShow);

	designerData.hwndTarget = CreateWindowEx(0, szTargetLayerClass, szTargetTitle,
		WS_CAPTION | WS_CHILDWINDOW | WS_VISIBLE | WS_DISABLED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU,
		7, 7, 
        tmpTargetWidth,
        tmpTargetHeight,
        designerData.hwndMain, NULL, hInst, NULL);

	if (!designerData.hwndTarget)
	{
		return FALSE;
	}

    // set default margin box
    GetClientRect(designerData.hwndTarget, &designerData.rcMargin);
    InflateRect(&designerData.rcMargin, -7, -7);

    focus = designerData.hwndTarget;

    // child does not need to specify show state
    //ShowWindow(designerData.hwndTarget, nCmdShow);

    // display everything
    UpdateWindow(designerData.hwndMain);

    // debug test controls
    btnMain = CreateWindow(L"BUTTON", L"Test", WS_VISIBLE | WS_CHILD,
        tmpTargetWidth + 20, 10, tmpControlWidth, tmpControlHeight, 
        designerData.hwndMain, NULL, hInst, nullptr);
    staticMain = CreateWindow(L"STATIC", L"???", WS_VISIBLE | WS_CHILD,
        tmpTargetWidth + 20, tmpControlHeight + 10, tmpControlWidth, tmpControlHeight,
        designerData.hwndMain, NULL, hInst, nullptr);
    staticMainFocus = CreateWindow(L"STATIC", L"???", WS_VISIBLE | WS_CHILD,
        tmpTargetWidth + 20, tmpControlHeight * 2 + 10, 500, tmpControlHeight,
        designerData.hwndMain, NULL, hInst, nullptr);
    staticMainCur = CreateWindow(L"STATIC", L"???", WS_VISIBLE | WS_CHILD,
        tmpTargetWidth + 20, tmpControlHeight * 3 + 10, 500, tmpControlHeight,
        designerData.hwndMain, NULL, hInst, nullptr);
    CreateWindow(L"BUTTON", L"Test 1", WS_VISIBLE | WS_CHILD,
        7, 7, tmpControlWidth, tmpControlHeight, 
        designerData.hwndTarget, NULL, hInst, nullptr);
    CreateWindow(L"BUTTON", L"Test 2", WS_VISIBLE | WS_CHILD,
        7, tmpControlHeight + 7, tmpControlWidth, tmpControlHeight, 
        designerData.hwndTarget, NULL, hInst, nullptr);
    CreateWindow(L"BUTTON", L"Test 3", WS_VISIBLE | WS_CHILD,
        7, tmpControlHeight * 2 + 7, tmpControlWidth, tmpControlHeight,
        designerData.hwndTarget, NULL, hInst, nullptr);
    CreateWindow(L"EDIT", L"Test 4", WS_VISIBLE | WS_CHILD,
        20, tmpControlHeight * 4 + 7, tmpControlWidth, tmpControlHeight,
        designerData.hwndTarget, NULL, hInst, nullptr);

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
            SetCursor(designerData.curCurrent);
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

            // keep designerData.bVisible to TRUE for better transition update
            designerData.ptTrackStart.x = x;
            designerData.ptTrackStart.y = y;
            SetRect(&designerData.rcTrackPrev, x, y, x, y);

            // TODO: hit test routinue
            POINT pt;
            pt.x = x;
            pt.y = y;
            HWND hit = ChildWindowFromPoint(hWnd, pt);
            if (hit != NULL && hit != hWnd) {
                // if hitting target window, keep descending
                focus = hit;
                MapWindowPoints(hWnd, designerData.hwndTarget, &pt, 1);
                hit = ChildWindowFromPoint(designerData.hwndTarget, pt);
                if (hit != NULL && hit != designerData.hwndTarget) {
                    // TODO: if hit a control, set bounding box
                    // current test is just for single control ONLY
                    focus = hit;
                    GetWindowRect(hit, &designerData.rcSelectionBB);
                    MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&designerData.rcSelectionBB, 2);
                }
            }
            else {
                // reset focus
                focus = designerData.hwndTarget;
            }

            // immediate update 1 frame to show focus change (pre-drag)
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);

            break;
        }
        case WM_LBUTTONUP:
        {
            designerData.bVisible = TRUE;
            if (focus != designerData.hwndTarget) {
                MapWindowPoints(hWnd, designerData.hwndTarget, (LPPOINT)&designerData.rcSelectionBB, 2);
                MoveWindow(focus,
                    designerData.rcSelectionBB.left, designerData.rcSelectionBB.top,
                    designerData.rcSelectionBB.right - designerData.rcSelectionBB.left,
                    designerData.rcSelectionBB.bottom - designerData.rcSelectionBB.top,
                    FALSE);
            }
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        case WM_MOUSEMOVE:
        {
            LONG x = GET_X_LPARAM(lParam);
            LONG y = GET_Y_LPARAM(lParam);

            if (wParam & MK_LBUTTON) {
                HDC hdc;

                // check the guard: pre-drag: LB pressed
                if (designerData.bVisible) {
                    designerData.bVisible = FALSE;
                    // immediate refresh 1 frame to purge any existing handles
                    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
                }
                
                if (focus == designerData.hwndTarget) {
                    // if not hitting a control, show selection track rectangle
                    LONG trackW = abs(designerData.ptTrackStart.x - x);
                    LONG trackH = abs(designerData.ptTrackStart.y - y);

                    x = min(designerData.ptTrackStart.x, x);
                    y = min(designerData.ptTrackStart.y, y);

                    hdc = GetDC(hWnd);

                    // DrawFocusRect is XOR, re-apply on same rect twice will erase it (no effect)
                    DrawFocusRect(hdc, &designerData.rcTrackPrev);
                    SetRect(&designerData.rcTrackPrev, x, y, x + trackW, y + trackH);
                    DrawFocusRect(hdc, &designerData.rcTrackPrev);

                    ReleaseDC(hWnd, hdc);
                }
                else {
                    // if hitting a control, move selection bounding box
                    LONG dx = x - designerData.ptTrackStart.x;
                    LONG dy = y - designerData.ptTrackStart.y;

                    hdc = GetDC(hWnd);

                    // TODO: bounding test: add more data member to avoid API calls
                    /* Method 1: strict check */
                    /*DrawFocusRect(hdc, &designerData.rcSelectionBB); // clear previous
                    RECT rcTmpBound;
                    BOOL isInside;
                    CopyRect(&rcTmpBound, &designerData.rcMargin);
                    MapWindowPoints(designerData.hwndTarget, hWnd, (LPPOINT)&rcTmpBound, 2);
                    // start simulating
                    OffsetRect(&designerData.rcSelectionBB, dx, dy);
                    isInside = IsSubRect(&designerData.rcSelectionBB, &rcTmpBound);
                    if (!isInside) {
                        OffsetRect(&designerData.rcSelectionBB, -dx, -dy);
                        // cursor is moving too far, stop updating trackStart to wait for it
                        // to come back
                    }
                    else {
                        designerData.ptTrackStart.x = x;
                        designerData.ptTrackStart.y = y;
                    }
                    EnableWindow(btnMain, isInside); // test
                    DrawFocusRect(hdc, &designerData.rcSelectionBB);*/
                    /* Methos 2: Seperate check */
                    DrawFocusRect(hdc, &designerData.rcSelectionBB); // clear previous
                    RECT rcTmpBound;
                    BOOL isInsideX, isInsideY;
                    CopyRect(&rcTmpBound, &designerData.rcMargin);
                    MapWindowPoints(designerData.hwndTarget, hWnd, (LPPOINT)&rcTmpBound, 2);
                    // start simulating X
                    OffsetRect(&designerData.rcSelectionBB, dx, 0);
                    isInsideX = IsSubRect(&designerData.rcSelectionBB, &rcTmpBound);
                    if (!isInsideX) {
                        OffsetRect(&designerData.rcSelectionBB, -dx, 0);
                        // cursor is moving too far, stop updating trackStart to wait for it
                        // to come back
                    }
                    else {
                        designerData.ptTrackStart.x = x;
                    }
                    // start simulating Y
                    OffsetRect(&designerData.rcSelectionBB, 0, dy);
                    isInsideY = IsSubRect(&designerData.rcSelectionBB, &rcTmpBound);
                    if (!isInsideY) {
                        OffsetRect(&designerData.rcSelectionBB, 0, -dy);
                        // cursor is moving too far, stop updating trackStart to wait for it
                        // to come back
                    }
                    else {
                        designerData.ptTrackStart.y = y;
                    }
                    DrawFocusRect(hdc, &designerData.rcSelectionBB);
                    EnableWindow(btnMain, isInsideX || isInsideY); // test

                    ReleaseDC(hWnd, hdc);
                }
            }
            else {
                // simple moving

                // TODO: updating cursor should happen unconditionally, i.e. take off the "else"
                POINT cur;
                LONG hit_handle;
                HCURSOR curNext;

                cur.x = x;
                cur.y = y;
                hit_handle = IsHoveringOnHandles(&designerData, cur, 6);

                wchar_t buf[5] = L"\0";
                swprintf_s(buf, 5, L"%d", hit_handle);
                SetWindowText(staticMain, buf);

                switch (hit_handle) {
                    case 0:
                        curNext = focus == designerData.hwndTarget ? designerData.curDefault : designerData.curMove;
                        break;
                    case HANDLE_TOP_LEFT:
                    case HANDLE_BOTTOM_RIGHT:
                        curNext = designerData.curSE;
                        break;
                    case HANDLE_TOP_CENTER:
                    case HANDLE_BOTTOM_CENTER:
                        curNext = designerData.curUD;
                        break;
                    case HANDLE_TOP_RIGHT:
                    case HANDLE_BOTTOM_LEFT:
                        curNext = designerData.curNE;
                        break;
                    case HANDLE_MIDDLE_LEFT:
                    case HANDLE_MIDDLE_RIGHT:
                        curNext = designerData.curLR;
                        break;
                    default:
                        curNext = designerData.curDefault;
                        break;
                }

                if (curNext != designerData.curCurrent) {
                    SetCursor(curNext);
                    designerData.curCurrent = curNext;
                }
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
                ;
            }
            break;
        }
        case WM_PAINT:
        {
            //OutputDebugString(L"Parent Paint\n");
            PAINTSTRUCT ps;
            //HDC hdc;

            // focus test
            LONG enable = focus == designerData.hwndTarget ? ENABLE_RIGHTBOTTOM : ENABLE_ALL;
            
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);

            /*if (designerData.bVisible) {
                DrawWindowHandles(&designerData, hWnd, focus, 6, ENABLE_RIGHTBOTTOM);
            }*/

            // TODO: fall-through the re-paint: need optimize
            RedrawWindow(designerData.hwndTarget, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);

            if (designerData.bVisible) {
                DrawWindowHandles(&designerData, focus, 6, enable);
            }

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
    case WM_PAINT:
    {
        //OutputDebugString(L"Target Paint\n");
        PAINTSTRUCT ps;
        HDC hdc;
        HBRUSH brFrame;
        //RECT rect;

        hdc = BeginPaint(hWnd, &ps);
        brFrame = CreateSolidBrush(RGB(0, 0, 255));

        // TODO: margin rect test
        FrameRect(hdc, &designerData.rcMargin, brFrame);

        DeleteObject(brFrame);
        EndPaint(hWnd, &ps);
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
