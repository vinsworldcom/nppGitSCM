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
#include <vector>

#include "DockingDlgInterface.h"
#include "..\PluginDefinition.h"
#include "resource.h"

typedef std::basic_string<TCHAR> generic_string;

std::vector<std::wstring> getListSelected();
bool updateLoc( std::wstring & );
void updateListWithDelay();
void updateList();
void clearList();
bool execCommand( std::wstring, std::wstring & );
void SetNppColors();
void SetSysColors();
void ChangeColors();

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

#endif //GITPANEL_DLG_H
