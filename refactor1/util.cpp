#include "global.h"
#include "framework.h"
#include "designer.h"

void debugCheckError(int pos) {
    DWORD err = GetLastError();
    if (err != ERROR_SUCCESS) {
        wchar_t buf[100] = L"\0";
        swprintf_s(buf, 100, L"Error (at %d): %d\n", pos, err);
        OutputDebugString(buf);
        DebugBreak();
    }
}

BOOL IsSubRect(const RECT* target, const RECT* bound)
{
    return target->left >= bound->left &&
        target->top >= bound->top &&
        target->right <= bound->right &&
        target->bottom <= bound->bottom;
}

void MapWindowRectToDesigner(WINDOW_DESIGNER* pwd, RECT* lprect, HWND hwnd)
{
    GetWindowRect(hwnd, lprect);
    MapWindowPoints(HWND_DESKTOP, pwd->hwndMain, (LPPOINT)lprect, 2);
}

// point in base window coordinates
// -1 nowhere, 0 inside, otherwise a handle
LONG IsHoveringOnHandles(WINDOW_DESIGNER* pwd, POINT pt)
{
    // if CTRL pressed or selection has more than one control
    if (!pwd->bDetectHandleHover || isFocusSelection(pwd)) {
        return HANDLE_NOWHERE;
    }
    
    RECT rcInflate, rcSquare;
    LONG width, height, step_x, step_y;
    HWND focus = pwd->listSelection ? pwd->listSelection->item->hwnd : pwd->hwndTarget;
    LONG prop = isFocusTarget(pwd) ? LOCK_TOPLEFT : ENABLE_ALL;
    LONG handle = 1;

    MapWindowRectToDesigner(pwd, &rcInflate, focus);

    // Check if inside
    if (PtInRect(&rcInflate, pt)) {
        return HANDLE_INSIDE;
    }

    InflateRect(&rcInflate, pwd->dd, pwd->dd);
    width = rcInflate.right - rcInflate.left;
    height = rcInflate.bottom - rcInflate.top;
    step_x = (width - pwd->dd) / 2;
    step_y = (height - pwd->dd) / 2;

    if (PtInRect(&rcInflate, pt)) {
        for (int y = rcInflate.top; y <= rcInflate.bottom; y += step_y) {
            for (int x = rcInflate.left; x <= rcInflate.right; x += step_x) {
                // skip center
                if (x == rcInflate.left + step_x && y == rcInflate.top + step_y) {
                    continue;
                }

                // only process enabled handles
                if (prop & handle) {
                    // handle is 6x6 by default
                    SetRect(&rcSquare, x, y, x + pwd->dd, y + pwd->dd);
                    if (PtInRect(&rcSquare, pt)) {
                        return handle;
                    }
                }

                handle <<= 1;
            }
        }
    }

    return HANDLE_NOWHERE;
}
