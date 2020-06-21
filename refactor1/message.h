#pragma once

#include "framework.h"
#include "resource.h"
#include "designer.h"

void OnMainCreate(WINDOW_DESIGNER*, HWND);
void OnMainPaint(WINDOW_DESIGNER*, HWND);
void OnMainLButtonPress(WINDOW_DESIGNER*, HWND, WPARAM, LPARAM);
void OnMainMouseMove(WINDOW_DESIGNER*, WPARAM, LONG, LONG);
void OnMainLButtonDrag(WINDOW_DESIGNER*, HWND, LONG, LONG);
void OnMainLButtonRelease(WINDOW_DESIGNER*, HWND);

void OnTargetPaint(WINDOW_DESIGNER*, HWND);
