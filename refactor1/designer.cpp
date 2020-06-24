#include "designer.h"
#include "proc.h"

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
    pwd->typeTrack = NO_TRACK;
    pwd->lHandle = HANDLE_NOWHERE;
    pwd->pDlgTemplate = TemplateCreate();

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
    TemplateRelease(pwd->pDlgTemplate);
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

void DesignerResetTemplate(WINDOW_DESIGNER* pwd)
{
    // reset fields, reuse the memory
    free(pwd->pDlgTemplate->menu);
    free(pwd->pDlgTemplate->windowClass);
    free(pwd->pDlgTemplate->title);
    free(pwd->pDlgTemplate->typeface);

    DIALOG_TEMPLATE_ITEM_DATA* pItem = pwd->pDlgTemplate->pItemBegin, * pTemp;
    while (pItem) {
        pTemp = pItem->next;
        TemplateDeleteByReference(pwd->pDlgTemplate, pItem);
        pItem = pTemp;
    }
    
    memset(pwd->pDlgTemplate, 0, sizeof(DIALOG_TEMPLATE_DATA));
    pwd->pDlgTemplate->dlgVer = 1;
    pwd->pDlgTemplate->signature = 0xFFFF;
    
    // this is another way: purge everything and re-allocate
    //TemplateRelease(pwd->pDlgTemplate);
    //pwd->pDlgTemplate = TemplateCreate();
}

// Map designer data to dialog template
// use DesignerResetTemplate to free mapped data and reset template to empty state
void DesignerSyncTemplate(WINDOW_DESIGNER* pwd)
{
    // compile data from designer and fill template
    DESIGNER_CONTROL_ITEM* pci = pwd->listControls->begin;
    DIALOG_TEMPLATE_ITEM_DATA* pItem;
    WINDOWINFO infoWindow;

    // use pixel unit to do first pass, unit conversion comes afterwards
    infoWindow.cbSize = sizeof(WINDOWINFO);
    GetWindowInfo(pwd->hwndTarget, &infoWindow);

    // style
    pwd->pDlgTemplate->style = infoWindow.dwStyle;
    pwd->pDlgTemplate->style &= ~(WS_CHILDWINDOW | WS_DISABLED);
    pwd->pDlgTemplate->style |= (WS_POPUP | DS_MODALFRAME);
    // geometry: cx, cy specifies client area size
    // for now, x and y will be 0 all the time to avoid any weird position
    pwd->pDlgTemplate->x = 0;
    pwd->pDlgTemplate->y = 0;
    pwd->pDlgTemplate->cx = infoWindow.rcClient.right - infoWindow.rcClient.left;
    pwd->pDlgTemplate->cy = infoWindow.rcClient.bottom - infoWindow.rcClient.top;

    // menu: for now, just nothing
    pwd->pDlgTemplate->size_menu = sizeof(WCHAR) * 1;
    pwd->pDlgTemplate->menu = (WCHAR*)malloc(pwd->pDlgTemplate->size_menu);
    pwd->pDlgTemplate->menu[0] = 0x0000;
    // class: for now, just default
    pwd->pDlgTemplate->size_class = sizeof(WCHAR) * 1;
    pwd->pDlgTemplate->windowClass = (WCHAR*)malloc(pwd->pDlgTemplate->size_class);
    pwd->pDlgTemplate->windowClass[0] = 0x0000;
    // title
    pwd->pDlgTemplate->size_title = sizeof(WCHAR) * (GetWindowTextLengthW(pwd->hwndTarget) + 1);
    pwd->pDlgTemplate->title = (WCHAR*)malloc(pwd->pDlgTemplate->size_title);
    memset(pwd->pDlgTemplate->title, 0, pwd->pDlgTemplate->size_title);
    GetWindowTextW(pwd->hwndTarget, pwd->pDlgTemplate->title, pwd->pDlgTemplate->size_title / sizeof(WCHAR));
    // font: since it is a test, font can be fixed and internal
    pwd->pDlgTemplate->style |= DS_SETFONT;
    pwd->pDlgTemplate->pointsize = 8;
    pwd->pDlgTemplate->weight = FW_NORMAL;
    pwd->pDlgTemplate->italic = 0;
    pwd->pDlgTemplate->charset = DEFAULT_CHARSET;
    WCHAR fontName[] = L"MS Shell Dlg";
    pwd->pDlgTemplate->size_typeface = sizeof(WCHAR) * (wcslen(fontName) + 1);
    pwd->pDlgTemplate->typeface = (WCHAR*)malloc(pwd->pDlgTemplate->size_typeface);
    memcpy(pwd->pDlgTemplate->typeface, fontName, pwd->pDlgTemplate->size_typeface);
    // controls
    while (pci) {
        pItem = TemplateAddControl(pwd->pDlgTemplate);
        GetWindowInfo(pci->hwnd, &infoWindow);
        MapWindowPoints(HWND_DESKTOP, pwd->hwndTarget, (LPPOINT)&infoWindow.rcWindow, 2);
        // basic
        pItem->style = infoWindow.dwStyle;
        pItem->x = infoWindow.rcWindow.left;
        pItem->y = infoWindow.rcWindow.top;
        pItem->cx = infoWindow.rcWindow.right - infoWindow.rcWindow.left;
        pItem->cy = infoWindow.rcWindow.bottom - infoWindow.rcWindow.top;
        // class: class name is 256 characters maximum
        WCHAR strCtrlClassName[256] = L"\0";
        GetClassNameW(pci->hwnd, strCtrlClassName, 256);
        DebugPrintf(L"Control Class: %s\n", strCtrlClassName);
        if (!wcscmp(strCtrlClassName, WC_BUTTONW)) {
            pItem->size_class = sizeof(WCHAR) * 2;
            pItem->windowClass = (WCHAR*)malloc(pItem->size_class);
            pItem->windowClass[0] = 0xFFFF;
            pItem->windowClass[1] = 0x0080;
        }
        else if (!wcscmp(strCtrlClassName, WC_EDITW)) {
            pItem->size_class = sizeof(WCHAR) * 2;
            pItem->windowClass = (WCHAR*)malloc(pItem->size_class);
            pItem->windowClass[0] = 0xFFFF;
            pItem->windowClass[1] = 0x0081;
        }
        else if (!wcscmp(strCtrlClassName, WC_STATICW)) {
            pItem->size_class = sizeof(WCHAR) * 2;
            pItem->windowClass = (WCHAR*)malloc(pItem->size_class);
            pItem->windowClass[0] = 0xFFFF;
            pItem->windowClass[1] = 0x0082;
        }
        else if (!wcscmp(strCtrlClassName, WC_LISTBOXW)) {
            pItem->size_class = sizeof(WCHAR) * 2;
            pItem->windowClass = (WCHAR*)malloc(pItem->size_class);
            pItem->windowClass[0] = 0xFFFF;
            pItem->windowClass[1] = 0x0083;
        }
        else if (!wcscmp(strCtrlClassName, WC_SCROLLBARW)) {
            pItem->size_class = sizeof(WCHAR) * 2;
            pItem->windowClass = (WCHAR*)malloc(pItem->size_class);
            pItem->windowClass[0] = 0xFFFF;
            pItem->windowClass[1] = 0x0084;
        }
        else if (!wcscmp(strCtrlClassName, WC_COMBOBOXW)) {
            pItem->size_class = sizeof(WCHAR) * 2;
            pItem->windowClass = (WCHAR*)malloc(pItem->size_class);
            pItem->windowClass[0] = 0xFFFF;
            pItem->windowClass[1] = 0x0085;
        }
        else {
            // it is a non-standard control, copy over class name
            DebugPrintf(L"Non-standard Control Detected.\n");
            pItem->size_class = sizeof(WCHAR) * (wcslen(strCtrlClassName) + 1);
            pItem->windowClass = (WCHAR*)malloc(pItem->size_class);
            memset(pItem->windowClass, 0, pItem->size_class);
            wcsncpy(pItem->windowClass, strCtrlClassName, wcslen(strCtrlClassName));
        }
        // title
        pItem->size_title = sizeof(WCHAR) * (GetWindowTextLengthW(pci->hwnd) + 1);
        pItem->title = (WCHAR*)malloc(pItem->size_title);
        memset(pItem->title, 0, pItem->size_title);
        GetWindowTextW(pci->hwnd, pItem->title, pItem->size_title / sizeof(WCHAR));
        // creation data: for now, just nothing
        pItem->extraCount = 0;

        pci = pci->next;
    }
}

// compiler will produce final binary data and reset the template
PBYTE DesignerCompileTemplate(WINDOW_DESIGNER* pwd, DLG_TEMPLATE_BUILD_INFO* build)
{
    DIALOG_TEMPLATE_ITEM_DATA* pItem;
    LONG dlgBaseUnitsX, dlgBaseUnitsY;

    DesignerSyncTemplate(pwd);

    // prepare binary data buffer
    // 4 additional bytes are added as guards just in case
    build->szData = TemplateGetSizeAligned(pwd->pDlgTemplate) + 4;
    build->pBase = (PBYTE)malloc(build->szData);
    if (!build->pBase) {
        DesignerResetTemplate(pwd);
        return NULL;
    }

    // Align start of the template to DWORD boundary to make sure 
    // any padding is consistant in both memory and file.
    PBYTE pTempl = alignToDWORD(build->pBase, NULL);
    build->szOffset = pTempl - build->pBase;

    // Dummy-create a dialog to get base units, so sizes can be converted
    pwd->pDlgTemplate->style &= ~WS_VISIBLE;
    build->nStatus = TemplateBindDialogData(build->pBase, pTempl, pwd->pDlgTemplate, build->szData);
    if (build->nStatus != DLG_MEM_SUCCESS) {
        DesignerResetTemplate(pwd);
        free(build->pBase);
        return NULL;
    }
    HWND dlgContext = CreateDialogIndirectParamW(
        pwd->appInstance, (LPCDLGTEMPLATE)pTempl, pwd->hwndMain, DialogDummyProc, 0);
    RECT rcTest;
    SetRect(&rcTest, 0, 0, 4, 8);
    MapDialogRect(dlgContext, &rcTest);
    DestroyWindow(dlgContext);
    dlgBaseUnitsX = rcTest.right;
    dlgBaseUnitsY = rcTest.bottom;

    // Now convert all coordinates and sizes to dialog units
    pItem = pwd->pDlgTemplate->pItemBegin;
    pwd->pDlgTemplate->x = MulDiv(pwd->pDlgTemplate->x, 4, dlgBaseUnitsX);
    pwd->pDlgTemplate->y = MulDiv(pwd->pDlgTemplate->y, 8, dlgBaseUnitsY);
    pwd->pDlgTemplate->cx = MulDiv(pwd->pDlgTemplate->cx, 4, dlgBaseUnitsX);
    pwd->pDlgTemplate->cy = MulDiv(pwd->pDlgTemplate->cy, 8, dlgBaseUnitsY);
    while (pItem) {
        pItem->x = MulDiv(pItem->x, 4, dlgBaseUnitsX);
        pItem->y = MulDiv(pItem->y, 8, dlgBaseUnitsY);
        pItem->cx = MulDiv(pItem->cx, 4, dlgBaseUnitsX);
        pItem->cy = MulDiv(pItem->cy, 8, dlgBaseUnitsY);
        pItem = pItem->next;
    }

    // Now rebuild template and free internal template
    pwd->pDlgTemplate->style |= WS_VISIBLE;
    memset(build->pBase, 0, build->szData);
    build->nStatus = TemplateBindDialogData(build->pBase, pTempl, pwd->pDlgTemplate, build->szData);
    if (build->nStatus != DLG_MEM_SUCCESS) {
        free(build->pBase);
        build->pBase = NULL;
    }
    DesignerResetTemplate(pwd);

    return build->pBase;
}

// compile & launch dialog design
// window size would be a little different, because the compiler only preserves the 
// exact size of the client area, any size difference caused by outlook will be ignored.
int DesignerLaunchTemplate(WINDOW_DESIGNER* pwd)
{
    DLG_TEMPLATE_BUILD_INFO bi;
    PBYTE pMem = DesignerCompileTemplate(pwd, &bi);

    switch (bi.nStatus) {
        case DLG_MEM_SUCCESS:
        {
            // Do NOT use CreateDialogIndirectParam, we want it to pause main thread
            DebugPrintf(L"[DIALOG] Dialog created at: %x (+%d bytes), total %d bytes\n", 
                pMem, bi.szOffset, bi.szData);
            INT_PTR h = DialogBoxIndirectParamW(
                pwd->appInstance, (LPCDLGTEMPLATE)(pMem + bi.szOffset), pwd->hwndMain, DialogDummyProc, 0);
            if (h == -1) {
                DebugPrintf(L"[DIALOG] Error launching dialog box: %d\n", GetLastError());
            }
            DebugPrintf(L"[DIALOG] Dialog exited: %d\n", h);
            break;
        }
        case DLG_MEM_NOT_ALIGNED:
        {
            DebugPrintf(L"[DIALOG] Template memory mis-aligned on 4-byte boundary.\n");
            break;
        }
        case DLG_MEM_EXCESS_USAGE:
        {
            DebugPrintf(L"[DIALOG] Template memory corrupted, data is out of boundary.\n");
            break;
        }
        default:
            break;
    }

    free(pMem);
    return bi.nStatus;
}
