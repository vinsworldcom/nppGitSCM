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

#include "GitPanelDlg.h"
#include "../PluginDefinition.h"

extern NppData nppData;

INT_PTR CALLBACK DemoDlg::run_dlgProc( UINT message, WPARAM wParam,
                                       LPARAM lParam )
{
    switch ( message )
    {
        case WM_COMMAND :
        {
            switch ( wParam )
            {
                case IDC_BTN1 :
                {
                    gitGui();
                    return TRUE;
                }
                case IDC_BTN2 :
                {
                    giTk();
                    return TRUE;
                }
                case IDC_BTN3 :
                {
                    statusAll();
                    return TRUE;
                }
                case IDC_BTN4 :
                {
                    commitAll();
                    return TRUE;
                }
                case IDC_BTN6 :
                {
                    addFile();
                    return TRUE;
                }
                case IDC_BTN7 :
                {
                    diffFile();
                    return TRUE;
                }
                case IDC_BTN8 :
                {
                    revertFile();
                    return TRUE;
                }
                case IDC_BTN10 :
                {
                    logFile();
                    return TRUE;
                }
                case IDC_BTN11 :
                {
                    blameFile();
                    return TRUE;
                }
            }
            return FALSE;
        }

        default :
            return DockingDlgInterface::run_dlgProc( message, wParam, lParam );
    }
}

