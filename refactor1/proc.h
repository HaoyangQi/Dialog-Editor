#pragma once

#include "framework.h"
#include "resource.h"
#include "global.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TargetProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DialogDummyProc(HWND, UINT, WPARAM, LPARAM);
