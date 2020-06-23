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

    if (isScaleFocus(pwd)) {
        pwd->typeTrack = TRACK_SCALE;
        MapWindowRectToDesigner(pwd, &pwd->rcTrackPrev, 
            pwd->listSelection ? pwd->listSelection->item->hwnd : pwd->hwndTarget);
        pwd->ptTrackStart.x = pwd->rcTrackPrev.left;
        pwd->ptTrackStart.y = pwd->rcTrackPrev.top;
    }
    else if (hitTarget != NULL && hitTarget != pwd->hwndTarget) {
        // if hit a control
        // TODO: as for now, only single selection is supported, suppose to accumulate selection
        // check if the hit is in selection first, if so, do nothing, reset selection otherwise
        pwd->typeTrack = TRACK_MOVE;
        DesignerResetSelectionToFocus(pwd, hitTarget);
        MapWindowRectToDesigner(pwd, &pwd->rcSelectionBB, hitTarget);
        CopyRect(&pwd->rcPreDragBB, &pwd->rcSelectionBB);
    }
    else {
        // anywhere else, reset focus
        pwd->typeTrack = TRACK_SELECTION;
        DesignerClearSelection(pwd);
    }

    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
}

void OnMainMouseMove(WINDOW_DESIGNER* pwd, WPARAM wParam, LONG x, LONG y)
{
    POINT cur = { x, y };
    HCURSOR curNext;

    if (wParam & MK_LBUTTON) {
        curNext = pwd->curCurrent;
    }
    else {
        pwd->lHandle = IsHoveringOnHandles(pwd, cur);
        switch (pwd->lHandle) {
            case HANDLE_INSIDE:
                curNext = isFocusTarget(pwd) ? pwd->curDefault : pwd->curMove;
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
    }

    if (curNext != pwd->curCurrent || curNext != GetCursor()) {
        SetCursor(curNext);
        pwd->curCurrent = curNext;
    }
}

void OnMainLButtonDrag(WINDOW_DESIGNER* pwd, HWND hWnd, LONG x, LONG y)
{
    HDC hdc = GetDC(hWnd);
    
    // check the guard: pre-drag: LB pressed
    if (pwd->bVisible) {
        pwd->bVisible = FALSE;
        // immediate refresh to purge any existing handles
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);

        if (pwd->typeTrack == TRACK_SCALE) {
            DrawFocusRect(hdc, &pwd->rcTrackPrev);
        }
    }

    //HDC hdc = GetDC(hWnd);

    if (pwd->typeTrack == TRACK_SELECTION) {
        // if not hitting a control or any valid handle, show selection track rectangle
        LONG trackW = abs(pwd->ptTrackStart.x - x);
        LONG trackH = abs(pwd->ptTrackStart.y - y);

        x = min(pwd->ptTrackStart.x, x);
        y = min(pwd->ptTrackStart.y, y);

        // DrawFocusRect is XOR, re-apply on same rect again will erase it
        DrawFocusRect(hdc, &pwd->rcTrackPrev);
        SetRect(&pwd->rcTrackPrev, x, y, x + trackW, y + trackH);
        DrawFocusRect(hdc, &pwd->rcTrackPrev);
    }
    else if (pwd->typeTrack == TRACK_MOVE) {
        // if a selection is still active when drag starts, then we are dragging it
        LONG dx = x - pwd->ptTrackStart.x;
        LONG dy = y - pwd->ptTrackStart.y;

        // TODO: another way to draw is to draw one BB for each control
        // currently is the BB of whole selection

        // clear previous
        DrawFocusRect(hdc, &pwd->rcSelectionBB);
        // if any coordinate is out, snap it to nearest margin border
        // start simulating X
        OffsetRect(&pwd->rcSelectionBB, dx, 0);
        if (!IsSubRect(&pwd->rcSelectionBB, &pwd->rcMarginBox)) {
            dx = pwd->rcMarginBox.left - pwd->rcSelectionBB.left;
            dx = dx >= 0 ? dx : pwd->rcMarginBox.right - pwd->rcSelectionBB.right;
            OffsetRect(&pwd->rcSelectionBB, dx, 0);
        }
        else {
            pwd->ptTrackStart.x = x;
        }
        // start simulating Y
        OffsetRect(&pwd->rcSelectionBB, 0, dy);
        if (!IsSubRect(&pwd->rcSelectionBB, &pwd->rcMarginBox)) {
            dy = pwd->rcMarginBox.top - pwd->rcSelectionBB.top;
            dy = dy >= 0 ? dy : pwd->rcMarginBox.bottom - pwd->rcSelectionBB.bottom;
            OffsetRect(&pwd->rcSelectionBB, 0, dy);
        }
        else {
            pwd->ptTrackStart.y = y;
        }
        DrawFocusRect(hdc, &pwd->rcSelectionBB);
    }
    else if (pwd->typeTrack == TRACK_SCALE) {
        // TODO: scale based on handles, margin box, only scale along corresponding direction
        LONG trackW = 0, trackH = 0;

        switch (pwd->lHandle) {
            case HANDLE_TOP_LEFT:
                trackW = max(pwd->rcTrackPrev.right - x, 0);
                trackH = max(pwd->rcTrackPrev.bottom - y, 0);
                x = min(pwd->rcTrackPrev.right, x);
                y = min(pwd->rcTrackPrev.bottom, y);
                break;
            case HANDLE_BOTTOM_RIGHT:
                trackW = max(x - pwd->ptTrackStart.x, 0);
                trackH = max(y - pwd->ptTrackStart.y, 0);
                x = pwd->ptTrackStart.x;
                y = pwd->ptTrackStart.y;
                break;
            case HANDLE_TOP_CENTER:
                trackW = pwd->rcTrackPrev.right - pwd->rcTrackPrev.left;
                trackH = max(pwd->rcTrackPrev.bottom - y, 0);
                x = pwd->rcTrackPrev.left;
                y = min(pwd->rcTrackPrev.bottom, y);
                break;
            case HANDLE_BOTTOM_CENTER:
                trackW = pwd->rcTrackPrev.right - pwd->rcTrackPrev.left;
                trackH = max(y - pwd->ptTrackStart.y, 0);
                x = pwd->ptTrackStart.x;
                y = pwd->ptTrackStart.y;
                break;
            case HANDLE_TOP_RIGHT:
                trackW = max(x - pwd->ptTrackStart.x, 0);
                trackH = max(pwd->rcTrackPrev.bottom - y, 0);
                x = pwd->ptTrackStart.x;
                y = min(pwd->rcTrackPrev.bottom, y);
                break;
            case HANDLE_BOTTOM_LEFT:
                trackW = max(pwd->rcTrackPrev.right - x, 0);
                trackH = max(y - pwd->ptTrackStart.y, 0);
                x = min(pwd->rcTrackPrev.right, x);
                y = pwd->ptTrackStart.y;
                break;
            case HANDLE_MIDDLE_LEFT:
                trackW = max(pwd->rcTrackPrev.right - x, 0);
                trackH = pwd->rcTrackPrev.bottom - pwd->rcTrackPrev.top;
                x = min(pwd->rcTrackPrev.right, x);
                y = pwd->ptTrackStart.y;
                break;
            case HANDLE_MIDDLE_RIGHT:
                trackW = max(x - pwd->ptTrackStart.x, 0);
                trackH = pwd->rcTrackPrev.bottom - pwd->rcTrackPrev.top;
                x = pwd->ptTrackStart.x;
                y = pwd->ptTrackStart.y;
                break;
            default:
                // should never reach here
                break;
        }

        // DrawFocusRect is XOR, re-apply on same rect again will erase it
        DrawFocusRect(hdc, &pwd->rcTrackPrev);
        SetRect(&pwd->rcTrackPrev, x, y, x + trackW, y + trackH);
        DrawFocusRect(hdc, &pwd->rcTrackPrev);
    }

    ReleaseDC(hWnd, hdc);
}

void OnMainLButtonRelease(WINDOW_DESIGNER* pwd, HWND hWnd)
{
    DESIGNER_SELECTION_LIST* l = pwd->listSelection;

    if (pwd->typeTrack == TRACK_MOVE) {
        LONG dx = pwd->rcSelectionBB.left - pwd->rcPreDragBB.left;
        LONG dy = pwd->rcSelectionBB.top - pwd->rcPreDragBB.top;
        RECT rcWindow;
        while (l) {
            GetWindowRect(l->item->hwnd, &rcWindow);
            MapWindowPoints(HWND_DESKTOP, pwd->hwndTarget, (LPPOINT)&rcWindow, 2);
            MoveWindow(l->item->hwnd, rcWindow.left + dx, rcWindow.top + dy,
                rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, FALSE);
            l = l->next;
        }
    }
    else if (pwd->typeTrack == TRACK_SCALE) {
        if (isFocusTarget(pwd)) {
            MoveWindow(pwd->hwndTarget, pwd->rcTrackPrev.left, pwd->rcTrackPrev.top,
                pwd->rcTrackPrev.right - pwd->rcTrackPrev.left, pwd->rcTrackPrev.bottom - pwd->rcTrackPrev.top, FALSE);
            DesignerUpdateMarginBox(pwd);
        }
        else {
            // it is safe to use *else* here because a selection cannot be scaled
            MapWindowPoints(pwd->hwndMain, pwd->hwndTarget, (LPPOINT)&pwd->rcTrackPrev, 2);
            MoveWindow(l->item->hwnd, pwd->rcTrackPrev.left, pwd->rcTrackPrev.top,
                pwd->rcTrackPrev.right - pwd->rcTrackPrev.left, pwd->rcTrackPrev.bottom - pwd->rcTrackPrev.top, FALSE);
        }
    }

    pwd->typeTrack = NO_TRACK;
    pwd->bVisible = TRUE;
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_ERASE);
    RedrawWindow(pwd->hwndTarget, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
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

    //debugControlList(pwd->listControls);
    //debugSelectionList(pwd->listSelection);

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
