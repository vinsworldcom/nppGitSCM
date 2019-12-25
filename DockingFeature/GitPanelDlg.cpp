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

#include <locale>
#include <codecvt>

#include <fstream>
#include <vector>

extern NppData nppData;
extern bool useTortoise;
extern HWND hDialog;

HWND getCurScintilla()
{
    int which = -1;
    ::SendMessage( nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0,
                   ( LPARAM )&which );
    return ( which == 0 ) ? nppData._scintillaMainHandle :
           nppData._scintillaSecondHandle;
}

void clearList()
{
    SendMessage( GetDlgItem( hDialog, IDC_LST1 ), LB_RESETCONTENT, 0, 0 );
}

std::vector<std::wstring> split(std::wstring stringToBeSplitted, std::wstring delimeter)
{
    std::vector<std::wstring> splittedString;
    size_t startIndex = 0;
    size_t endIndex = 0;

    while( (endIndex = stringToBeSplitted.find(delimeter, startIndex)) < stringToBeSplitted.size() )
    {
        std::wstring val = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
        splittedString.push_back(val);
        startIndex = endIndex + delimeter.size();
    }

    if(startIndex < stringToBeSplitted.size())
    {
        std::wstring val = stringToBeSplitted.substr(startIndex);
        splittedString.push_back(val);
    }

    return splittedString;
}

void updateList()
{
    // HWND hCurScintilla = getCurScintilla();
    // TCHAR pathName[MAX_PATH];
    // ::SendMessage( hCurScintilla, NPPM_GETCURRENTDIRECTORY, 0, (LPARAM)pathName);

    // std::wstring gitLoc;
    // bool gitInstalled = getGitLocation( gitLoc );
    // if ( !gitInstalled )
        // return;

    const TCHAR *programPath = TEXT("\0");  // Overridden as NULL in Process.cpp
    // const TCHAR *pProgramDir = pathName; // Overridden as NULL in Process.cpp
    const TCHAR *pProgramDir = TEXT("\0");  // Overridden as NULL in Process.cpp
    const TCHAR *param       = TEXT("cmd /c \"git status --porcelain\"");
    const TCHAR *progInput   = TEXT("");
    const TCHAR *progOutput  = TEXT("");

    generic_string progInputStr  = progInput?progInput:TEXT("");
    generic_string progOutputStr = progOutput?progOutput:TEXT("");
    generic_string paramInput    = param;

    // paramInput += gitLoc + TEXT( " status --porcelain\"" );

    Process program(programPath, paramInput.c_str(), pProgramDir, CONSOLE_PROG);
	program.run();

    if (!program.hasStderr())
    {
        const char *pOutput = NULL;
        size_t pOutputLen = 0;
        // If progOutput is defined, then we search the file to read
        if (progOutputStr != TEXT(""))
        {
            if (::PathFileExists(progOutputStr.c_str()))
            {
                // open the file for binary reading
                std::ifstream file;
                file.open(progOutputStr.c_str(), std::ios_base::binary);
                std::vector<byte> fileContent;
                if (file.is_open())
                {
                    // get the length of the file
                    file.seekg(0, std::ios::end);
                    pOutputLen = static_cast<size_t>(file.tellg());
                    file.seekg(0, std::ios::beg);

                    // create a vector to hold all the bytes in the file
                    fileContent.resize(pOutputLen, 0);

                    // read the file
                    file.read(reinterpret_cast<char*>(&fileContent[0]), (std::streamsize)pOutputLen);

                    // close the file
                    file.close();

                    pOutput = reinterpret_cast<const char*>(&fileContent[0]);
                }
                const char errMsg[] = "ERROR: NO FILE CONTENT";
                if (!pOutput || !(pOutput[0]))
                {
                    pOutput = errMsg;
                    pOutputLen = strlen(errMsg);
                }
            }
            else
            {
                ::MessageBox(NULL, TEXT("The file is invalid"), progOutputStr.c_str(), MB_OK);
            }
        }
        // otherwise, we look in stdout
        else if ( program.hasStdout() )
        {
            std::wstring outputW = program.getStdout();
            std::string output(outputW.begin(), outputW.end());
            output.assign(outputW.begin(), outputW.end());

            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wide = converter.from_bytes(output.c_str());

            clearList();
            std::vector<std::wstring> splittedStrings_2 = split(wide, TEXT("\n"));
            for(unsigned int i = 0; i < splittedStrings_2.size() ; i++)
                SendMessage( GetDlgItem( hDialog, IDC_LST1 ), LB_ADDSTRING, i, ( LPARAM )splittedStrings_2[i].c_str() );
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
    SendMessage( GetDlgItem( hDialog, IDC_LST1 ), LB_SETHORIZONTALEXTENT, 165, 0 );
    updateList();
}

INT_PTR CALLBACK DemoDlg::run_dlgProc( UINT message, WPARAM wParam,
                                       LPARAM lParam )
{

    ::SendMessage( GetDlgItem( hDialog, IDC_CHK1 ), BM_SETCHECK, ( LPARAM )( useTortoise ? 1 : 0 ), 0 );

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
                    commitAll();
                    return TRUE;
                }
                case IDC_BTN5 :
                {
                    addFile();
                    return TRUE;
                }
                case IDC_BTN6 :
                {
                    diffFile();
                    return TRUE;
                }
                case IDC_BTN7 :
                {
                    unstageFile();
                    return TRUE;
                }
                case IDC_BTN8 :
                {
                    revertFile();
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

// TODO:2019-12-25:MVINCENT: Weird focus issue when clicking in the ListBox
//                           Even with this disabled
                // case IDC_LST1 :
                // {
                    // switch (HIWORD(wParam))
                    // {
                        // case LBN_SETFOCUS :
                        // {
                            // updateList();
                            // return TRUE;
                        // }
                        // case LBN_KILLFOCUS :
                        // {
                            // clearList();
                            // return TRUE;
                        // }
                    // }
                    // return FALSE;
                // }
            }
        }

        case WM_INITDIALOG :
        {
            initDialog();
            return TRUE;
        }

        default :
            return DockingDlgInterface::run_dlgProc( message, wParam, lParam );
    }

    // return FALSE;
}

