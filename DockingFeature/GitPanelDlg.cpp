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

#include "../PluginDefinition.h"
#include "ContextMenu.h"
#include "GitPanelDlg.h"
#include "Process.h"
#include "SettingsDlg.h"
#include "resource.h"

#include <algorithm>
#include <codecvt>
#include <commctrl.h>
#include <fstream>
#include <locale>
#include <shlobj.h>
#include <vector>
#include <windowsx.h>

extern DemoDlg _gitPanel;
extern TCHAR g_GitPath[MAX_PATH];
extern TCHAR g_GitPrompt[MAX_PATH];
extern bool  g_useTortoise;
extern bool  g_NppReady;
extern bool  g_useNppColors;
extern bool  g_RaisePanel;
extern bool  g_Debug;
extern int   g_LVDelay;

LVITEM   LvItem;
LVCOLUMN LvCol;
COLORREF colorBg;
COLORREF colorFg;

// #define COL_CHK  0
#define COL_I    0
#define COL_W    1
#define COL_FILE 2
#define TIMER_ID 1

const int WS_TOOLBARSTYLE = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS |TBSTYLE_FLAT | CCS_TOP | BTNS_AUTOSIZE | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;
                         /* WS_CHILD | WS_VISIBLE |                                                                                                                    CCS_NORESIZE |                CCS_ADJUSTABLE */

TBBUTTON tbButtonsAdd1[] =
{
    {MAKELONG( 0, 0 ), IDC_BTN_GITGUI,  TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 1, 0 ), IDC_BTN_GITK,    TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 2, 0 ), IDC_BTN_PROMPT,  TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,               0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 3, 0 ), IDC_BTN_PULL,    TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 4, 0 ), IDC_BTN_STATUS,  TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,               0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 5, 0 ), IDC_BTN_COMMIT,  TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 6, 0 ), IDC_BTN_PUSH,    TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0}
};
const int sizeButtonArray1 = sizeof( tbButtonsAdd1 ) / sizeof( TBBUTTON );
const int numButtons1      = sizeButtonArray1 - 2 /* separators */;

TBBUTTON tbButtonsAdd2[] =
{
    {MAKELONG( 0, 0 ), IDC_BTN_ADD,      TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 1, 0 ), IDC_BTN_UNSTAGE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 2, 0 ), IDC_BTN_RESTORE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 3, 0 ), IDC_BTN_DIFF,     TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,                0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 4, 0 ), IDC_BTN_LOG,      TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {MAKELONG( 5, 0 ), IDC_BTN_BLAME,    TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0},
    {0,                0,                0,               BTNS_SEP,       {0}, 0, 0},
    {MAKELONG( 6, 0 ), IDC_BTN_SETTINGS, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0}, 0, 0}
};
const int sizeButtonArray2 = sizeof( tbButtonsAdd2 ) / sizeof( TBBUTTON );
const int numButtons2      = sizeButtonArray2 - 2 /* separators */;

static LPCTSTR szToolTip[16] = {
    TEXT("Git GUI"),
    TEXT("GiTk"),
    TEXT("Git Prompt"),
    TEXT("Pull"),
    TEXT("Status"),
    TEXT("Commit"),
    TEXT("Push"),
    TEXT("Add"),
    TEXT("Unstage"),
    TEXT("Restore"),
    TEXT("Diff"),
    TEXT("Log"),
    TEXT("Blame"),
    TEXT("Settings")
};

LPCTSTR GetNameStrFromCmd( UINT_PTR resID )
{
    if ((IDC_BTN_GITGUI <= resID) && (resID <= IDC_BTN_SETTINGS))
    {
        return szToolTip[resID - IDC_BTN_GITGUI];
    }
    return NULL;
}

void imageToolbar( HINSTANCE hInst, HWND hWndToolbar, UINT ToolbarID, const int numButtons )
{
    HBITMAP hbm = LoadBitmap( hInst, MAKEINTRESOURCE( ToolbarID ) );
    BITMAP bm = {0};
    GetObject( hbm, sizeof( BITMAP ), &bm );
    int iImageWidth  = bm.bmWidth / numButtons;
    int iImageHeight = bm.bmHeight;
    HIMAGELIST himlToolBar1 = ( HIMAGELIST )SendMessage( hWndToolbar, TB_GETIMAGELIST, 0, 0 );
    ImageList_Destroy( himlToolBar1 );
    himlToolBar1 = ImageList_Create( iImageWidth, iImageHeight, ILC_COLOR32 | ILC_MASK, numButtons, 0 );
    ImageList_AddMasked( himlToolBar1, hbm, RGB( 192, 192, 192 ) );
    SendMessage( hWndToolbar, TB_SETIMAGELIST, 0, ( LPARAM )himlToolBar1 );
}

std::vector<std::wstring> DemoDlg::split( std::wstring stringToBeSplitted, std::wstring delimeter )
{
    std::vector<std::wstring> splittedString;
    size_t startIndex = 0;
    size_t endIndex = 0;

    while ( ( endIndex = stringToBeSplitted.find( delimeter, startIndex ) ) < stringToBeSplitted.size() )
    {
        std::wstring val = stringToBeSplitted.substr( startIndex, endIndex - startIndex );
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

bool DemoDlg::updateLoc( std::wstring &loc )
{
    TCHAR pathName[MAX_PATH] = {0};
    SendMessage( _hParent, NPPM_GETCURRENTDIRECTORY, MAX_PATH, ( LPARAM )pathName );
    SendMessage( GetDlgItem( _hSelf, IDC_EDT_DIR ), WM_SETTEXT, 0, ( LPARAM )pathName );

    loc = pathName;
    return true;
}

void DemoDlg::setListColumns( unsigned int uItem, std::wstring strI, std::wstring strW, std::wstring strFile )
{
    // https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
    memset( &LvItem, 0, sizeof( LvItem ) ); // Zero struct's Members
    LvItem.mask       = LVIF_TEXT;          // Text Style
    LvItem.cchTextMax = MAX_PATH;           // Max size of text
    LvItem.iItem      = uItem;              // choose item

    // LvItem.iSubItem = COL_CHK;         // Put in first coluom
    // LvItem.pszText  = TEXT( "" );
    // SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_INSERTITEM, 0, ( LPARAM )&LvItem );

    LvItem.iSubItem = COL_I;        // Put in second coluom
    LvItem.pszText  = const_cast<LPWSTR>( strI.c_str() );
    SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_INSERTITEM, 0, ( LPARAM )&LvItem );

    LvItem.iSubItem = COL_W;        // Put in third coluom
    LvItem.pszText  = const_cast<LPWSTR>( strW.c_str() );
    SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_SETITEM, 0, ( LPARAM )&LvItem );

    LvItem.iSubItem = COL_FILE;     // Put in fourth coluom
    LvItem.pszText  = const_cast<LPWSTR>( strFile.c_str() );
    SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_SETITEM, 0, ( LPARAM )&LvItem );
}

std::vector<std::wstring> DemoDlg::getListSelected(void)
{
    std::vector<std::wstring> selectedItems;
    int itemInt = -1;
    itemInt = ( int )::SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_GETNEXTITEM, itemInt, LVNI_SELECTED );
    if ( itemInt == -1 )
        return selectedItems;

    std::wstring wide = TEXT( "" );
    if ( execCommand( TEXT( "git.exe rev-parse --show-toplevel" ), wide ) )
    {
        wide.erase(std::remove(wide.begin(), wide.end(), '\n'), wide.end());

        for (itemInt = -1; ( itemInt = ( int )::SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_GETNEXTITEM, itemInt, LVNI_SELECTED ) ) != -1; )
        {
            TCHAR file[MAX_PATH] = {0};

            memset( &LvItem, 0, sizeof(LvItem) );
            LvItem.mask       = LVIF_TEXT;
            LvItem.iSubItem   = COL_FILE;
            LvItem.pszText    = file;
            LvItem.cchTextMax = MAX_PATH;
            LvItem.iItem      = itemInt;

            SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_GETITEMTEXT, itemInt, (LPARAM)&LvItem );

            std::wstring tempPath = wide;
            tempPath += TEXT( "\\" );
            tempPath += file;

            DWORD fileOrDir = GetFileAttributes( tempPath.c_str() );
            if ( fileOrDir == INVALID_FILE_ATTRIBUTES )
            {
                selectedItems = {};
                return selectedItems;
            }

            for (unsigned int j = 0; j < tempPath.size(); j++)
            {
                if (tempPath[j] == '/')
                {
                    tempPath[j] = '\\';
                }
            }

            selectedItems.push_back( tempPath );
        }
    }
    return selectedItems;
}

bool DemoDlg::execCommand( std::wstring command, std::wstring &wide )
{
    wide = TEXT( "" );

    std::wstring pathName = TEXT( "" );
    updateLoc( pathName );
    if ( pathName.empty() )
        return false;

    const TCHAR *programPath = TEXT( "\0" ); // Overridden as NULL in Process.cpp
    const TCHAR *pProgramDir = pathName.c_str();
    const TCHAR *progInput   = TEXT( "" );
    const TCHAR *progOutput  = TEXT( "" );

    generic_string progInputStr  = progInput ? progInput : TEXT( "" );
    generic_string progOutputStr = progOutput ? progOutput : TEXT( "" );
    generic_string paramInput    = getGitLocation();
    paramInput += command;

    if ( g_Debug )
        OutputDebugString( paramInput.c_str() );

    Process program( programPath, paramInput.c_str(), pProgramDir, CONSOLE_PROG );
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
                    file.read( reinterpret_cast<char *>( &fileContent[0] ), ( std::streamsize )pOutputLen );

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
                ::MessageBox( NULL, TEXT( "The file is invalid" ), progOutputStr.c_str(), MB_OK );
                wide = progOutputStr.c_str();
                return false;
            }
        }
        // otherwise, we look in stdout
        else if ( program.hasStdout() )
        {
            wide = program.getStdout();
            return true;
        }
    }
    else
    {
        wide = program.getStderr();
        return false;
    }
    return false;
}

void DemoDlg::updateListWithDelay()
{
    if ( ! g_NppReady )
        return;

    KillTimer( _hSelf, TIMER_ID );
    SetTimer( _hSelf, TIMER_ID, g_LVDelay, NULL );
}

void DemoDlg::updateList()
{
    KillTimer( _hSelf, TIMER_ID );

    if ( ! g_NppReady )
        return;

    // HANDLE hThread = CreateThread(NULL, 0, _static_updateList, (void*) this, 0, NULL);
    // CloseHandle(hThread);
    _updateList();
}

DWORD DemoDlg::_updateList()
{
    // clear list
    SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_DELETEALLITEMS, 0, 0 );

    std::wstring wide = TEXT( "" );
    if ( execCommand( TEXT( "git.exe status --porcelain --branch" ), wide ) )
    {
        std::vector<std::wstring> splittedStrings = split( wide, TEXT( "\n" ) );

        std::wstring strBranch = splittedStrings[0].erase(0, 3);
        SendMessage( GetDlgItem( _hSelf, IDC_EDT_BRANCH ), WM_SETTEXT, 0, ( LPARAM )strBranch.c_str() );
        for ( unsigned int i = 1; i < splittedStrings.size() ; i++ )
        {
            std::wstring strI    = splittedStrings[i].substr(0, 1);
            std::wstring strW    = splittedStrings[i].substr(1, 1);
            std::wstring strFile = splittedStrings[i].erase(0, 3);
            setListColumns( i-1, strI, strW, strFile );
        }
    }
    else
    {
        SendMessage( GetDlgItem( _hSelf, IDC_EDT_BRANCH ), WM_SETTEXT, 0, ( LPARAM )wide.c_str() );
        setListColumns( 0, TEXT( "" ), TEXT( "" ), TEXT( "" ) );
    }

    return 0;
}

void DemoDlg::SetNppColors()
{
    colorBg = ( COLORREF )::SendMessage( getCurScintilla(), SCI_STYLEGETBACK, 0, 0 );
    colorFg = ( COLORREF )::SendMessage( getCurScintilla(), SCI_STYLEGETFORE, 0, 0 );
}

void DemoDlg::SetSysColors()
{
    colorBg = GetSysColor( COLOR_WINDOW );
    colorFg = GetSysColor( COLOR_WINDOWTEXT );
}

void DemoDlg::ChangeColors()
{
    HWND hList = GetDlgItem( _hSelf, IDC_LSV1 );

    ::SendMessage(hList, WM_SETREDRAW, FALSE, 0);

    ListView_SetBkColor( hList, colorBg );
    ListView_SetTextBkColor( hList, colorBg);
    ListView_SetTextColor( hList, colorFg);

    ::SendMessage(hList, WM_SETREDRAW, TRUE, 0);
    ::RedrawWindow(hList, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void DemoDlg::refreshDialog()
{
    SendMessage( GetDlgItem( _hSelf, IDC_CHK_TORTOISE ),    BM_SETCHECK, ( WPARAM )( g_useTortoise  ? 1 : 0 ), 0 );
    SendMessage( GetDlgItem( _hSelf, IDC_CHK_NPPCOLOR ),    BM_SETCHECK, ( WPARAM )( g_useNppColors ? 1 : 0 ), 0 );
    SendMessage( GetDlgItem( _hSelf, IDC_CHK_PANELTOGGLE ), BM_SETCHECK, ( WPARAM )( g_RaisePanel   ? 1 : 0 ), 0 );
}

void DemoDlg::initDialog()
{
    INITCOMMONCONTROLSEX ic;

    ic.dwSize = sizeof( INITCOMMONCONTROLSEX );
    ic.dwICC = ICC_BAR_CLASSES | ICC_PAGESCROLLER_CLASS;
    InitCommonControlsEx( &ic );

    HWND hWndToolbar1, hWndToolbar2, hWndPager1, hWndPager2;
    HMODULE module_name = GetModuleHandle( _moduleName.c_str() );

    // TOOLBAR1
    // Create pager.  The parent window is the parent.
    hWndPager1 = CreateWindow( WC_PAGESCROLLER, NULL, WS_VISIBLE | WS_CHILD | PGS_HORZ,
                               0, 0, 200, 32, _hSelf, (HMENU) IDB_PAGER1, module_name, NULL );
    // Create Toolbar.  The parent window is the Pager.
    hWndToolbar1 = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL, WS_TOOLBARSTYLE,
                                   0, 0, 200, 32, hWndPager1, ( HMENU ) IDB_TOOLBAR1, module_name, NULL );

    SendMessage( hWndToolbar1, TB_BUTTONSTRUCTSIZE, sizeof( TBBUTTON ), 0 );
    SendMessage( hWndToolbar1, TB_SETEXTENDEDSTYLE, 0, ( LPARAM ) TBSTYLE_EX_HIDECLIPPEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS );
    SendMessage( hWndToolbar1, TB_ADDBUTTONS, sizeButtonArray1, ( LPARAM )tbButtonsAdd1 );
    SendMessage( hWndToolbar1, TB_AUTOSIZE, 0, 0 );
    // Notify the pager that it contains the toolbar
    SendMessage( hWndPager1, PGM_SETCHILD, 0, (LPARAM) hWndToolbar1 );
    imageToolbar( module_name, hWndToolbar1, IDB_TOOLBAR1, numButtons1 );

    // TOOLBAR2
    // Create pager.  The parent window is the parent.
    hWndPager2 = CreateWindow( WC_PAGESCROLLER, NULL, WS_VISIBLE | WS_CHILD | PGS_HORZ,
                               0, 32, 200, 32, _hSelf, (HMENU) IDB_PAGER2, module_name, NULL );
    // Create Toolbar.  The parent window is the Pager.
    hWndToolbar2 = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL, WS_TOOLBARSTYLE,
                                   0, 0, 200, 32, hWndPager2, ( HMENU ) IDB_TOOLBAR2, module_name, NULL );

    SendMessage( hWndToolbar2, TB_BUTTONSTRUCTSIZE, sizeof( TBBUTTON ), 0 );
    SendMessage( hWndToolbar1, TB_SETEXTENDEDSTYLE, 0, ( LPARAM ) TBSTYLE_EX_HIDECLIPPEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS );
    SendMessage( hWndToolbar2, TB_ADDBUTTONS, sizeButtonArray2, ( LPARAM )tbButtonsAdd2 );
    SendMessage( hWndToolbar2, TB_AUTOSIZE, 0, 0 );
    // in CreateWindowEx()
    SendMessage( hWndPager2, PGM_SETCHILD, 0, (LPARAM) hWndToolbar2 );
    imageToolbar( module_name, hWndToolbar2, IDB_TOOLBAR2, numButtons2 );

    // Edit and List controls
    if ( g_useNppColors )
        SetNppColors();
    else
        SetSysColors();
    ChangeColors();
    refreshDialog();

    HWND hList = GetDlgItem( _hSelf, IDC_LSV1 );

    // https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
    SendMessage( hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, ( LVS_EX_FULLROWSELECT /*| LVS_EX_CHECKBOXES*/ ) );

    memset( &LvCol, 0, sizeof( LvCol ) );            // Zero Members
    LvCol.mask    = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM; // Type of mask

    // Column I and W are Index and Working from:
    // https://git-scm.com/docs/git-status
    // LvCol.cx      = 25;                                    // width between each coloum
    // LvCol.pszText = TEXT( "" );                            // First Header Text
    // SendMessage( hList, LVM_INSERTCOLUMN, COL_CHK, ( LPARAM )&LvCol );

    LvCol.cx      = 25;                                    // width between each coloum
    LvCol.pszText = TEXT( "I" );                           // Second Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_I, ( LPARAM )&LvCol );

    LvCol.cx      = 25;                                    // width between each coloum
    LvCol.pszText = TEXT( "W" );                           // Third Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_W, ( LPARAM )&LvCol );

    LvCol.cx      = 25;                                    // width of column
    LvCol.pszText = TEXT( "File" );                        // Fourth Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, COL_FILE, ( LPARAM )&LvCol );

    SendMessage( hList, LVM_SETCOLUMNWIDTH, COL_FILE, LVSCW_AUTOSIZE_USEHEADER );

    updateList();
}

void DemoDlg::gotoFile()
{
    std::vector<std::wstring> files = getListSelected();
    if ( files.size() == 0 )
        return;

    for ( unsigned int i = 0; i < files.size(); i++ )
    {
        DWORD fileOrDir = GetFileAttributes( files[i].c_str() );
        if ( fileOrDir == INVALID_FILE_ATTRIBUTES )
            return;
        else if ( fileOrDir & FILE_ATTRIBUTE_DIRECTORY )
        {
            std::wstring err = TEXT( "" );
            err += files[i];
            err += TEXT( "\n\nIs a directory.  Continue to open all files?" );
            int ret = ( int )::MessageBox( _hSelf, err.c_str(), TEXT( "Continue?" ), ( MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL ) );
            if ( ret == IDYES )
                SendMessage( _hParent, NPPM_DOOPEN, 0, ( LPARAM )files[i].c_str() );
        }
        else
            SendMessage( _hParent, NPPM_DOOPEN, 0, ( LPARAM )files[i].c_str() );
    }
}

INT_PTR CALLBACK DemoDlg::run_dlgProc( UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        case WM_INITDIALOG :
        {
            initDialog();
            break;
        }

        case WM_COMMAND :
        {
            switch ( wParam )
            {
                case IDC_BTN_GITGUI :
                {
                    gitGui();
                    return TRUE;
                }

                case IDC_BTN_GITK :
                {
                    giTk();
                    return TRUE;
                }

                case IDC_BTN_PROMPT :
                {
                    std::wstring pathName = TEXT( "" );
                    updateLoc( pathName );
                    ShellExecute( _hParent, TEXT("open"), g_GitPrompt, NULL, pathName.c_str(), SW_SHOW );
                    return TRUE;
                }

                case IDC_BTN_PULL :
                {
                    pullFile();
                    return TRUE;
                }

                case IDC_BTN_STATUS :
                {
                    statusAll();
                    return TRUE;
                }

                case IDC_BTN_COMMIT :
                {
                    commitAll();
                    return TRUE;
                }

                case IDC_BTN_PUSH :
                {
                    pushFile();
                    return TRUE;
                }

                case IDC_BTN_ADD :
                {
                    std::vector<std::wstring> files = getListSelected();
                    if ( files.size() == 0 )
                        addFile();
                    else
                        addFileFiles( files );
                    updateListWithDelay();
                    return TRUE;
                }

                case IDC_BTN_UNSTAGE :
                {
                    std::vector<std::wstring> files = getListSelected();
                    if ( files.size() == 0 )
                        unstageFile();
                    else
                        unstageFileFiles( files );
                    updateListWithDelay();
                    return TRUE;
                }

                case IDC_BTN_RESTORE :
                {
                    std::vector<std::wstring> files = getListSelected();
                    if ( files.size() == 0 )
                        restoreFile();
                    else
                        restoreFileFiles( files );
                    updateListWithDelay();
                    return TRUE;
                }

                case IDC_BTN_DIFF :
                {
                    std::vector<std::wstring> files = getListSelected();
                    if ( files.size() == 0 )
                        diffFile();
                    else
                        diffFileFiles( files );
                    return TRUE;
                }

                case IDC_BTN_LOG :
                {
                    logFile();
                    return TRUE;
                }

                case IDC_BTN_BLAME :
                {
                    blameFile();
                    return TRUE;
                }

                case IDB_BTN_BRANCH :
                {
                    branchFile();
                    updateListWithDelay();
                    return TRUE;
                }

                case IDC_BTN_SETTINGS :
                {
                    doSettings();
                    return TRUE;
                }

                case IDC_CHK_TORTOISE :
                {
                    doTortoise();
                    return TRUE;
                }

                case IDC_CHK_NPPCOLOR:
                {
                    int check = ( int )::SendMessage( GetDlgItem( _hSelf, IDC_CHK_NPPCOLOR ), BM_GETCHECK, 0, 0 );

                    if ( check & BST_CHECKED )
                    {
                        SetNppColors();
                        g_useNppColors = true;
                    }
                    else
                    {
                        SetSysColors();
                        g_useNppColors = false;
                    }

                    ChangeColors();
                    refreshDialog();
                    return TRUE;
                }

                case IDC_CHK_PANELTOGGLE:
                {
                    int check = ( int )::SendMessage( GetDlgItem( _hSelf, IDC_CHK_PANELTOGGLE ), BM_GETCHECK, 0, 0 );

                    if ( check & BST_CHECKED )
                        g_RaisePanel = true;
                    else
                        g_RaisePanel = false;
                    return TRUE;
                }

                case IDB_BTN_GOTOREMOTE:
                {
                    std::wstring wide = TEXT( "" );
                    if ( execCommand( TEXT( "git config --get remote.origin.url" ), wide ) )
                    {
                        ShellExecute(_hSelf, TEXT("open"), wide.c_str(), NULL, NULL, SW_SHOWNORMAL);
                    }
                    return TRUE;
                }

                case MAKELONG( IDC_EDT_BRANCH, EN_SETFOCUS ) :
                case MAKELONG( IDC_EDT_DIR, EN_SETFOCUS ) :
                {
                    updateListWithDelay();
                    return TRUE;
                }

                // Trap VK_ENTER in the LISTVIEW
                case IDOK :
                {
                    HWND hWndCtrl = GetFocus();
                    if ( hWndCtrl == GetDlgItem( _hSelf, IDC_LSV1 ) )
                        gotoFile();
                    return TRUE;
                }
                // Trap VK_ESCAPE
                case IDCANCEL :
                {
                    if ( _gitPanel.isFloating() )
                    {
                        EndDialog(_hSelf, 0);
                        _gitPanel.display(false);
                    }
                    else
                        ::SetFocus( getCurScintilla() );

                    return TRUE;
                }
            }
            break;
        }

        case WM_TIMER:
        {
            KillTimer( _hSelf, TIMER_ID );
            updateList();
            return FALSE;
        }

        case WM_NOTIFY:
        {
            LPNMHDR nmhdr = (LPNMHDR)lParam;

            switch ( nmhdr->code )
            {
                case NM_DBLCLK:
                {
                    if ( nmhdr->hwndFrom == GetDlgItem( _hSelf, IDC_LSV1 ) )
                    {
                        POINT         pt    = {0};
                        LVHITTESTINFO ht    = {0};
                        DWORD         dwpos = ::GetMessagePos();

                        pt.x = GET_X_LPARAM(dwpos);
                        pt.y = GET_Y_LPARAM(dwpos);

                        ht.pt = pt;
                        ::ScreenToClient( GetDlgItem( _hSelf, IDC_LSV1 ), &ht.pt);

                        ListView_SubItemHitTest( GetDlgItem( _hSelf, IDC_LSV1 ), &ht);
                        if ( ht.iItem == -1 )
                            break;

                        std::vector<std::wstring> files = getListSelected();
                        if ( files.size() == 0 )
                            break;

                        if ( ht.iSubItem == COL_I )
                        {
                            unstageFileFiles( files );
                            updateListWithDelay();
                        }
                        else if ( ht.iSubItem == COL_W )
                        {
                            addFileFiles( files );
                            updateListWithDelay();
                        }
                        else if ( ht.iSubItem == COL_FILE )
                        {
                            for ( unsigned int i = 0; i < files.size(); i++ )
                            {
                                DWORD fileOrDir = GetFileAttributes( files[i].c_str() );
                                if ( fileOrDir == INVALID_FILE_ATTRIBUTES )
                                    break;
                                else if ( fileOrDir & FILE_ATTRIBUTE_DIRECTORY )
                                {
                                    std::wstring err = TEXT( "" );
                                    err += files[i];
                                    err += TEXT( "\n\nIs a directory.  Continue to open all files?" );
                                    int ret = ( int )::MessageBox( _hSelf, err.c_str(), TEXT( "Continue?" ), ( MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL ) );
                                    if ( ret == IDYES )
                                        SendMessage( _hParent, NPPM_DOOPEN, 0, ( LPARAM )files[i].c_str() );
                                }
                                else
                                    SendMessage( _hParent, NPPM_DOOPEN, 0, ( LPARAM )files[i].c_str() );
                            }
                        }
                        return TRUE;
                    }
                    break;
                }

                case NM_RCLICK:
                {
                    if ( nmhdr->hwndFrom == GetDlgItem( _hSelf, IDC_LSV1 ) )
                    {
                        POINT         pt    = {0};
                        LVHITTESTINFO ht    = {0};
                        DWORD         dwpos = ::GetMessagePos();

                        pt.x = GET_X_LPARAM(dwpos);
                        pt.y = GET_Y_LPARAM(dwpos);

                        ContextMenu cm;
                        std::vector<std::wstring> files = getListSelected();
                        if ( files.size() == 0 )
                            break;

                        cm.SetObjects( files );
                        cm.ShowContextMenu( _hInst, _hParent, _hSelf, pt );
                        return TRUE;
                    }
                    break;
                }

                case TTN_GETDISPINFO: /* TTN_NEEDTEXT */
                {
                    UINT_PTR idButton;
                    LPTOOLTIPTEXT lpttt;

                    lpttt           = (LPTOOLTIPTEXT) lParam;
                    lpttt->hinst    = NULL;
                    idButton        = lpttt->hdr.idFrom;
                    lpttt->lpszText = const_cast<LPTSTR>( GetNameStrFromCmd( idButton ) );
                    return TRUE;
                }
                case LVN_KEYDOWN:
                {
                    LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
                    if ( ( nmhdr->hwndFrom == GetDlgItem( _hSelf, IDC_LSV1 ) ) &&
                       ( ( pnkd->wVKey == VK_RETURN )
                      || ( pnkd->wVKey == VK_SPACE )
                      ) )
                    {
                        gotoFile();
                        return TRUE;
                    }
                    break;
                }

                default :
                    break;
            }

            DockingDlgInterface::run_dlgProc( message, wParam, lParam );
            return FALSE;
        }

        case WM_SIZE:
        case WM_MOVE:
        {
            RECT rc = {0};
            getClientRect( rc );

            ::SetWindowPos( GetDlgItem( _hSelf, IDC_EDT_BRANCH ), NULL,
                            rc.left + 5, rc.top + 120, rc.right - 75, 20,
                            SWP_NOZORDER | SWP_SHOWWINDOW );
            ::SetWindowPos( GetDlgItem( _hSelf, IDB_BTN_BRANCH ), NULL,
                            rc.right - 65, rc.top + 120, 60, 20,
                            SWP_NOZORDER | SWP_SHOWWINDOW );
            ::SetWindowPos( GetDlgItem( _hSelf, IDC_EDT_DIR ), NULL,
                            rc.left + 5, rc.top + 150, rc.right - 10, 20,
                            SWP_NOZORDER | SWP_SHOWWINDOW );
            ::SetWindowPos( GetDlgItem( _hSelf, IDC_LSV1 ), NULL,
                            rc.left + 5, rc.top + 180, rc.right - 10, rc.bottom - 185,
                            SWP_NOZORDER | SWP_SHOWWINDOW );

            SendMessage( GetDlgItem( _hSelf, IDC_LSV1 ), LVM_SETCOLUMNWIDTH, COL_FILE, LVSCW_AUTOSIZE_USEHEADER );

            // redraw();
            break;
        }

        case WM_PAINT:
        {
            ::RedrawWindow( _hSelf, NULL, NULL, TRUE );
            break;
        }

        default :
            return DockingDlgInterface::run_dlgProc( message, wParam, lParam );
    }

    return FALSE;
}
