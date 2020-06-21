#include "designer.h"

void InitWindowDesigner(WINDOW_DESIGNER* pwd, HINSTANCE hInst)
{
    BITMAP image;
    HDC hdc;

    pwd->appInstance = hInst;

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

    // literal initial values
    pwd->hwndMain = NULL;
    pwd->hwndTarget = NULL;
    pwd->bVisible = TRUE;
    pwd->dd = 6;
    pwd->bDetectHandleHover = TRUE;

    // additional procedural initialize
    pwd->hdcHandleDisable = CreateCompatibleDC(NULL);
    pwd->hdcHandleEnable = CreateCompatibleDC(NULL);
    pwd->hdcMask = CreateCompatibleDC(NULL);

    pwd->bmpHandleEnable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HANDLE_ENABLE));
    pwd->bmpHandleDisable = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HANDLE_DISABLE));
    pwd->curDefault = LoadCursor(NULL, IDC_ARROW);
    pwd->curMove = LoadCursor(hInst, MAKEINTRESOURCE(IDC_MOVE_CTL));
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
    pwd->bmpHandleDisable = (HBITMAP)SelectObject(pwd->hdcHandleDisable, pwd->bmpHandleDisable);
    pwd->bmpHandleEnable = (HBITMAP)SelectObject(pwd->hdcHandleEnable, pwd->bmpHandleEnable);
    pwd->bmpMask = (HBITMAP)SelectObject(pwd->hdcMask, pwd->bmpMask);

    // Additional design data
    pwd->listControls = NewDesignerControlList();
    pwd->listSelection = NULL;
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
    pwd->bmpHandleDisable = (HBITMAP)SelectObject(pwd->hdcHandleDisable, pwd->bmpHandleDisable);
    pwd->bmpHandleEnable = (HBITMAP)SelectObject(pwd->hdcHandleEnable, pwd->bmpHandleEnable);
    pwd->bmpMask = (HBITMAP)SelectObject(pwd->hdcMask, pwd->bmpMask);

    // release objects
    DeleteObject(pwd->bmpHandleDisable);
    DeleteObject(pwd->bmpHandleEnable);
    DeleteObject(pwd->bmpMask);

    // release DCs
    DeleteDC(pwd->hdcHandleDisable);
    DeleteDC(pwd->hdcHandleEnable);
    DeleteDC(pwd->hdcMask);

    // free design data
    ReleaseDesignerControlList(pwd->listControls);
    ClearDesignerSelectionList(pwd->listSelection);
}

void DrawHandle(WINDOW_DESIGNER* pwd, HDC hdc, int x, int y, BOOL bEnable)
{
    HDC hdcHandle = bEnable ? pwd->hdcHandleEnable : pwd->hdcHandleDisable;

    // When copying to mask, SetBkColor will set a color that cannot be represented in mask 
    // to be converted to the default background color, which is white
    // As for other color cannot be represented, they will be converted to forground color by default
    SetBkColor(hdcHandle, RGB(255, 0, 255));

    BitBlt(pwd->hdcMask, 0, 0, pwd->imgWidth, pwd->imgHeight, hdcHandle, 0, 0, SRCCOPY);
    BitBlt(hdc, x, y, pwd->imgWidth, pwd->imgHeight, hdcHandle, 0, 0, SRCINVERT);
    BitBlt(hdc, x, y, pwd->imgWidth, pwd->imgHeight, pwd->hdcMask, 0, 0, SRCAND);
    BitBlt(hdc, x, y, pwd->imgWidth, pwd->imgHeight, hdcHandle, 0, 0, SRCINVERT);
}

BOOL DrawControlHandles(WINDOW_DESIGNER* pwd, HDC hdc, HWND target, LONG flagEnableHandles)
{
    RECT rcTarget;
    LONG width, height, step_x, step_y;
    //HDC hdc;

    if (!target) {
        return FALSE;
    }

    GetWindowRect(target, &rcTarget);
    MapWindowPoints(HWND_DESKTOP, pwd->hwndMain, (LPPOINT)&rcTarget, 2);
    InflateRect(&rcTarget, pwd->dd, pwd->dd);

    width = rcTarget.right - rcTarget.left;
    height = rcTarget.bottom - rcTarget.top;
    step_x = (width - pwd->dd) / 2;
    step_y = (height - pwd->dd) / 2;

    //hdc = GetDC(pwd->hwndMain);

    if (!hdc) {
        return FALSE;
    }

    for (int y = rcTarget.top; y <= rcTarget.bottom; y += step_y) {
        for (int x = rcTarget.left; x <= rcTarget.right; x += step_x) {
            // skip center
            if (x == rcTarget.left + step_x && y == rcTarget.top + step_y) {
                continue;
            }

            DrawHandle(pwd, hdc, x, y, flagEnableHandles & 1);
            flagEnableHandles >>= 1;
        }
    }

    //ReleaseDC(pwd->hwndMain, hdc);
    return TRUE;
}

void DesignerUpdateMarginBox(WINDOW_DESIGNER* pwd)
{
    GetClientRect(pwd->hwndTarget, &pwd->rcMarginBox);
    
    pwd->rcMarginBox.left += pwd->rcMargin.left;
    pwd->rcMarginBox.top += pwd->rcMargin.top;
    pwd->rcMarginBox.right -= pwd->rcMargin.right;
    pwd->rcMarginBox.bottom -= pwd->rcMargin.bottom;

    MapWindowPoints(pwd->hwndTarget, pwd->hwndMain, (LPPOINT)&pwd->rcMarginBox, 2);
}

// when adding control, selection should be reset
void DesignerAddControl(WINDOW_DESIGNER* pwd, LPCWSTR className, LPCWSTR title, int x, int y, int w, int h)
{
    HWND hwnd = CreateWindowW(
        className, title, WS_VISIBLE | WS_CHILD, x, y, w, h, pwd->hwndTarget, NULL, pwd->appInstance, nullptr);

    if (!hwnd) {
        return;
    }

    DESIGNER_CONTROL_ITEM* item = AddDesignerControlItem(pwd->listControls, hwnd);
    //pwd->listSelection = ResetDesignerSelectionList(pwd->listControls, pwd->listSelection, hwnd);
}

void DesignerClearSelection(WINDOW_DESIGNER* pwd)
{
    ClearDesignerSelectionList(pwd->listSelection);
    pwd->listSelection = NULL;
}

void DesignerResetSelectionToFocus(WINDOW_DESIGNER* pwd, HWND hitTarget)
{
    pwd->listSelection = ResetDesignerSelectionList(pwd->listControls, pwd->listSelection, hitTarget);
}

void DesignerAddToSelection(WINDOW_DESIGNER* pwd, HWND hitTarget)
{
    pwd->listSelection = AccumulateSelection(pwd->listControls, pwd->listSelection, hitTarget);

    // update BB
    RECT rcTarget;
    MapWindowRectToDesigner(pwd, &rcTarget, hitTarget);
    pwd->rcSelectionBB.left = min(pwd->rcSelectionBB.left, rcTarget.left);
    pwd->rcSelectionBB.top = min(pwd->rcSelectionBB.top, rcTarget.top);
    pwd->rcSelectionBB.right = max(pwd->rcSelectionBB.right, rcTarget.right);
    pwd->rcSelectionBB.bottom = max(pwd->rcSelectionBB.bottom, rcTarget.bottom);
}
