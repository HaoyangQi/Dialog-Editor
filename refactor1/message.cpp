#include "message.h"
#include "designer.h"
#include "global.h"

// forward from refactor1.cpp
extern WCHAR szTargetLayerClass[MAX_LOADSTRING];
extern WCHAR szTargetTitle[MAX_LOADSTRING];

void OnMainCreate(WINDOW_DESIGNER* pwd, HWND hwnd)
{
    pwd->hwndMain = hwnd;
    SetCursor(pwd->curCurrent);

    pwd->hwndTarget = CreateWindowEx(0, szTargetLayerClass, szTargetTitle,
        WS_CAPTION | WS_CHILDWINDOW | WS_VISIBLE | WS_DISABLED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU,
        DEFAULT_TARGET_X,
        DEFAULT_TARGET_Y,
        DEFAULT_TARGET_WIDTH,
        DEFAULT_TARGET_HEIGHT,
        hwnd, NULL, pwd->appInstance, NULL);

    // Initialize margin box data
    SetRect(&pwd->rcMargin, 7, 7, 7, 7);
    DesignerUpdateMarginBox(pwd);

    // DEBUG ONLY
    DesignerAddControl(pwd, L"BUTTON", L"#", 7, 7, 50, 50);

    // Oh boy, magic calls:
    // Update target window accordingly (especially SWP_DRAWFRAME).
    // Without calling once during startup will create ugly dark corner around window frame.
    // This may contain huge overhead, but can set everything back on track, avoid it during heavy refresh.
    SetWindowPos(
        pwd->hwndTarget, 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void OnMainLButtonPress(WINDOW_DESIGNER* pwd, HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // every time there is a click, need to check CTRL key status
    // if CTRL pressed, accumulate selection, no focus, target window not selected
    // otherwise, reset selection to current hit

    // TODO: when accumulating selection, update BB

    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    HWND hitTarget;

    // keep designerData.bVisible to TRUE for better transition update
    pwd->ptTrackStart = pt;
    SetRect(&pwd->rcTrackPrev, pt.x, pt.y, pt.x, pt.y);
    MapWindowPoints(hWnd, pwd->hwndTarget, &pt, 1);
    hitTarget = ChildWindowFromPoint(pwd->hwndTarget, pt);

    // TODO: create control: the click has to be inside margin box, and overrides following if-stmt.

    if (hitTarget != NULL && hitTarget != pwd->hwndTarget) {
        // if hit a control
        // TODO: as for now, only single selection is supported, suppose to accumulate selection
        // check if the hit is in selection first, if so, do nothing, reset selection otherwise
        DebugPrintf(L"[FOCUS] handle focus: %x\n", (unsigned long long)hitTarget);
        DesignerResetSelectionToFocus(pwd, hitTarget);
        MapWindowRectToDesigner(pwd, &pwd->rcSelectionBB, hitTarget);
    }
    else {
        // anywhere else, reset focus
        DesignerClearSelection(pwd);
    }

    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
}

void OnMainMouseMove(WINDOW_DESIGNER* pwd, LONG x, LONG y)
{
    POINT cur = { x, y };
    LONG hit_handle;
    HCURSOR curNext;

    hit_handle = IsHoveringOnHandles(pwd, cur);
    switch (hit_handle) {
        case 0:
            curNext = isFocusTarget(pwd) ? pwd->curDefault : pwd->curMove;
            curNext = pwd->curDefault;
            break;
        case HANDLE_TOP_LEFT:
        case HANDLE_BOTTOM_RIGHT:
            curNext = pwd->curSE;
            break;
        case HANDLE_TOP_CENTER:
        case HANDLE_BOTTOM_CENTER:
            curNext = pwd->curUD;
            break;
        case HANDLE_TOP_RIGHT:
        case HANDLE_BOTTOM_LEFT:
            curNext = pwd->curNE;
            break;
        case HANDLE_MIDDLE_LEFT:
        case HANDLE_MIDDLE_RIGHT:
            curNext = pwd->curLR;
            break;
        default:
            curNext = pwd->curDefault;
            break;
    }

    if (curNext != pwd->curCurrent || curNext != GetCursor()) {
        SetCursor(curNext);
        pwd->curCurrent = curNext;
    }
}

void OnMainLButtonDrag(WINDOW_DESIGNER* pwd, HWND hWnd, LONG x, LONG y)
{
    HDC hdc;

    // check the guard: pre-drag: LB pressed
    if (pwd->bVisible) {
        pwd->bVisible = FALSE;
        // immediate refresh to purge any existing handles
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
    }

    // TODO: dragging rect optimization. When the track rect very close to margin box, there might remain
    // a very small gap when mouse leaves too fast, to avoid this, ever since mouse is out, snap the rect 
    // to the border, instead of stop tracking immediately.
    if (isFocusTarget(pwd)) {
        // if not hitting a control, show selection track rectangle
        // For the track rectangle, it can mean two things:
        // 1. a selection
        // 2. a control to be created

        LONG trackW = abs(pwd->ptTrackStart.x - x);
        LONG trackH = abs(pwd->ptTrackStart.y - y);

        x = min(pwd->ptTrackStart.x, x);
        y = min(pwd->ptTrackStart.y, y);

        hdc = GetDC(hWnd);

        // DrawFocusRect is XOR, re-apply on same rect again will erase it
        DrawFocusRect(hdc, &pwd->rcTrackPrev);
        SetRect(&pwd->rcTrackPrev, x, y, x + trackW, y + trackH);
        DrawFocusRect(hdc, &pwd->rcTrackPrev);

        ReleaseDC(hWnd, hdc);
    }
    else {
        ;// TODO: if a selection is still active when drag starts, then we are dragging it
    }
}

void OnMainLButtonRelease(WINDOW_DESIGNER* pwd, HWND hWnd)
{
    pwd->bVisible = TRUE;
    // TODO: move controls based on BB
    InvalidateRect(hWnd, NULL, TRUE);
}

/** 
 * Window Paint Logic
 * 
 * When paint happens, here goes in following order:
 * Erase base -> Paint base -> Erase target -> Paint target
 * Now here comes the assignment:
 *
 * Target window handles (if exists) will be handled in *Paint base*, since it is outside of the target region, 
 * so no clipping for handle drawings.
 *
 * Margin box will be handled in *Paint target*.
 *
 * Control handles (if exists) will be handled in *Paint target*, but after EndPaint, and in base client DC.
 *
 **/

void OnMainPaint(WINDOW_DESIGNER* pwd, HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    debugControlList(pwd->listControls);
    debugSelectionList(pwd->listSelection);

    if (pwd->bVisible && isFocusTarget(pwd)) {
        DrawControlHandles(pwd, hdc, pwd->hwndTarget, LOCK_TOPLEFT);
    }

    EndPaint(hWnd, &ps);
}

void OnTargetPaint(WINDOW_DESIGNER* pwd, HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HBRUSH brFrame;
    RECT rcMargin;
    DESIGNER_SELECTION_LIST* pl;

    // TODO: margin rect should look like a blue focus rect
    hdc = BeginPaint(hWnd, &ps);
    brFrame = CreateSolidBrush(RGB(0, 0, 255));
    CopyRect(&rcMargin, &pwd->rcMarginBox);
    MapWindowPoints(pwd->hwndMain, hWnd, (LPPOINT)&rcMargin, 2);
    FrameRect(hdc, &rcMargin, brFrame);
    DeleteObject(brFrame);
    EndPaint(hWnd, &ps);

    if (pwd->bVisible) {
        hdc = GetDC(pwd->hwndMain);
        pl = pwd->listSelection;
        if (pl) {
            DrawControlHandles(pwd, hdc, pl->item->hwnd, ENABLE_ALL);
            pl = pl->next;
        }
        while (pl) {
            DrawControlHandles(pwd, hdc, pl->item->hwnd, LOCK_ALL);
            pl = pl->next;
        }
        ReleaseDC(pwd->hwndMain, hdc);
    }
}