#pragma once

#include "resource.h"

BOOL InitWindowDesignerInstance(HINSTANCE hInstance, HWND hParent, int nCmdShow);
void MoveTargetWindow(int delta_horizontal, int delta_vertical);
void ScaleTargetWindow(int delta_width, int delta_height);
void WindowDesignerRefresh();
void WindowDesignerRefreshEx(HDC hdcMain);
