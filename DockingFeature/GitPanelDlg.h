//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef GITPANEL_DLG_H
#define GITPANEL_DLG_H

#include <string>

#include "DockingDlgInterface.h"
#include "..\PluginDefinition.h"
#include "resource.h"

typedef std::basic_string<TCHAR> generic_string;

void updateList();
void clearList();

class DemoDlg : public DockingDlgInterface
{
    public :
        DemoDlg() : DockingDlgInterface( IDD_PLUGINGITPANEL ) {};

        virtual void display( bool toShow = true ) const
        {
            DockingDlgInterface::display( toShow );
        };

        void setParent( HWND parent2set )
        {
            _hParent = parent2set;
        };

    protected :
        virtual INT_PTR CALLBACK run_dlgProc( UINT message, WPARAM wParam,
                                              LPARAM lParam );
};

// https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
typedef struct _LV_ITEM {  
    UINT   mask;        // attributes of this data structure
    int    iItem;       // index of the item to which this structure refers
    int    iSubItem;    // index of the subitem to which this structure refers
    UINT   state;       // Specifies the current state of the item
    UINT   stateMask;   // Specifies the bits of the state member that are valid. 
    LPWSTR pszText;     // Pointer to a null-terminated string
                        // that contains the item text 
    int    cchTextMax;  // Size of the buffer pointed to by the pszText member
    int    iImage;      // index of the list view item's icon 
    LPARAM lParam;      // 32-bit value to associate with item 
} LV_ITEM;

typedef struct _LV_COLUMN {  
    UINT   mask;     // which members of this structure contain valid information
    int    fmt;      // alignment of the column heading and the subitem text 
    int    cx;       // Specifies the width, in pixels, of the column.
    LPWSTR pszText;  // Pointer to a null-terminated string
                     // that contains the column heading 
    int cchTextMax;  // Specifies the size, in characters, of the buffer
    int iSubItem;    // index of subitem
} LV_COLUMN;

#endif //GITPANEL_DLG_H
