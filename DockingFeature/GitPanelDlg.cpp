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
#include "Process.h"
#include "resource.h"
#include <commctrl.h>
#include <shlobj.h>

#include <locale>
#include <codecvt>

#include <algorithm>
#include <fstream>
#include <vector>

extern NppData nppData;
extern bool    g_useTortoise;
extern bool    g_NppReady;
extern TCHAR   g_GitPath[MAX_PATH];;
extern HWND    hDialog;

LVITEM   LvItem;
LVCOLUMN LvCol;

#define COL_CHK  0
#define COL_I    1
#define COL_W    2
#define COL_FILE 3

static int __stdcall BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM, LPARAM pData)
{
    if (uMsg == BFFM_INITIALIZED)
        ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
    return 0;
};

void clearList()
{
    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_DELETEALLITEMS, 0, 0 );
}

std::vector<std::wstring> split( std::wstring stringToBeSplitted,
                                 std::wstring delimeter )
{
    std::vector<std::wstring> splittedString;
    size_t startIndex = 0;
    size_t endIndex = 0;

    while ( ( endIndex = stringToBeSplitted.find( delimeter,
                         startIndex ) ) < stringToBeSplitted.size() )
    {
        std::wstring val = stringToBeSplitted.substr( startIndex,
                           endIndex - startIndex );
        splittedString.push_back( val );
        startIndex = endIndex + delimeter.size();
    }

    if ( startIndex < stringToBeSplitted.size() )
    {
        std::wstring val = stringToBeSplitted.substr( startIndex );
        splittedString.push_back( val );
    }

    return splittedString;
}

void convertProcessText2Wide( std::wstring outputW, std::wstring &wide )
{
    std::string output(outputW.begin(), outputW.end());
    output.assign(outputW.begin(), outputW.end());

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    wide = converter.from_bytes(output.c_str());
}

bool updateLoc( std::wstring &loc )
{
    TCHAR pathName[MAX_PATH] = {0};
    SendMessage( nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, ( LPARAM )pathName );
    SendMessage( GetDlgItem( hDialog, IDC_EDT1 ), WM_SETTEXT, 0, ( LPARAM )pathName );

    loc = loc.append( pathName );
    return true;
}

void setListColumns( unsigned int uItem, std::wstring strI, std::wstring strW,
                 std::wstring strFile )
{
    // https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
    memset( &LvItem, 0, sizeof( LvItem ) ); // Zero struct's Members
    LvItem.mask       = LVIF_TEXT;    // Text Style
    LvItem.cchTextMax = MAX_PATH;     // Max size of text
    LvItem.iItem      = uItem;        // choose item

    LvItem.iSubItem   = COL_CHK;      // Put in first coluom
    LvItem.pszText    = TEXT( "" );
    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_INSERTITEM, 0,
                 ( LPARAM )&LvItem );

    LvItem.iSubItem   = COL_I;        // Put in second coluom
    LvItem.pszText    = const_cast<LPWSTR>( strI.c_str() );
    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_SETITEM, 0,
                 ( LPARAM )&LvItem );

    LvItem.iSubItem   = COL_W;        // Put in third coluom
    LvItem.pszText    = const_cast<LPWSTR>( strW.c_str() );
    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_SETITEM, 0,
                 ( LPARAM )&LvItem );

    LvItem.iSubItem   = COL_FILE;     // Put in fourth coluom
    LvItem.pszText    = const_cast<LPWSTR>( strFile.c_str() );
    SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_SETITEM, 0,
                 ( LPARAM )&LvItem );
}

/*
Call with:
    std::wstring selectedItems;
    if ( getListSelected( selectedItems ) )
        MessageBox( NULL, selectedItems.c_str(), TEXT( "Selected Items" ), MB_OK );
    else
        MessageBox( NULL, TEXT("NONE"), TEXT( "No Selected Items" ), MB_OK );
 */
 bool getListSelected( std::wstring &selectedItems )
{
    bool found = false;
    for (int itemInt = -1; (itemInt = ( int )::SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_GETNEXTITEM, itemInt, LVNI_SELECTED)) != -1; )
    {
        found = true;
        TCHAR file[MAX_PATH] = {0};

        memset( &LvItem, 0, sizeof(LvItem) );
        LvItem.mask       = LVIF_TEXT;
        LvItem.iSubItem   = COL_FILE;
        LvItem.pszText    = file;
        LvItem.cchTextMax = MAX_PATH;
        LvItem.iItem      = itemInt;

        SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_GETITEMTEXT, itemInt, (LPARAM)&LvItem );
        selectedItems += TEXT( " " );
        selectedItems += file;
    }

    return found;
}

bool execCommand( std::wstring command, std::wstring &wide )
{
    std::wstring pathName;
    updateLoc( pathName );

    const TCHAR *programPath = TEXT( "\0" ); // Overridden as NULL in Process.cpp
    const TCHAR *pProgramDir = pathName.c_str();
    const TCHAR *progInput   = TEXT( "" );
    const TCHAR *progOutput  = TEXT( "" );

    generic_string progInputStr  = progInput ? progInput : TEXT( "" );
    generic_string progOutputStr = progOutput ? progOutput : TEXT( "" );
    generic_string paramInput    = g_GitPath;
    paramInput += command;
    // DEBUG: MessageBox( NULL, paramInput.c_str(), TEXT("Command"), MB_OK );

    Process program( programPath, paramInput.c_str(), pProgramDir,
                     CONSOLE_PROG );
    program.run();

    if ( !program.hasStderr() )
    {
        const char *pOutput = NULL;
        size_t pOutputLen = 0;

        // If progOutput is defined, then we search the file to read
        if ( progOutputStr != TEXT( "" ) )
        {
            if ( ::PathFileExists( progOutputStr.c_str() ) )
            {
                // open the file for binary reading
                std::ifstream file;
                file.open( progOutputStr.c_str(), std::ios_base::binary );
                std::vector<byte> fileContent;

                if ( file.is_open() )
                {
                    // get the length of the file
                    file.seekg( 0, std::ios::end );
                    pOutputLen = static_cast<size_t>( file.tellg() );
                    file.seekg( 0, std::ios::beg );

                    // create a vector to hold all the bytes in the file
                    fileContent.resize( pOutputLen, 0 );

                    // read the file
                    file.read( reinterpret_cast<char *>( &fileContent[0] ),
                               ( std::streamsize )pOutputLen );

                    // close the file
                    file.close();

                    pOutput = reinterpret_cast<const char *>( &fileContent[0] );
                }

                const char errMsg[] = "ERROR: NO FILE CONTENT";

                if ( !pOutput || !( pOutput[0] ) )
                {
                    pOutput = errMsg;
                    pOutputLen = strlen( errMsg );
                }
            }
            else
            {
                ::MessageBox( NULL, TEXT( "The file is invalid" ), progOutputStr.c_str(),
                              MB_OK );
                wide = progOutputStr.c_str();
                return false;
            }
        }
        // otherwise, we look in stdout
        else if ( program.hasStdout() )
        {
            convertProcessText2Wide( program.getStdout(), wide );
            return true;
        }
    }
    else
    {
        convertProcessText2Wide( program.getStderr(), wide );
        return false;
    }
    wide = TEXT( "" );
    return false;
}

void updateList()
{
    if ( ! g_NppReady )
        return;

    clearList();

    std::wstring wide;
    if ( execCommand( TEXT( "\\git.exe status --porcelain" ), wide ) )
    {
        std::vector<std::wstring> splittedStrings = split( wide, TEXT( "\n" ) );

        for ( unsigned int i = 0; i < splittedStrings.size() ; i++ )
        {
            std::wstring strI    = splittedStrings[i].substr(0, 1);
            std::wstring strW    = splittedStrings[i].substr(1, 1);
            std::wstring strFile = splittedStrings[i].erase(0, 3);
            setListColumns( i, strI, strW, strFile );
        }
    }
    else
        setListColumns( 0, TEXT( "" ), TEXT( "" ), wide );
}

void initDialog()
{
    HWND hList = GetDlgItem( hDialog, IDC_LSV1 );

    // https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
    SendMessage( hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, ( LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES ) );

    memset( &LvCol, 0, sizeof( LvCol ) );            // Zero Members
    LvCol.mask    = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM; // Type of mask

    // Column I and W are Index and Working from:
    // https://git-scm.com/docs/git-status
    LvCol.cx      = 25;                                    // width between each coloum
    LvCol.pszText = TEXT( "" );                            // First Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_CHK, ( LPARAM )&LvCol );

    LvCol.cx      = 25;                                    // width between each coloum
    LvCol.pszText = TEXT( "I" );                           // Second Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_I, ( LPARAM )&LvCol );

    LvCol.cx      = 25;                                    // width between each coloum
    LvCol.pszText = TEXT( "W" );                           // Third Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_W, ( LPARAM )&LvCol );

    LvCol.cx      = 25;                                   // width of column
    LvCol.pszText = TEXT( "File" );                        // Fourth Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_FILE, ( LPARAM )&LvCol );

    SendMessage( hList, LVM_SETCOLUMNWIDTH, COL_FILE, LVSCW_AUTOSIZE_USEHEADER );
    updateList();
}

INT_PTR CALLBACK DemoDlg::run_dlgProc( UINT message, WPARAM wParam,
                                       LPARAM lParam )
{

    ::SendMessage( GetDlgItem( hDialog, IDC_CHK1 ), BM_SETCHECK,
                   ( LPARAM )( g_useTortoise ? 1 : 0 ), 0 );

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

                case IDC_CHK1 :
                {
                    doTortoise();
                    return TRUE;
                }

                case IDC_BTN3 :
                {
                    statusAll();
                    updateList();
                    return TRUE;
                }

                case IDC_BTN4 :
                {
                    diffFile();
                    return TRUE;
                }

                case IDC_BTN5 :
                {
                    addFile();
                    updateList();
                    return TRUE;
                }

                case IDC_BTN6 :
                {
                    commitAll();
                    updateList();
                    return TRUE;
                }

                case IDC_BTN7 :
                {
                    unstageFile();
                    updateList();
                    return TRUE;
                }

                case IDC_BTN8 :
                {
                    revertFile();
                    updateList();
                    return TRUE;
                }

                case IDC_BTN9 :
                {
                    logFile();
                    return TRUE;
                }

                case IDC_BTN10 :
                {
                    blameFile();
                    return TRUE;
                }

                case IDC_BTN11 :
                {
                    // From:
                    // npp-explorer-plugin\Explorer\src\OptionDlg\OptionDialog.cpp
                    LPMALLOC pShellMalloc = 0;
                    if (::SHGetMalloc(&pShellMalloc) == NO_ERROR)
                    {
                        // If we were able to get the shell malloc object,
                        // then proceed by initializing the BROWSEINFO stuct
                        BROWSEINFO info;
                        ZeroMemory(&info, sizeof(info));
                        info.hwndOwner          = _hParent;
                        info.pidlRoot           = NULL;
                        info.pszDisplayName     = (LPTSTR)new TCHAR[MAX_PATH];
                        info.lpszTitle          = TEXT( "Foler where GIT.exe is installed:" );
                        info.ulFlags            = BIF_RETURNONLYFSDIRS;
                        info.lpfn               = BrowseCallbackProc;
                        info.lParam             = (LPARAM)g_GitPath;

                        // Execute the browsing dialog.
                        LPITEMIDLIST pidl = ::SHBrowseForFolder(&info);

                        // pidl will be null if they cancel the browse dialog.
                        // pidl will be not null when they select a folder.
                        if (pidl)
                        {
                            // Try to convert the pidl to a display string.
                            // Return is true if success.
//                          if (
                            ::SHGetPathFromIDList( pidl, g_GitPath );
//                            )
//                          {
                                // Set edit control to the directory path.
//                              ::SetWindowText(::GetDlgItem(hDialog, IDC_EDT1), g_GitPath);
//                          }
                            pShellMalloc->Free(pidl);
                        }
                        pShellMalloc->Release();
                        delete [] info.pszDisplayName;
                    }
                    return FALSE;
                }

                case MAKELONG( IDC_EDT1, EN_SETFOCUS ) :
                {
                    updateList();
                    return FALSE;
                }

            }
            return FALSE;
        }

        case WM_NOTIFY:
        {
            switch(LOWORD(wParam))
            {
                case IDC_LSV1 :
                {
                    if( ( (LPNMHDR)lParam )->code == NM_DBLCLK )
                    {
                        std::wstring wide;
                        if ( execCommand( TEXT( "\\git.exe rev-parse --show-toplevel" ), wide ) )
                        {
                            wide.erase(std::remove(wide.begin(), wide.end(), '\n'), wide.end());

                            TCHAR file[MAX_PATH] = {0};
                            unsigned int iSlected = ( int )::SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_GETNEXTITEM, (WPARAM) -1, LVNI_FOCUSED );

                            // No Items in ListView
                            if ( iSlected == -1 )
                                break;

                            memset( &LvItem, 0, sizeof(LvItem) );
                            LvItem.mask       = LVIF_TEXT;
                            LvItem.iSubItem   = COL_FILE;
                            LvItem.pszText    = file;
                            LvItem.cchTextMax = MAX_PATH;
                            LvItem.iItem      = iSlected;

                            SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_GETITEMTEXT, iSlected, (LPARAM)&LvItem );
                            wide += TEXT( "\\" );
                            wide += file;

                            DWORD fileOrDir = GetFileAttributes( wide.c_str() );
                            if ( fileOrDir == INVALID_FILE_ATTRIBUTES )
                                break;
                            else if ( fileOrDir & FILE_ATTRIBUTE_DIRECTORY )
                            {
                                std::wstring err;
                                err += wide;
                                err += TEXT( "\n\nIs a directory.  Continue to open all files?" );
                                int ret = ( int )::MessageBox( hDialog, err.c_str(), TEXT( "Continue?" ), ( MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL ) );
                                if ( ret == IDYES )
                                    SendMessage( nppData._nppHandle, NPPM_DOOPEN, 0, ( LPARAM )wide.c_str() );
                            }
                            else
                                SendMessage( nppData._nppHandle, NPPM_DOOPEN, 0, ( LPARAM )wide.c_str() );
                        }
                        else
                        {
                            clearList();
                            setListColumns( 0, TEXT( "" ), TEXT( "" ), wide );
                        }
                    }
                    return FALSE;
                }
            }
            return FALSE;
        }

        case WM_SIZE:
        case WM_MOVE:
        {
            RECT rc = {0};
            getClientRect( rc );

            ::SetWindowPos( GetDlgItem( hDialog, IDC_EDT1 ), NULL,
                            rc.left + 15, rc.top + 215, rc.right - 25, 20,
                            SWP_NOZORDER | SWP_SHOWWINDOW );
            ::SetWindowPos( GetDlgItem( hDialog, IDC_LSV1 ), NULL,
                            rc.left + 15, rc.top + 245, rc.right - 25, rc.bottom - 260,
                            SWP_NOZORDER | SWP_SHOWWINDOW );

            SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_SETCOLUMNWIDTH, COL_FILE, LVSCW_AUTOSIZE_USEHEADER );

            redraw();
            return FALSE;
        }

        case WM_INITDIALOG :
        {
            initDialog();
        }

        default :
            return DockingDlgInterface::run_dlgProc( message, wParam, lParam );
    }
}
