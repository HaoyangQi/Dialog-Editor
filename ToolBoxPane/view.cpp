#include "ToolBoxPane.h"

void InitToolBoxPane(TOOLBOX_PANE* ptp, HWND hwnd, HINSTANCE hInst)
{
    NONCLIENTMETRICSW ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICSW);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);

    InitCommonControls();

    HDC hdc;
    HBITMAP bmp;
    RECT rc;
    hdc = GetDC(hwnd);
    GetClientRect(hwnd, &rc);

    ptp->appInstance = hInst;
    ptp->hwnd = hwnd;
    ptp->fontRoot = CreateFontIndirectW(&ncm.lfMenuFont);
    ptp->fontChild = CreateFontIndirectW(&ncm.lfMenuFont);
    ptp->fontOld = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
    ptp->brBackground = CreateSolidBrush(RGB(37, 37, 38));
    ptp->bToggleList = FALSE;
    ptp->szPane.cx = rc.right - rc.left;
    ptp->szPane.cy = rc.bottom - rc.top;
    ptp->dmItemHeight = 28;
    ptp->dmItemImageMargin = 2;
    ptp->dmItemTextMargin = 20;
    ptp->dmLeftMargin = 5;
    ptp->clrForeground = RGB(241, 241, 241);
    ptp->clrHover = RGB(62, 62, 64);
    ptp->clrSelect = RGB(0, 122, 204);
    ptp->si.cbSize = sizeof(SCROLLINFO);
    ptp->si.fMask = SIF_ALL;
    ptp->si.nMin = 0;
    ptp->si.nMax = 0;
    ptp->si.nPage = ptp->szPane.cy;
    ptp->si.nPos = 0;
    ptp->si.nTrackPos = 0;
    ptp->tme.cbSize = sizeof(TRACKMOUSEEVENT);
    ptp->tme.hwndTrack = hwnd;
    ptp->tme.dwFlags = TME_HOVER | TME_LEAVE;
    ptp->tme.dwHoverTime = HOVER_DEFAULT;
    ptp->pvi = NULL;

    ptp->hItemImageList = ImageList_Create(measureImageWidth(ptp), measureImageWidth(ptp), ILC_COLOR | ILC_MASK, 0, 0);
    ptp->hStateImageList = ImageList_Create(ptp->dmItemHeight, ptp->dmItemHeight, ILC_COLOR | ILC_MASK, 0, 0);
    // load collapse state image
    bmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_STATE_COLLAPSE));
    ToolBoxAddImage(ptp->hStateImageList, bmp, RGB(255, 0, 255));
    DeleteObject(bmp);
    // load expand state icon
    bmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_STATE_EXPAND));
    ToolBoxAddImage(ptp->hStateImageList, bmp, RGB(255, 0, 255));
    DeleteObject(bmp);

    ReleaseDC(hwnd, hdc);
    SetScrollInfo(hwnd, SB_VERT, &ptp->si, TRUE);
}

void ReleaseToolBoxPane(TOOLBOX_PANE* ptp)
{
    // we do not touch item list here as this control only views the data
    HDC hdc = GetDC(ptp->hwnd);
    SelectObject(hdc, ptp->fontOld);
    ReleaseDC(ptp->hwnd, hdc);

    DeleteObject(ptp->fontRoot);
    DeleteObject(ptp->fontChild);
    ImageList_Destroy(ptp->hItemImageList);
    ImageList_Destroy(ptp->hStateImageList);
    DeleteObject(ptp->brBackground);
}

int ToolBoxAddImage(HIMAGELIST hImgList, HBITMAP bmp, COLORREF clrTransparent)
{
    if (!hImgList || !bmp) {
        return -1;
    }
    
    HDC hdc, hdcImg, hdcMem;
    HBITMAP bmpMem;
    BITMAP image;
    int w, h, idx;

    ImageList_GetIconSize(hImgList, &w, &h);
    hdc = GetDC(NULL);
    bmpMem = CreateCompatibleBitmap(hdc, w, h);
    hdcImg = CreateCompatibleDC(hdc);
    hdcMem = CreateCompatibleDC(hdc);

    // stretch
    GetObject(bmp, sizeof(BITMAP), &image);
    bmp = (HBITMAP)SelectObject(hdcImg, bmp);
    bmpMem = (HBITMAP)SelectObject(hdcMem, bmpMem);
    StretchBlt(hdcMem, 0, 0, w, h, hdcImg, 0, 0, image.bmWidth, image.bmHeight, SRCCOPY);
    bmp = (HBITMAP)SelectObject(hdcImg, bmp);
    bmpMem = (HBITMAP)SelectObject(hdcMem, bmpMem);

    idx = ImageList_AddMasked(hImgList, bmpMem, clrTransparent);

    DeleteObject(bmpMem);
    DeleteDC(hdcImg);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdc);

    return idx;
}

void ItemUpdateTextOffset(TOOLBOX_PANE* ptp, VIEW_ITEM* pvi)
{
    // if item is a level 0 empty root, only append a small margin on the left
    // otherwise: text start = level indention + (state image width) + (item image width + text-image margin)

    pvi->rcItemText.left = !(isChild(pvi) || isCategory(pvi)) ? ptp->dmLeftMargin : 0;
    pvi->rcItemText.left += 
        pvi->nLevel * ptp->dmItemHeight + 
        (isCategory(pvi) ? ptp->dmItemHeight : 0) +
        (hasItemImage(pvi) ? ptp->dmItemHeight + ptp->dmItemTextMargin : 0);
}

VIEW_ITEM* ItemAppend(TOOLBOX_PANE* ptp, VIEW_ITEM* pvi, LONG idParent, LONG id, LPCWSTR strTitle, LPCWSTR strImage)
{
    VIEW_ITEM* pItem = (VIEW_ITEM*)malloc(sizeof(VIEW_ITEM));
    if (!pItem) {
        return NULL;
    }

    memset(pItem, 0, sizeof(VIEW_ITEM));
    pItem->id = id;
    pItem->nImageIndex = -1;
    SetRect(&pItem->rcItemText, 0, 0, ptp->szPane.cx, ptp->dmItemHeight);
    // title
    if (strTitle) {
        LONG szStr = (wcslen(strTitle) + 1) * sizeof(WCHAR);
        pItem->strContent = (WCHAR*)malloc(szStr);
        
        if (!pItem->strContent) {
            free(pItem);
            return NULL;
        }

        memcpy(pItem->strContent, strTitle, szStr);
    }
    // image
    if (strImage) {
        HBITMAP bmp = LoadBitmapW(ptp->appInstance, strImage);
        pItem->nImageIndex = ToolBoxAddImage(ptp->hItemImageList, bmp, RGB(255, 0, 255));
        DeleteObject(bmp);
    }

    // update x pos once based on current state
    ItemUpdateTextOffset(ptp, pItem);
    
    // if a list is specified, we need to insert pItem to correct location
    if (pvi) {
        if (idParent == ITEM_NO_PARENT) {
            // a root item, just append at the very end
            while (pvi->next) {
                pvi = pvi->next;
            }
            pvi->next = pItem;
            OffsetRect(&pItem->rcItemText, 0, pvi->rcItemText.bottom);
        }
        else {
            // insert under an item as a child
            LONG levelParent;
            VIEW_ITEM* pInsertBefore;
            // find the parent
            while (pvi && pvi->id != idParent) {
                pvi = pvi->next;
            }
            if (!pvi) {
                free(pItem->strContent);
                free(pItem);
                return NULL;
            }
            // if found, find the location of appending
            pItem->parent = pvi;
            levelParent = pvi->nLevel;
            pInsertBefore = pvi->next;
            while (pInsertBefore && pInsertBefore->nLevel > levelParent) {
                pvi = pInsertBefore;
                pInsertBefore = pInsertBefore->next;
            }
            // insert
            pItem->nLevel = levelParent + 1;
            pItem->next = pInsertBefore;
            pvi->next = pItem;
            // update item y pos, and all items below insertion need to be offset by one item height
            OffsetRect(&pItem->rcItemText, 0, pvi->rcItemText.bottom);
            while (pInsertBefore) {
                OffsetRect(&pInsertBefore->rcItemText, 0, ptp->dmItemHeight);
                pInsertBefore = pInsertBefore->next;
            }
            // update item x pos: margin + indention + state image width + image width
            // for now, image BB is always square
            // need to do it also for its parent, ORDER MATTERS
            ItemUpdateTextOffset(ptp, pvi);
            ItemUpdateTextOffset(ptp, pItem);
        }
    }

    return pItem;
}

void ReleaseItemList(VIEW_ITEM* pvi)
{
    VIEW_ITEM* pTmp;
    while (pvi) {
        pTmp = pvi->next;
        free(pvi->strContent);
        free(pvi);
        pvi = pTmp;
    }
}

VIEW_ITEM* ItemGetNext(VIEW_ITEM* current)
{
    if (!current) {
        return NULL;
    }

    // get next visible items (i.e. skip collapsed ones)
    VIEW_ITEM* pvi = current->next;

    if (current->bCollapse) {
        LONG level = current->nLevel;
        while (pvi && pvi->nLevel > level) {
            pvi = pvi->next;
        }
    }
    
    return pvi;
}

void ItemDrawImage(TOOLBOX_PANE* ptp, VIEW_ITEM* pvi, HDC hdc)
{
    LONG x;

    // state image
    if (isCategory(pvi)) {
        int nSkip = (pvi->nImageIndex == -1) ? 1 : 2;
        int index = pvi->bCollapse ? 0 : 1;
        x = pvi->rcItemText.left - ptp->dmItemHeight * nSkip - getTextMargin(pvi);
        ImageList_Draw(ptp->hStateImageList, index, hdc, x, pvi->rcItemText.top, ILD_TRANSPARENT);
    }

    // item image
    if (pvi->nImageIndex != -1) {
        x = pvi->rcItemText.left - ptp->dmItemHeight - getTextMargin(pvi) + ptp->dmItemImageMargin;
        ImageList_Draw(ptp->hItemImageList, pvi->nImageIndex, hdc,
            x, pvi->rcItemText.top + ptp->dmItemImageMargin, ILD_TRANSPARENT);
    }
}

void ToolboxUpdateScrollRange(TOOLBOX_PANE* ptp)
{
    ptp->si.fMask = SIF_RANGE;
    ptp->si.nMin = 0;
    ptp->si.nMax = 0;

    VIEW_ITEM* pvi = ptp->pvi;
    while (pvi) {
        ptp->si.nMax += ptp->dmItemHeight;
        pvi = ItemGetNext(pvi);
    }
}

VIEW_ITEM* ToolboxBindViewData(TOOLBOX_PANE* ptp, VIEW_ITEM* pvi, VIEW_ITEM* selectionDefault)
{
    VIEW_ITEM* pviOld = ptp->pvi;
    VIEW_ITEM* pTemp = pvi;
    
    // reset real-time info
    ptp->pvi = pvi;
    ptp->itemHover = NULL;
    ptp->itemSelect = NULL;
    while (pTemp) {
        pTemp->bHover = FALSE;
        pTemp = pTemp->next;
    }

    // measure scroll
    pTemp = pvi;
    if (pTemp) {
        ptp->si.fMask = SIF_ALL;
        ptp->si.nMin = 0;
        ptp->si.nMax = 0;
        ptp->si.nPage = ptp->szPane.cy;
        ptp->si.nPos = 0;

        while (pTemp) {
            ptp->si.nMax += ptp->dmItemHeight;
            pTemp = ItemGetNext(pTemp);
        }

        SetScrollInfo(ptp->hwnd, SB_VERT, &ptp->si, TRUE);
    }

    // set default selection
    pTemp = pvi;
    if (selectionDefault) {
        // just to be safe, check if the item is valid
        // also reset selection status
        BOOL bValid = FALSE;
        while (pTemp) {
            pTemp->bSelect = FALSE;
            bValid = bValid | (pTemp == selectionDefault);
            pTemp = pTemp->next;
        }
        if (bValid) {
            selectionDefault->bSelect = TRUE;
            ptp->itemSelect = selectionDefault;
        }
    }
    else {
        // restore old selection
        while (pTemp) {
            if (pTemp->bSelect) {
                ptp->itemSelect = pTemp;
                break;
            }
            pTemp = pTemp->next;
        }
    }

    InvalidateRect(ptp->hwnd, NULL, TRUE);
    return pviOld;
}

void OnSize(TOOLBOX_PANE* ptp, LONG w, LONG h)
{
    // update scroll bar
    if (h != ptp->szPane.cy) {
        ptp->si.fMask = SIF_PAGE;
        ptp->si.nPage = h;
        SetScrollInfo(ptp->hwnd, SB_VERT, &ptp->si, TRUE);
    }

    RECT rcClient;
    GetClientRect(ptp->hwnd, &rcClient);
    w = rcClient.right - rcClient.left;
    h = rcClient.bottom - rcClient.top;

    // update item BB
    if (w != ptp->szPane.cx) {
        VIEW_ITEM* pvi = ptp->pvi;
        while (pvi) {
            pvi->rcItemText.right = w;
            pvi = pvi->next;
        }
    }

    ptp->szPane.cx = w;
    ptp->szPane.cy = h;

    if (ptp->pvi)
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        GetScrollInfo(ptp->hwnd, SB_VERT, &si);

        LONG dy = abs(ptp->pvi->rcItemText.top) - si.nPos;
        VIEW_ITEM* prop = ptp->pvi;

        while (prop)
        {
            OffsetRect(&prop->rcItemText, 0, dy);
            prop = prop->next;
        }

        ScrollWindowEx(ptp->hwnd, 0, dy, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE);
    }
}

void OnScroll(TOOLBOX_PANE* ptp, int scrollType, int lineStep)
{
    LONG posOld = 0;

    ptp->si.fMask = SIF_ALL;
    GetScrollInfo(ptp->hwnd, SB_VERT, &ptp->si);
    posOld = ptp->si.nPos;

    switch (scrollType) {
        case SB_BOTTOM:
            ptp->si.nPos = ptp->si.nMax;
            break;
        case SB_LINEDOWN:
            ptp->si.nPos = posOld + lineStep;
            break;
        case SB_LINEUP:
            ptp->si.nPos = posOld - lineStep;
            break;
        case SB_PAGEDOWN:
            ptp->si.nPos = posOld + ptp->si.nPage;
            break;
        case SB_PAGEUP:
            ptp->si.nPos = posOld - ptp->si.nPage;
            break;
        case SB_THUMBTRACK:
            ptp->si.nPos = ptp->si.nTrackPos;
            break;
        case SB_TOP:
            ptp->si.nPos = ptp->si.nMin;
            break;
        case SB_THUMBPOSITION:
        default:
            //ptp->si.nPos = ptp->si.nPos;
            break;
    }

    // update
    ptp->si.fMask = SIF_POS;
    SetScrollInfo(ptp->hwnd, SB_VERT, &ptp->si, TRUE);
    GetScrollInfo(ptp->hwnd, SB_VERT, &ptp->si); // make sure new value is valid within range

    LONG dy = posOld - ptp->si.nPos;
    VIEW_ITEM* pvi = ptp->pvi;
    while (pvi) {
        // this loop might be very slow when data set is large, consider optimizing
        OffsetRect(&pvi->rcItemText, 0, dy);
        pvi = pvi->next;
    }

    ScrollWindowEx(ptp->hwnd, 0, dy, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE);
}

void OnPaint(TOOLBOX_PANE* ptp, HDC hdc, LPRECT rcUpdate, BOOL bErase)
{
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, ptp->clrForeground);

    VIEW_ITEM* pCur = ptp->pvi;
    while (pCur) {
        if (pCur->rcItemText.top <= rcUpdate->bottom && pCur->rcItemText.bottom >= rcUpdate->top) {
            // choose font
            if (pCur->nLevel == 0) {
                SelectObject(hdc, ptp->fontRoot);
            }
            else {
                SelectObject(hdc, ptp->fontChild);
            }
            // redraw selected item background
            if (pCur->bSelect) {
                RECT rc;
                HBRUSH brSelect = CreateSolidBrush(ptp->clrSelect);
                SetRect(&rc, 0, pCur->rcItemText.top, pCur->rcItemText.right, pCur->rcItemText.bottom);
                IntersectRect(&rc, rcUpdate, &rc);
                FillRect(hdc, &rc, brSelect);
                DeleteObject(brSelect);
            }
            // draw contents
            ItemDrawImage(ptp, pCur, hdc);
            DrawTextW(hdc, pCur->strContent, wcslen(pCur->strContent), 
                &pCur->rcItemText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

            // uncomment following line to see layout
            //FrameRect(hdc, &pCur->rcItemText, (HBRUSH)GetStockObject(WHITE_BRUSH));
        }

        pCur = ItemGetNext(pCur);
    }
}

void OnMouseMove(TOOLBOX_PANE* ptp, int x, int y)
{
    POINT pt = { x, y };
    RECT rc;
    VIEW_ITEM* pvi = ptp->pvi;
    BOOL bHoverNothing = TRUE;

    // fire a mouse track if necessary
    if (!ptp->bMouseTrack) {
        TrackMouseEvent(&ptp->tme);
    }

    while (pvi) {
        SetRect(&rc, 0, pvi->rcItemText.top, pvi->rcItemText.right, pvi->rcItemText.bottom);

        if (PtInRect(&rc, pt)) {
            bHoverNothing = FALSE;
        }

        if (PtInRect(&rc, pt) && ptp->itemHover != pvi) {
            HDC hdc = GetDC(ptp->hwnd);

            // only highlight unselected non-category item
            if (!isCategory(pvi) && !pvi->bSelect) {
                HBRUSH brHover = CreateSolidBrush(ptp->clrHover);
                FillRect(hdc, &rc, brHover);
                InvalidateRect(ptp->hwnd, &rc, FALSE);
                DeleteObject(brHover);
            }
            
            // restore previous hover item
            if (ptp->itemHover && !ptp->itemHover->bSelect) {
                SetRect(&rc, 0, ptp->itemHover->rcItemText.top,
                    ptp->itemHover->rcItemText.right, ptp->itemHover->rcItemText.bottom);
                FillRect(hdc, &rc, ptp->brBackground);
                InvalidateRect(ptp->hwnd, &rc, FALSE);
                ptp->itemHover->bHover = FALSE;
            }

            ptp->itemHover = pvi;
            pvi->bHover = TRUE;
            ReleaseDC(ptp->hwnd, hdc);

            // if there is a hit, no need to continue
            break;
        }

        pvi = ItemGetNext(pvi);
    }

    if (bHoverNothing && ptp->itemHover && !ptp->itemHover->bSelect) {
        HDC hdc = GetDC(ptp->hwnd);
        SetRect(&rc, 0, ptp->itemHover->rcItemText.top,
            ptp->itemHover->rcItemText.right, ptp->itemHover->rcItemText.bottom);
        FillRect(hdc, &rc, ptp->brBackground);
        InvalidateRect(ptp->hwnd, &rc, FALSE);
        ptp->itemHover->bHover = FALSE;
        ptp->itemHover = NULL;
        ReleaseDC(ptp->hwnd, hdc);
    }
}

void OnMouseLeave(TOOLBOX_PANE* ptp)
{
    VIEW_ITEM* pvi = ptp->itemHover;
    if (pvi && !pvi->bSelect) {
        HDC hdc = GetDC(ptp->hwnd);
        RECT rc = { 0, pvi->rcItemText.top, pvi->rcItemText.right, pvi->rcItemText.bottom };
        FillRect(hdc, &rc, ptp->brBackground);
        InvalidateRect(ptp->hwnd, &rc, FALSE);
        ptp->itemHover = NULL;
        ReleaseDC(ptp->hwnd, hdc);
    }
    ptp->bMouseTrack = FALSE;
}

void OnMouseLeftButtonPress(TOOLBOX_PANE* ptp, int x, int y)
{
    POINT pt = { x, y };
    RECT rc;
    VIEW_ITEM* pvi = ptp->pvi;

    while (pvi) {
        SetRect(&rc, 0, pvi->rcItemText.top, pvi->rcItemText.right, pvi->rcItemText.bottom);
        if (PtInRect(&rc, pt)) {
            if (ptp->itemSelect != pvi) {
                HDC hdc = GetDC(ptp->hwnd);
                HBRUSH brSelect = CreateSolidBrush(ptp->clrSelect);

                FillRect(hdc, &rc, brSelect);
                InvalidateRect(ptp->hwnd, &rc, FALSE);
                DeleteObject(brSelect);
                pvi->bSelect = TRUE;

                // restore previous selected item
                if (ptp->itemSelect) {
                    SetRect(&rc, 0, ptp->itemSelect->rcItemText.top,
                        ptp->itemSelect->rcItemText.right, ptp->itemSelect->rcItemText.bottom);
                    FillRect(hdc, &rc, ptp->brBackground);
                    InvalidateRect(ptp->hwnd, &rc, FALSE);
                    ptp->itemSelect->bSelect = FALSE;
                }

                ptp->itemSelect = pvi;
                ReleaseDC(ptp->hwnd, hdc);
            }

            // if hit a category, toggle its content
            if (isCategory(pvi)) {
                LONG dy = 0;
                RECT rcScroll;
                VIEW_ITEM* tmp = pvi->next;

                // calculate the child tree height for both cases
                while (tmp && tmp->nLevel > pvi->nLevel) {
                    dy += ptp->dmItemHeight;
                    // need to skip invisible elements since they do not occupy space
                    tmp = ItemGetNext(tmp);
                }

                // setup scroll
                SetRect(&rcScroll, 0, pvi->rcItemText.bottom, ptp->szPane.cx, ptp->szPane.cy);
                ptp->si.fMask = SIF_ALL;
                GetScrollInfo(ptp->hwnd, SB_VERT, &ptp->si);

                if (pvi->bCollapse) {
                    // need to expand
                    pvi->bCollapse = FALSE;
                }
                else {
                    // need to collapse
                    dy = -dy;
                    pvi->bCollapse = TRUE;
                }

                // update info
                ptp->si.nMax += dy;
                // all item below need to offset
                tmp = pvi->next;
                while (tmp) {
                    OffsetRect(&tmp->rcItemText, 0, dy);
                    tmp = tmp->next;
                }

                // update: reset nPage to avoid auto-fix
                ptp->si.nPage = ptp->szPane.cy;
                SetScrollInfo(ptp->hwnd, SB_VERT, &ptp->si, TRUE);
                ScrollWindowEx(ptp->hwnd, 0, dy, &rcScroll, &rcScroll, NULL, NULL, SW_ERASE | SW_INVALIDATE);

                // also update state image
                LONG dmSkip = ptp->dmItemHeight * (pvi->nImageIndex != -1 ? 2 : 1) + getTextMargin(pvi);
                SetRect(&rc, pvi->rcItemText.left - dmSkip, pvi->rcItemText.top, 
                    pvi->rcItemText.right - dmSkip, pvi->rcItemText.bottom);
                InvalidateRect(ptp->hwnd, &rc, TRUE);

                // sync info
                GetScrollInfo(ptp->hwnd, SB_VERT, &ptp->si);
            }

            // if there is a hit, no need to continue
            break;
        }

        pvi = ItemGetNext(pvi);
    }
}
