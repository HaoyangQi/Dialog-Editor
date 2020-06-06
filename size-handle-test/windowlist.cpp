#include "windowlist.h"

WINDOW_LIST* NewWindowList(HWND hwndBase, BOOL isVisible)
{
	WINDOW_LIST* pList = (WINDOW_LIST*)malloc(sizeof(WINDOW_LIST));

	if (!pList) {
		return NULL;
	}

	memset(pList, 0, sizeof(WINDOW_LIST));
	pList->bVisible = isVisible;

	if (hwndBase) {
		pList->base = AddWindow(pList, hwndBase, LOCK_TOP_LEFT);
		if (!pList->base) {
			ReleaseWindowList(pList);
			return NULL;
		}
	}

	return pList;
}

void ReleaseWindowList(WINDOW_LIST* pList)
{
	if (pList) {
		WINDOW_GEOMETRY* pCurrent = pList->begin;
		WINDOW_GEOMETRY* pTmp;

		while (pCurrent) {
			pTmp = pCurrent->next;
			free(pCurrent);
			pCurrent = pTmp;
		}
	}
}

WINDOW_GEOMETRY* AddWindow(WINDOW_LIST* pList, HWND hwnd, LONG disableHandles)
{
	WINDOW_GEOMETRY* pwe = (WINDOW_GEOMETRY*)malloc(sizeof(WINDOW_GEOMETRY));

	if (!pwe) {
		return NULL;
	}

	memset(pwe, 0, sizeof(WINDOW_GEOMETRY));
	pwe->disable = disableHandles;
	pwe->hwnd = hwnd;
	GetWindowRect(hwnd, &(pwe->rect));

	pwe->prev = NULL;
	pwe->next = pList->begin;
	pList->begin = pwe;
	if (pList->end == NULL) {
		pList->end = pwe;
	}

	return pwe;
}

void DeleteWindow(WINDOW_LIST* pList, HWND hwnd)
{
	DeleteWindowByReference(pList, WindowListFind(pList, hwnd));
}

void DeleteWindowByReference(WINDOW_LIST* pList, WINDOW_GEOMETRY* pwe)
{
	if (pwe) {
		if (pwe->prev) {
			pwe->prev->next = pwe->next;
		}
		else {
			pList->begin = pwe->next;
		}

		if (pwe->next) {
			pwe->next->prev = pwe->prev;
		}
		else {
			pList->end = pwe->prev;
		}

		free(pwe);
	}
}

WINDOW_GEOMETRY* WindowListFind(const WINDOW_LIST* pList, const HWND hwnd)
{
	WINDOW_GEOMETRY* pwe = pList->begin;

	while (pwe) {
		if (pwe->hwnd == hwnd) {
			return pwe;
		}
		pwe = pwe->next;
	}

	return NULL;
}

WINDOW_GEOMETRY* WindowListHitTest(const WINDOW_LIST* pList, const POINT pt)
{
	WINDOW_GEOMETRY* pwe = pList->begin;

	while (pwe) {
		if (PtInRect(&(pwe->rect), pt)) {
			return pwe;
		}
		pwe = pwe->next;
	}

	return NULL;
}

void WindowListUpdateWindowPosition(WINDOW_GEOMETRY* window, int delta_x, int delta_y)
{
	MapWindowPoints(HWND_DESKTOP, GetParent(window->hwnd), (LPPOINT)(&(window->rect)), 2);
	MoveWindow(window->hwnd,
		window->rect.left + delta_x, window->rect.top + delta_y,
		window->rect.right - window->rect.left, window->rect.bottom - window->rect.top, TRUE);
	GetWindowRect(window->hwnd, &(window->rect));
}
