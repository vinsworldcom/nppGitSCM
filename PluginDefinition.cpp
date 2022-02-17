//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
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

#include "DockingFeature/SettingsDlg.h"
#include "PluginDefinition.h"
#include "resource.h"
#include "DockingFeature/GitPanelDlg.h"
#include "menuCmdID.h"
#include "stdafx.h"

#include <shlwapi.h>
#include <string>
#include <vector>

const TCHAR configFileName[]     = TEXT( "GitSCM.ini" );
const TCHAR sectionName[]        = TEXT( "Git" );
const TCHAR iniKeyTortoise[]     = TEXT( "Tortoise" );
const TCHAR iniKeyGitPath[]      = TEXT( "GitPath" );
const TCHAR iniKeyGitPrompt[]    = TEXT( "GitPrompt" );
const TCHAR iniKeyUseNppColors[] = TEXT( "UseNppColors" );
const TCHAR iniKeyRaisePanel[]   = TEXT( "RaisePanelorToggle" );
const TCHAR iniKeyDebug[]        = TEXT( "Debug" );

DemoDlg _gitPanel;

toolbarIcons g_TBGit;

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData   nppData;
HINSTANCE g_hInst;

TCHAR iniFilePath[MAX_PATH];
TCHAR g_GitPath[MAX_PATH];
TCHAR g_GitPrompt[MAX_PATH];
bool  g_useTortoise  = false;
bool  g_NppReady     = false;
bool  g_useNppColors = false;
bool  g_RaisePanel   = false;
bool  g_Debug        = false;

std::wstring g_tortoiseLoc;

//
// Initialize your plugin data here
// It will be called while plugin loading
void pluginInit( HANDLE hModule )
{
    // Initialize dockable dialog
    _gitPanel.init( ( HINSTANCE )hModule, NULL );
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
    ::WritePrivateProfileString( sectionName, iniKeyTortoise,
                                 g_useTortoise ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
    ::WritePrivateProfileString( sectionName, iniKeyGitPath, 
                                 g_GitPath, iniFilePath);
    ::WritePrivateProfileString( sectionName, iniKeyGitPrompt, 
                                 g_GitPrompt, iniFilePath);
    ::WritePrivateProfileString( sectionName, iniKeyUseNppColors,
                                 g_useNppColors ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
    ::WritePrivateProfileString( sectionName, iniKeyRaisePanel,
                                 g_RaisePanel ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );

    if (g_TBGit.hToolbarBmp) {
        ::DeleteObject(g_TBGit.hToolbarBmp);
        g_TBGit.hToolbarBmp = nullptr;
    }
    if (g_TBGit.hToolbarIcon) {
        ::DestroyIcon(g_TBGit.hToolbarIcon);
        g_TBGit.hToolbarIcon = nullptr;
    }
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
    //
    // Firstly we get the parameters from your plugin config file (if any)
    //

    // get path of plugin configuration
    ::SendMessage( nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH,
                   ( LPARAM )iniFilePath );

    // if config path doesn't exist, we create it
    if ( PathFileExists( iniFilePath ) == FALSE )
        ::CreateDirectory( iniFilePath, NULL );

    // make your plugin config file full file path name
    PathAppend( iniFilePath, configFileName );

    // get the parameter value from plugin config
    g_useTortoise = ::GetPrivateProfileInt( sectionName, iniKeyTortoise,
                                            0, iniFilePath );
    ::GetPrivateProfileString( sectionName, iniKeyGitPath, TEXT(""), 
                               g_GitPath, MAX_PATH, iniFilePath );
    ::GetPrivateProfileString( sectionName, iniKeyGitPrompt, TEXT("powershell.exe"), 
                               g_GitPrompt, MAX_PATH, iniFilePath );
    g_useNppColors = ::GetPrivateProfileInt( sectionName, iniKeyUseNppColors,
                                             0, iniFilePath );
    g_Debug = ::GetPrivateProfileInt( sectionName, iniKeyDebug,
                                             0, iniFilePath );
    g_RaisePanel = ::GetPrivateProfileInt( sectionName, iniKeyRaisePanel,
                                                0, iniFilePath );

    if ( g_useTortoise )
    {
        if ( ! getTortoiseLocation( g_tortoiseLoc ) )
            g_useTortoise = false;
    }

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );

    setCommand( 0,  TEXT( "Git &GUI" ),      gitGui, NULL, false );
    setCommand( 1,  TEXT( "GiT&k" ),         giTk, NULL, false );
    setCommand( 2,  TEXT( "Git Pro&mpt" ),   gitPrompt, NULL, false );
    setCommand( 3,  TEXT( "-SEPARATOR-" ),   NULL, NULL, false );
    setCommand( DOCKABLE_INDEX, TEXT( "Git Docking Panel" ), DockableDlg, NULL,
                false );
    setCommand( 5,  TEXT( "-SEPARATOR-" ),   NULL, NULL, false );
    setCommand( 6,  TEXT( "&Add File" ),     addFile, NULL, false );
    setCommand( 7,  TEXT( "&Unstage File" ), unstageFile, NULL, false );
    setCommand( 8,  TEXT( "&Restore File" ), restoreFile, NULL, false );
    setCommand( 9,  TEXT( "&Diff File" ),    diffFile, NULL, false );
    setCommand( 10, TEXT( "&Log File" ),     logFile, NULL, false );
    setCommand( 11, TEXT( "&Blame File" ),   blameFile, NULL, false );
    setCommand( 12, TEXT( "-SEPARATOR-" ),   NULL, NULL, false );
    setCommand( 13, TEXT( "&Pull" ),         pullFile, NULL, false );
    setCommand( 14, TEXT( "&Status" ),       statusAll, NULL, false );
    setCommand( 15, TEXT( "Bra&nch/Checkout" ), branchFile, NULL, false );
    setCommand( 16, TEXT( "&Commit" ),       commitAll, NULL, false );
    setCommand( 17, TEXT( "Pus&h" ),         pushFile, NULL, false );
    setCommand( 18, TEXT( "-SEPARATOR-" ),   NULL, NULL, false );
    setCommand( TORTOISE_INDEX, TEXT( "Use &TortoiseGit" ), doTortoise, NULL,
                g_useTortoise ? true : false );
    setCommand( 20, TEXT( "S&ettings" ),      doSettings, NULL, false );
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
    // Don't forget to deallocate your shortcut here
}

//
// This function help you to initialize your plugin commands
//
bool setCommand( size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc,
                 ShortcutKey *sk, bool check0nInit )
{
    if ( index >= nbFunc )
        return false;

    if ( !pFunc )
        return false;

    lstrcpy( funcItem[index]._itemName, cmdName );
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//

///
/// Gets the path to the current file.
///
/// @return Current file path.
///
HWND getCurScintilla()
{
    int which = -1;
    ::SendMessage( nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0,
                   ( LPARAM )&which );
    return ( which == 0 ) ? nppData._scintillaMainHandle :
           nppData._scintillaSecondHandle;
}

std::wstring getCurrentFile()
{
    TCHAR path[MAX_PATH];
    ::SendMessage( nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH,
                   ( LPARAM )path );

    return std::wstring( path );
}

///
/// Gets the path to the current file's directory.
///
/// @return Current file's directory path.
///
std::wstring getCurrentFileDirectory()
{
    TCHAR path[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH,
        (LPARAM)path);

    return std::wstring(path);
}

///
/// Gets the path to the TortioseGit executable from the registry.
///
/// @param loc [out] Location of Tortoise executable
///
/// @return Whether or not the path was successfully retrieved.
///         If false, Tortoise is most likely not installed.
bool getTortoiseLocation( std::wstring &loc )
{
    HKEY hKey;

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT( "Software\\TortoiseGit" ), 0,
                       KEY_READ | KEY_WOW64_64KEY, &hKey ) != ERROR_SUCCESS )
        return false;

    TCHAR procPath[MAX_PATH];
    DWORD length = MAX_PATH;

    // Modified Douglas Phillips <doug@sbscomp.com> 2008-12-29 to
    // support 32-bit and Non-Vista operating Systems.
    if ( RegQueryValueEx(
                hKey,
                TEXT( "ProcPath" ),
                NULL,
                NULL,
                ( LPBYTE )procPath,
                &length ) != ERROR_SUCCESS )
        return false;

    loc = procPath;
    return true;
}

std::wstring getGitLocation()
{
    std::wstring gp = g_GitPath;
    if ( gp.size() == 0 )
        return TEXT("");
    else
    {
        gp += TEXT("\\");
        return gp;
    }
}

///
/// Launches Git using the supplied command
///
/// @param Command line string to execute.
///
/// @return Whether or not Git could be launched.
///
bool launchGit( std::wstring &command )
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset( &si, 0, sizeof( si ) );
    memset( &pi, 0, sizeof( pi ) );
    si.cb = sizeof( si );

    if ( g_Debug )
        OutputDebugString( command.c_str() );
    return CreateProcess(
               NULL,
               const_cast<LPWSTR>( command.c_str() ),
               NULL,
               NULL,
               FALSE,
               CREATE_DEFAULT_ERROR_MODE,
               NULL,
               const_cast<LPCWSTR>( getCurrentFileDirectory().c_str() ),
               &si,
               &pi ) != 0;
}

///
/// Builds and executes command line string to send to CreateProcess
///
/// @param cmd Command name to execute.
/// @param all Execute command on all files, or just the current file.
/// @param ignoreFiles No files
/// @param pause Pause after command.
///
void ExecGitCommand(
    const std::wstring &cmd, 
    std::vector<std::wstring> files,
    bool ignoreFiles = false, 
    bool pause = true )
{
    std::wstring command = TEXT( "cmd /d/c \"\"" );
    command += getGitLocation();
    command += TEXT( "git" );
    command += cmd + TEXT( " " );

    if ( !ignoreFiles )
    {
        for ( std::vector<std::wstring>::iterator itr = files.begin();
                itr != files.end(); itr++ )
        {
            command += ( *itr );

            if ( itr != files.end() - 1 )
                command += TEXT( " " );
        }
    }

    if ( pause )
        command += TEXT( " & pause" );

    command += TEXT( "\"" );

    if ( !launchGit( command ) )
        MessageBox( nppData._nppHandle, TEXT( "Could not launch Git." ),
                    TEXT( "Failed" ), ( MB_OK | MB_ICONWARNING | MB_APPLMODAL ) );

    updatePanel();
}

///
/// Builds and executes command line string to send to CreateProcess
/// See http://tortoisesvn.net/docs/release/TortoiseSVN_en/tsvn-automation.html
/// for TortoiseSVN command line parameters.
///
/// @param cmd Command name to execute.
/// @param all Execute command on all files, or just the current file.
///
void ExecTortoiseCommand(
    const std::wstring &cmd, 
    std::vector<std::wstring> files,
    bool ignoreFiles = false, 
    bool pause = true )
{
    std::wstring command = g_tortoiseLoc;
    command += TEXT( " /command:" ) + cmd + TEXT( " /path:\"" );

    if ( !ignoreFiles )
    {
        for ( std::vector<std::wstring>::iterator itr = files.begin();
                itr != files.end(); itr++ )
        {
            command += ( *itr );

            if ( itr != files.end() - 1 )
                command += TEXT( "*" );
        }
    }
    else
        command += TEXT( "*" );

    if ( pause )
        command += TEXT( "\" /closeonend:0" );
    else
        command += TEXT( "\" /closeonend:2" );

    if ( !launchGit( command ) )
        MessageBox( nppData._nppHandle, TEXT( "Could not launch TortoiseGit." ),
                    TEXT( "Failed" ), ( MB_OK | MB_ICONWARNING | MB_APPLMODAL ) );

    updatePanel();
}

////////////////////////////////////////////////////////////////////////////
///
/// Execution commands:
///
void updatePanelLoc()
{
    if ( _gitPanel.isVisible() )
    {
        std::wstring temp;
        _gitPanel.updateLoc( temp );
        _gitPanel.updateListWithDelay();
    }
}

void updatePanel()
{
    if ( _gitPanel.isVisible() )
        _gitPanel.updateList();
}

void gitPrompt()
{
    std::wstring pathName;
    if ( _gitPanel.isVisible() )
        _gitPanel.updateLoc( pathName );
    else
        pathName = getCurrentFileDirectory();

    ShellExecute( nppData._nppHandle, TEXT("open"), g_GitPrompt, NULL, pathName.c_str(), SW_SHOW );
}

void gitGui()
{
    std::vector<std::wstring> files = {};
    gitGuiFiles( files );
}
void gitGuiFiles( std::vector<std::wstring> files )
{
    ExecGitCommand( TEXT( "-gui\"" ), files, true, false );
}

void giTk()
{
    std::vector<std::wstring> files = {};
    giTkFiles( files );
}
void giTkFiles( std::vector<std::wstring> files = {} )
{
    ExecGitCommand( TEXT( "k\"" ), files, true, false );
}

void statusAll()
{
    std::vector<std::wstring> files = {};
    statusAllFiles( files );
}
void statusAllFiles( std::vector<std::wstring> files = {} )
{
    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "repostatus" ), files, true , true);
    else
        ExecGitCommand( TEXT( "\" status" ), files, true, true);
}

void commitAll()
{
    std::vector<std::wstring> files = {};
    commitAllFiles( files );
}
void commitAllFiles( std::vector<std::wstring> files = {} )
{
    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "commit" ), files, true, true );
    else
        ExecGitCommand( TEXT( "\" commit" ), files, true, true );
}

void addFile()
{
    std::vector<std::wstring> files = {};
    addFileFiles( files );
}
void addFileFiles( std::vector<std::wstring> files = {} )
{
    if ( files.size() == 0 )
        files.push_back( getCurrentFile() );

    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "add" ), files, false, false );
    else
        ExecGitCommand( TEXT( "\" add" ), files, false, false );
}

void diffFile()
{
    std::vector<std::wstring> files = {};
    diffFileFiles( files );
}
void diffFileFiles( std::vector<std::wstring> files = {} )
{
    if ( files.size() == 0 )
        files.push_back( getCurrentFile() );

    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "diff" ), files, false, true );
    else
        ExecGitCommand( TEXT( "\" diff" ), files, false, true );
}

void unstageFile()
{
    std::vector<std::wstring> files = {};
    unstageFileFiles( files );
}
void unstageFileFiles( std::vector<std::wstring> files = {} )
{
    if ( files.size() == 0 )
        files.push_back( getCurrentFile() );

    ExecGitCommand( TEXT( "\" reset HEAD" ), files, false, false );
}

void restoreFile()
{
    std::vector<std::wstring> files = {};
    restoreFileFiles( files );
}
void restoreFileFiles( std::vector<std::wstring> files = {} )
{
    if ( files.size() == 0 )
        files.push_back( getCurrentFile() );

    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "revert" ), files, false, false );
    else
        ExecGitCommand( TEXT( "\" checkout --" ), files, false, false );
}

void logFile()
{
    std::vector<std::wstring> files = {};
    logFileFiles( files );
}
void logFileFiles( std::vector<std::wstring> files = {} )
{
    if ( files.size() == 0 )
        files.push_back( getCurrentFile() );

    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "log" ), files, false, true );
    else
        ExecGitCommand( TEXT( "\" log" ), files, false, true );
}

void blameFile()
{
    std::vector<std::wstring> files = {};
    blameFileFiles( files );
}
void blameFileFiles( std::vector<std::wstring> files = {} )
{
    if ( files.size() == 0 )
        files.push_back( getCurrentFile() );

    if ( g_useTortoise )
    {
        Sci_Position pos = (Sci_Position)::SendMessage( getCurScintilla(), SCI_GETCURRENTPOS, 0, 0 );
        Sci_Position line = (Sci_Position)::SendMessage( getCurScintilla(), SCI_LINEFROMPOSITION, pos, 0 );
        std::wstring blame = TEXT( "blame /line:" );
        blame += std::to_wstring( line + 1 );
        ExecTortoiseCommand( blame, files, false, true );
    }
    else
        ExecGitCommand( TEXT( "\" blame" ), files, false, true );
}

void branchFile()
{
    std::vector<std::wstring> files = {};

    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "switch" ), files, true, true );
    else
    {
        MessageBox( nppData._nppHandle, TEXT("Only supported with TortioseGit"), TEXT("Not Implemented"), MB_OK );
        // files.push_back( getBranchDlg() );
        // branchFileFiles( files );
    }
}
void branchFileFiles( std::vector<std::wstring> files = {} )
{
    ExecGitCommand( TEXT( "\" branch" ), files, false, false );
    ExecGitCommand( TEXT( "\" checkout" ), files, false, true );
}

void pullFile()
{
    std::vector<std::wstring> files = {};
    pullFileFiles( files );
}
void pullFileFiles( std::vector<std::wstring> files = {} )
{
    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "pull" ), files, true, true );
    else
        ExecGitCommand( TEXT( "\" pull" ), files, true, true );
}

void pushFile()
{
    std::vector<std::wstring> files = {};
    pushFileFiles( files );
}
void pushFileFiles( std::vector<std::wstring> files = {} )
{
    if ( g_useTortoise )
        ExecTortoiseCommand( TEXT( "push" ), files, true, true );
    else
        ExecGitCommand( TEXT( "\" push" ), files, true, true );
}

void doTortoise()
{
    // UINT state = ::GetMenuState( ::GetMenu( nppData._nppHandle ),
                                 // funcItem[TORTOISE_INDEX]._cmdID,
                                 // MF_BYCOMMAND );

    if ( g_useTortoise )
    {
        g_useTortoise = false;
        ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                       funcItem[TORTOISE_INDEX]._cmdID, MF_UNCHECKED );
    }
    else
    {
        if ( ! getTortoiseLocation( g_tortoiseLoc ) )
        {
            MessageBox( nppData._nppHandle, TEXT( "Could not locate TortoiseGit" ),
                        TEXT( "Not Found" ), ( MB_OK | MB_ICONWARNING | MB_APPLMODAL ) );
            g_useTortoise = false;
            return;
        }
        else
        {
            g_useTortoise = true;
            ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                           funcItem[TORTOISE_INDEX]._cmdID, MF_CHECKED );
        }
    }
}

////////////////////////////////////////////////////////////////////////////
///
/// Dockable dialog:
///
void DockableDlg()
{
    _gitPanel.setParent( nppData._nppHandle );
    tTbData data = {0};

    if ( !_gitPanel.isCreated() )
    {
        _gitPanel.create( &data );

        // define the default docking behaviour
        data.uMask = DWS_DF_CONT_LEFT | DWS_ICONTAB;

        data.hIconTab = ( HICON )::LoadImage( _gitPanel.getHinst(),
                                              MAKEINTRESOURCE( IDI_PLUGINGITPANEL ), IMAGE_ICON, 0, 0,
                                              LR_LOADTRANSPARENT );
        data.pszModuleName = _gitPanel.getPluginFileName();

        // the dlgDlg should be the index of funcItem where the current function pointer is
        // in this case is DOCKABLE_INDEX
        data.dlgID = DOCKABLE_INDEX;
        ::SendMessage( nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0,
                       ( LPARAM )&data );

        _gitPanel.setClosed(true);
    }

    if ( _gitPanel.isClosed() || g_RaisePanel )
    {
        _gitPanel.display();
        _gitPanel.setClosed(false);
        ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                       funcItem[DOCKABLE_INDEX]._cmdID, MF_CHECKED );
    }
    else
    {
        _gitPanel.display( false );
        _gitPanel.setClosed(true);
        ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                       funcItem[DOCKABLE_INDEX]._cmdID, MF_UNCHECKED );
    }
}
