#include "proc.h"
#include "designer.h"
#include "message.h"

// forward from refactor1.cpp
extern WCHAR szTargetLayerClass[MAX_LOADSTRING];
extern WCHAR szTargetTitle[MAX_LOADSTRING];
extern WINDOW_DESIGNER designerData;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            OnMainCreate(&designerData, hWnd);
            break;
        }
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_ABOUT:
                    DialogBox(designerData.appInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                case IDM_RUNDIALOG:
                    DesignerLaunchTemplate(&designerData);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        case WM_LBUTTONDOWN:
        {
            OnMainLButtonPress(&designerData, hWnd, wParam, lParam);
            break;
        }
        case WM_LBUTTONUP:
        {
            OnMainLButtonRelease(&designerData, hWnd);
            break;
        }
        case WM_MOUSEMOVE:
        {
            LONG x = GET_X_LPARAM(lParam);
            LONG y = GET_Y_LPARAM(lParam);

            OnMainMouseMove(&designerData, wParam, x, y);
            if (wParam & MK_LBUTTON) {
                OnMainLButtonDrag(&designerData, hWnd, x, y);
            }

            break;
        }
        case WM_PAINT:
        {
            OnMainPaint(&designerData, hWnd);
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

// TODO: handle resizing message to update margin box data
LRESULT CALLBACK TargetProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_PAINT:
        {
            OnTargetPaint(&designerData, hWnd);
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

INT_PTR CALLBACK DialogDummyProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    case WM_COMMAND:
        break;
    default:
        break;
    }

    return FALSE;
}
