/** Property Grid Public Interfaces
 *
 **/

#include "PropertyGridView.h"

// If *value* is NULL, then it is a category
HPROPERTY PropertyGridNewProperty(PROPERTY_GRID* ppg, LPCWSTR key, LPCWSTR value)
{
    PROPERTY_ITEM* property = (PROPERTY_ITEM*)malloc(sizeof(PROPERTY_ITEM));

    if (property)
    {
        property->strKey = _wcsdup(key);
        property->strValueOriginal = _wcsdup(value);
        property->strValue = _wcsdup(value);
        property->strDescription = NULL;
        property->clrKey = ppg->clrTextDefault;
        property->clrVal = ppg->clrTextDefault;
        property->nLevel = 0;
        property->rcItem = { 0, 0, 0, 0 };
        property->bCollapse = FALSE;
        property->bSelect = FALSE;
        property->bDisable = FALSE;
        property->bAllowEdit = FALSE;
        property->bVisible = TRUE;
        property->lpfnVerifyProc = PropertyGridItemDefaultVerifier;
        property->data = _wcsdup(value);
        property->szData = value ? sizeof(WCHAR) * (wcslen(value) + 1) : 0;
        property->next = NULL;
        property->parent = NULL;
    }

    return property;
}

PROPERTY_ITEM* PropertyGridReleaseProperty(HPROPERTY hProperty)
{
    PROPERTY_ITEM* property = (PROPERTY_ITEM*)hProperty;
    PROPERTY_ITEM* next = property->next;

    if (property)
    {
        free(property->strKey);
        free(property->strValue);
        free(property->strValueOriginal);
        free(property->strDescription);
        free(property->data);
        free(property);
    }

    return next;
}

void PropertyGridDeleteProperty(PROPERTY_GRID* ppg, HPROPERTY hProperty)
{
    PROPERTY_ITEM* property = (PROPERTY_ITEM*)hProperty;
    PROPERTY_ITEM* previous;
    PROPERTY_ITEM* cur;
    int deleteHeight = 0;

    if (property)
    {
        LONG level = property->nLevel;
        BOOL bRepaint = PropertyGridItemIsValid(ppg, property, &previous);

        if (bRepaint)
        {
            // Delete category content
            cur = property->next;
            while (cur && cur->nLevel > level)
            {
                deleteHeight += cur->bVisible ? ppg->dmItemHeight : 0;
                cur = PropertyGridReleaseProperty(cur);
            }
            
            // Detach item
            if (previous)
            {
                previous->next = cur;
            }
            else
            {
                ppg->content = cur;
            }

            PropertyGridReleaseProperty(property);
            deleteHeight += property->bVisible ? ppg->dmItemHeight : 0;

            // Update scroll info, redraw
            if (cur)
            {
                RECT rc = { 0, cur->rcItem.top, ppg->szControl.cx, ppg->szControl.cy };
                while (cur)
                {
                    OffsetRect(&cur->rcItem, 0, -deleteHeight);
                    cur = cur->next;
                }
                PropertyGridUpdateScrollRange(ppg);
                ScrollWindowEx(ppg->hwnd, 0, ppg->dmItemHeight, &rc, &rc, NULL, NULL, SW_ERASE | SW_INVALIDATE);
            }
        }
    }
}

BOOL PropertyGridAddProperty(PROPERTY_GRID* ppg, HPROPERTY hProperty)
{
    PROPERTY_ITEM* current = ppg->content;
    PROPERTY_ITEM* property = (PROPERTY_ITEM*)hProperty;
    PROPERTY_ITEM* previous;

    if (PropertyGridItemIsValid(ppg, property, &previous))
    {
        return FALSE;
    }

    if (!current)
    {
        ppg->content = property;
        SetRect(&property->rcItem, 0, 0, ppg->szControl.cx, ppg->dmItemHeight);
    }
    else
    {
        previous->next = property;
        CopyRect(&property->rcItem, &previous->rcItem);
        OffsetRect(&property->rcItem, 0, ppg->dmItemHeight);
    }

    // Update scroll info
    PropertyGridUpdateScrollRange(ppg);

    // Redraw this item
    InvalidateRect(ppg->hwnd, &property->rcItem, TRUE);

    return TRUE;
}

BOOL PropertyGridAddSubItem(PROPERTY_GRID* ppg, HPROPERTY hParent, HPROPERTY hProperty)
{
    PROPERTY_ITEM* parent = (PROPERTY_ITEM*)hParent;
    PROPERTY_ITEM* property = (PROPERTY_ITEM*)hProperty;
    LONG levelParent = parent->nLevel;

    if (!isCategory(parent) || !property || 
        PropertyGridItemIsValid(ppg, property, NULL) || !PropertyGridItemIsValid(ppg, parent, NULL))
    {
        return FALSE;
    }

    property->nLevel = levelParent + 1;
    property->parent = parent;
    property->bVisible = TRUE;

    // Test visibility
    while (parent)
    {
        if (!parent->bVisible || parent->bCollapse)
        {
            property->bVisible = FALSE;
            break;
        }
        parent = parent->parent;
    }

    // Add property
    parent = (PROPERTY_ITEM*)hParent;
    while (parent && parent->next && parent->next->nLevel > levelParent)
    {
        parent = parent->next;
    }
    property->next = parent->next;
    parent->next = property;

    // Adjust item rect
    CopyRect(&property->rcItem, &parent->rcItem);
    while (property)
    {
        OffsetRect(&property->rcItem, 0, ppg->dmItemHeight);
        property = property->next;
    }

    // Update scroll info, scroll content, and redraw
    property = (PROPERTY_ITEM*)hProperty;
    if (property->bVisible)
    {
        RECT rc = { 0, property->rcItem.top, ppg->szControl.cx, ppg->szControl.cy };

        PropertyGridUpdateScrollRange(ppg);
        ScrollWindowEx(ppg->hwnd, 0, ppg->dmItemHeight, &rc, &rc, NULL, NULL, SW_ERASE | SW_INVALIDATE);
    }

    return TRUE;
}

void PropertyGridAddOption(PROPERTY_GRID* ppg)
{
    // TODO
}

void PropertyGridAddColorProperty(PROPERTY_GRID* ppg)
{
    // TODO
}

void PropertyGridAddFontProperty(PROPERTY_GRID* ppg)
{
    // TODO
}

void PropertyGridSetVerifier(HPROPERTY hProperty, PROCVERIFY verifier, BOOL bValidate)
{
    PROPERTY_ITEM* property = (PROPERTY_ITEM*)hProperty;

    if (verifier)
    {
        property->lpfnVerifyProc = verifier;
    }

    if (bValidate)
    {
        PropertyGridItemVerify(property);
    }
}

void PropertyGridDeleteAll(PROPERTY_GRID* ppg)
{
    PROPERTY_ITEM* cur = ppg->content;

    while (cur)
    {
        cur = PropertyGridReleaseProperty(cur);
    }

    ppg->content = NULL;
}

void PropertyGridSetSelection(PROPERTY_GRID* ppg, HPROPERTY hProperty)
{
    PROPERTY_ITEM* property = (PROPERTY_ITEM*)hProperty;
    RECT rc;

    if (property != ppg->itemSelect)
    {
        // Restore previous selection
        if (ppg->itemSelect)
        {
            ppg->itemSelect->bSelect = FALSE;
            PropertyGridItemGetKeyRect(ppg, ppg->itemSelect, &rc);
            InvalidateRect(ppg->hwnd, &rc, TRUE);
            PropertyGridItemGetValueRect(ppg, ppg->itemSelect, &rc);
            InvalidateRect(ppg->hwnd, &rc, TRUE);
        }

        // Highlight new selection
        property->bSelect = TRUE;
        PropertyGridItemGetKeyRect(ppg, property, &rc);
        InvalidateRect(ppg->hwnd, &rc, TRUE);
        PropertyGridItemGetValueRect(ppg, property, &rc);
        InvalidateRect(ppg->hwnd, &rc, TRUE);

        // Attach info
        ppg->itemSelect = property;
    }
}
