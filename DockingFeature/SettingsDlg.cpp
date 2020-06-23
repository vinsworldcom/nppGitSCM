#include <windows.h>
#include <shlobj.h>

#include "..\PluginInterface.h"
#include "GitPanelDlg.h"
#include "SettingsDlg.h"
#include "resource.h"

extern HINSTANCE g_hInst;
extern NppData   nppData;
extern TCHAR     g_GitPath[MAX_PATH];
extern TCHAR     g_GitPrompt[MAX_PATH];
extern bool      g_useNppColors;

static int __stdcall BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM, LPARAM pData)
{
    if (uMsg == BFFM_INITIALIZED)
        ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
    return 0;
};

INT_PTR CALLBACK SettingsDlg(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ::SendMessage( GetDlgItem( hWndDlg, IDC_CHK_NPPCOLOR ), BM_SETCHECK,
                   ( LPARAM )( g_useNppColors ? 1 : 0 ), 0 );

    switch(msg)
    {
        case WM_INITDIALOG:
        {
            SendMessage( GetDlgItem( hWndDlg, IDC_EDT_GITPATH ), WM_SETTEXT, 0, ( LPARAM )g_GitPath );
            SendMessage( GetDlgItem( hWndDlg, IDC_EDT_GITPROMPT ), WM_SETTEXT, 0, ( LPARAM )g_GitPrompt );
            return TRUE;
        }

        case WM_CLOSE:
        {
            PostMessage(hWndDlg, WM_DESTROY, 0, 0);
            return TRUE;
        }

        case WM_DESTROY:
        {
            EndDialog(hWndDlg, 0);
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDB_OK:
                    SendMessage( GetDlgItem( hWndDlg, IDC_EDT_GITPROMPT ), WM_GETTEXT, ( MAX_PATH - 1 ), ( LPARAM ) g_GitPrompt );
                    PostMessage(hWndDlg, WM_CLOSE, 0, 0);
                    return TRUE;

                case IDC_CHK_NPPCOLOR :
                {
                    if ( SendMessage( GetDlgItem( hWndDlg, IDC_CHK_NPPCOLOR ), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
                    {
                      SetSysColors();
                      g_useNppColors = false;
                    }
                    else
                    {
                      SetNppColors();
                      g_useNppColors = true;
                    }
                    ChangeColors();
                    return TRUE;
                }

                case IDC_BTN_GITPATH :
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
                        info.hwndOwner          = nppData._nppHandle;
                        info.pidlRoot           = NULL;
                        info.pszDisplayName     = (LPTSTR)new TCHAR[MAX_PATH];
                        info.lpszTitle          = TEXT( "Folder where git.exe is installed" );
                        info.ulFlags            = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON;
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
                            SendMessage( GetDlgItem( hWndDlg, IDC_EDT_GITPATH ), WM_SETTEXT, 0, ( LPARAM )g_GitPath );
//                            )
//                          {
                                // Set edit control to the directory path.
//                              ::SetWindowText(::GetDlgItem(hDialog, IDC_EDT_DIR), g_GitPath);
//                          }
                            pShellMalloc->Free(pidl);
                        }
                        pShellMalloc->Release();
                        delete [] info.pszDisplayName;
                    }
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void doSettings()
{
    DialogBoxParam( g_hInst, MAKEINTRESOURCE( IDD_SETTINGS ), nppData._nppHandle, SettingsDlg, 0 );
}
