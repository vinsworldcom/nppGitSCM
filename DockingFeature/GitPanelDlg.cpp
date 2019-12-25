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

#include <cstdio>
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <array>

extern NppData nppData;
extern bool useTortoise;
extern HWND hDialog;

// std::string gitStatusPorc()
// {
    // std::string result;
    // std::array<char, 128> buffer;
    // const char* cmd = "git status --porcelain";

    // std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);

    // if (!pipe)
        // return result;

    // while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        // result += buffer.data();
    
    // return result;
// }

HWND getCurScintilla()
{
    int which = -1;
    ::SendMessage( nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0,
                   ( LPARAM )&which );
    return ( which == 0 ) ? nppData._scintillaMainHandle :
           nppData._scintillaSecondHandle;
}

void refreshDialog()
{

    HWND hCurScintilla = getCurScintilla();
    TCHAR pathName[MAX_PATH];
    ::SendMessage( hCurScintilla, NPPM_GETCURRENTDIRECTORY, 0, (LPARAM)pathName);

    // std::wstring gitLoc;
    // bool gitInstalled = getGitLocation( gitLoc );
    // if ( !gitInstalled )
        // return;

    const TCHAR *programPath = TEXT("\0"); // Overridden as NULL in Process.cpp
    const TCHAR *pProgramDir = pathName;   // Overridden as NULL in Process.cpp
    // const TCHAR *param       = TEXT("C:\\usr\\bin\\git\\cmd\\git.exe status --porcelain");
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

            SendMessage( GetDlgItem( hDialog, IDC_LST1 ), LB_RESETCONTENT, 0, 0 );
            SendMessage( GetDlgItem( hDialog, IDC_LST1 ), LB_ADDSTRING, 0, ( LPARAM )wide.c_str() );
        }
    }
    else
    {
// TODO:2019-12-24:MVINCENT: currently printing every other character
//                           outputW.c_str() prints Asian characters
//                           UTF8/16 issue
        std::wstring outputW = program.getStderr();
        std::string output(outputW.begin(), outputW.end());
        output.assign(outputW.begin(), outputW.end());

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wide = converter.from_bytes(output.c_str());

//        MessageBox(NULL, wide.c_str(), TEXT("Error"), MB_OK);
        SendMessage( GetDlgItem( hDialog, IDC_LST1 ), LB_RESETCONTENT, 0, 0 );
    }
}

void initDialog()
{
    refreshDialog();    
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
                    refreshDialog();
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
            }
            return FALSE;
        }

        case WM_INITDIALOG :
        {
            initDialog();
            break;
        }

        default :
            return DockingDlgInterface::run_dlgProc( message, wParam, lParam );
    }

    return FALSE;
}

