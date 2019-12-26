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

#include <locale>
#include <codecvt>

#include <fstream>
#include <vector>

extern NppData nppData;
extern bool useTortoise;
extern bool g_NppReady;
extern HWND hDialog;

LVITEM LvItem;
LVCOLUMN LvCol;

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

void updateLoc()
{
    TCHAR pathName[MAX_PATH];
    SendMessage( nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, ( LPARAM )pathName );
    SendMessage( GetDlgItem( hDialog, IDC_EDT1 ), WM_SETTEXT, 0, ( LPARAM )pathName );
}

void updateList()
{
    if ( ! g_NppReady )
        return;

    TCHAR pathName[MAX_PATH];
    SendMessage( nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, ( LPARAM )pathName );
    SendMessage( GetDlgItem( hDialog, IDC_EDT1 ), WM_SETTEXT, 0, ( LPARAM )pathName );

    // std::wstring gitLoc;
    // bool gitInstalled = getGitLocation( gitLoc );
    // if ( !gitInstalled )
    // return;

    const TCHAR *programPath = TEXT( "\0" ); // Overridden as NULL in Process.cpp
    const TCHAR *pProgramDir = pathName;     // Overridden as NULL in Process.cpp
    // const TCHAR *pProgramDir = TEXT( "\0" ); // Overridden as NULL in Process.cpp
    const TCHAR *param       = TEXT( "cmd /c \"git status --porcelain\"" );
    const TCHAR *progInput   = TEXT( "" );
    const TCHAR *progOutput  = TEXT( "" );

    generic_string progInputStr  = progInput ? progInput : TEXT( "" );
    generic_string progOutputStr = progOutput ? progOutput : TEXT( "" );
    generic_string paramInput    = param;

    // paramInput += gitLoc + TEXT( " status --porcelain\"" );

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
                ::MessageBox( NULL, TEXT( "The file is invalid" ), progOutputStr.c_str(),
                              MB_OK );
        }
        // otherwise, we look in stdout
        else if ( program.hasStdout() )
        {
            std::wstring outputW = program.getStdout();
            std::string output( outputW.begin(), outputW.end() );
            output.assign( outputW.begin(), outputW.end() );

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wide = converter.from_bytes( output.c_str() );

            clearList();
            std::vector<std::wstring> splittedStrings = split( wide, TEXT( "\n" ) );

            for ( unsigned int i = 0; i < splittedStrings.size() ; i++ )
            {
                // https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
                memset( &LvItem, 0, sizeof( LvItem ) ); // Zero struct's Members
                LvItem.mask       = LVIF_TEXT;    // Text Style
                LvItem.cchTextMax = MAX_PATH;     // Max size of text
                LvItem.iItem      = i;            // choose item

                LvItem.iSubItem   = 0;            // Put in first coluom
                std::wstring strI = splittedStrings[i].substr(0, 1);
                LvItem.pszText    = const_cast<LPWSTR>( strI.c_str() );
                SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_INSERTITEM, 0,
                             ( LPARAM )&LvItem );

                LvItem.iSubItem   = 1;            // Put in second coluom
                std::wstring strW = splittedStrings[i].substr(1, 1);
                LvItem.pszText    = const_cast<LPWSTR>( strW.c_str() );
                SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_SETITEM, 0,
                             ( LPARAM )&LvItem );

                LvItem.iSubItem   = 2;            // Put in third coluom
                splittedStrings[i].erase(0, 3);
                LvItem.pszText    =  const_cast<LPWSTR>( splittedStrings[i].c_str() );
                SendMessage( GetDlgItem( hDialog, IDC_LSV1 ), LVM_SETITEM, 0,
                             ( LPARAM )&LvItem );
            }
        }
    }
    else
    {
// TODO:2019-12-24:MVINCENT: currently printing every other character
//                           outputW.c_str() prints Asian characters
//                           UTF8/16 issue
        // std::wstring outputW = program.getStderr();
        // std::string output(outputW.begin(), outputW.end());
        // output.assign(outputW.begin(), outputW.end());

        // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        // std::wstring wide = converter.from_bytes(output.c_str());

//        MessageBox(NULL, wide.c_str(), TEXT("Error"), MB_OK);
        clearList();
    }
}

void initDialog()
{
    HWND hList = GetDlgItem( hDialog, IDC_LSV1 );

    // https://www.codeproject.com/Articles/2890/Using-ListView-control-under-Win32-API
    SendMessage( hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT );

    memset( &LvCol, 0, sizeof( LvCol ) );            // Zero Members
    LvCol.mask    = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM; // Type of mask

    // Column I and W are Index and Working from:
    // https://git-scm.com/docs/git-status
    LvCol.cx      = 25;                                    // width between each coloum
    LvCol.pszText = TEXT( "I" );                           // First Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, 0, ( LPARAM )&LvCol );

    LvCol.cx      = 25;                                    // width between each coloum
    LvCol.pszText = TEXT( "W" );                           // First Header Text
    SendMessage( hList, LVM_INSERTCOLUMN, 1, ( LPARAM )&LvCol );
    
    LvCol.cx      = 170;                                   // width of column
    LvCol.pszText = TEXT( "File" );
    SendMessage( hList, LVM_INSERTCOLUMN, 2, ( LPARAM )&LvCol );

    SendMessage( hList, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE_USEHEADER );
    updateList();
}

INT_PTR CALLBACK DemoDlg::run_dlgProc( UINT message, WPARAM wParam,
                                       LPARAM lParam )
{

    ::SendMessage( GetDlgItem( hDialog, IDC_CHK1 ), BM_SETCHECK,
                   ( LPARAM )( useTortoise ? 1 : 0 ), 0 );

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

                case MAKELONG( IDC_EDT1, EN_SETFOCUS ) :
                {
                    updateList();
                    return FALSE;
                }

            }
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

