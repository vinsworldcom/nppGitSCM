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

#include "PluginDefinition.h"
#include "menuCmdID.h"

#include "stdafx.h"
#include <string>
#include <vector>
#include <shlwapi.h>
#include "DockingFeature/GitPanelDlg.h"

const TCHAR sectionName[] = TEXT( "Git" );
const TCHAR keyName[] = TEXT( "Tortoise" );
const TCHAR configFileName[] = TEXT( "GitSCM.ini" );

DemoDlg _gitPanel;

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

TCHAR iniFilePath[MAX_PATH];
bool useTortoise = false;

#define DOCKABLE_INDEX 3
#define TORTOISE_INDEX 14

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
    ::WritePrivateProfileString( sectionName, keyName,
                                 useTortoise ? TEXT( "1" ) : TEXT( "0" ), iniFilePath );
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
    useTortoise = (::GetPrivateProfileInt( sectionName, keyName, 0,
                                          iniFilePath ) ? true : false);

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

    setCommand( 0, TEXT( "Git &GUI" ), gitGui, NULL, false );
    setCommand( 1, TEXT( "GiT&k" ), giTk, NULL, false );
    setCommand( 2, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( DOCKABLE_INDEX, TEXT( "Git Docking &Panel" ), DockableDlg, NULL,
                false );
    setCommand( 4, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( 5, TEXT( "&Status" ), statusAll, NULL, false );
    setCommand( 6, TEXT( "&Commit" ), commitAll, NULL, false );
    setCommand( 7, TEXT( "&Add File" ), addFile, NULL, false );
    setCommand( 8, TEXT( "&Diff File" ), diffFile, NULL, false );
    setCommand( 9, TEXT( "&Unstage File" ), unstageFile, NULL, false );
    setCommand( 10, TEXT( "&Revert File" ), revertFile, NULL, false );
    setCommand( 11, TEXT( "&Log File" ), logFile, NULL, false );
    setCommand( 12, TEXT( "&Blame File" ), blameFile, NULL, false );
    setCommand( 13, TEXT( "-SEPARATOR-" ), NULL, NULL, false );
    setCommand( TORTOISE_INDEX, TEXT( "Use &TortoiseGit" ), doTortoise, NULL,
                useTortoise ? true : false );
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
std::wstring getCurrentFile()
{
    TCHAR path[MAX_PATH];
    ::SendMessage( nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH,
                   ( LPARAM )path );

    return std::wstring( path );
}

///
/// Gets the full path to every opened file.
///
/// @param numFiles [out] Returns the number of opened files.
///
/// @return Vector of filenames.
///
std::vector<std::wstring> getAllFiles()
{
    //get the number of opened files
    //notepad++ always returns an extra "new 1", remove it by subtracting 1
    int numFiles = ( int )( ::SendMessage( nppData._nppHandle,
                                           NPPM_GETNBOPENFILES, 0,
                                           ALL_OPEN_FILES ) ) - 1;

    //allocate memory to hold filenames
    TCHAR **files = new TCHAR*[numFiles];

    for ( int i = 0; i < numFiles; i++ )
        files[i] = new TCHAR[MAX_PATH];

    //get filenames
    ::SendMessage( nppData._nppHandle, NPPM_GETOPENFILENAMES, ( WPARAM )files,
                   ( LPARAM )numFiles );

    std::vector<std::wstring> filePaths;

    for ( int i = 0; i < numFiles; i++ )
        filePaths.push_back( files[i] );

    return filePaths;
}

///
/// Gets the path to the Git executable.
///
/// @param loc [out] Location of Git executable
///
/// @return Whether or not the path was successfully retrieved.
///         If false, Git is most likely not installed.
bool getGitLocation( std::wstring &loc )
{
    TCHAR procPath[MAX_PATH];
    wcscpy( procPath, TEXT( "cmd /c \"git" ) );
    loc = loc.append( procPath );
    return true;
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

    loc = loc.append( procPath );
    return true;
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

    return CreateProcess(
               NULL,
               const_cast<LPWSTR>( command.c_str() ),
               NULL,
               NULL,
               FALSE,
               CREATE_DEFAULT_ERROR_MODE,
               NULL,
               NULL,
               &si,
               &pi ) != 0;
}

// Used for debug printing to MessageBox
std::string ws2s( const std::wstring &wstr )
{
    int size_needed = WideCharToMultiByte( CP_ACP, 0, wstr.c_str(),
                                           int( wstr.length() + 1 ), 0, 0, 0, 0 );
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte( CP_ACP, 0, wstr.c_str(), int( wstr.length() + 1 ),
                         &strTo[0], size_needed, 0, 0 );
    return strTo;
}

///
/// Builds and executes command line string to send to CreateProcess
///
/// @param cmd Command name to execute.
/// @param all Execute command on all files, or just the current file.
/// @param ALL No files
/// @param pause Pause after command.
///
void ExecGitCommand( const std::wstring &cmd, bool all = false,
                     bool ALL = false, bool pause = true )
{
    std::wstring gitLoc;
    bool gitInstalled = getGitLocation( gitLoc );

    if ( !gitInstalled )
    {
        MessageBox( NULL, TEXT( "Could not locate Git" ),
                    TEXT( "Update Failed" ), 0 );
        return;
    }

    std::vector<std::wstring> files;

    if ( all )
        files = getAllFiles();
    else
        files.push_back( getCurrentFile() );

    std::wstring command = gitLoc;
    command += cmd + TEXT( " " );

    if ( !ALL )
    {
        for ( std::vector<std::wstring>::iterator itr = files.begin();
                itr != files.end(); itr++ )
        {
            command += ( *itr );

            if ( itr != files.end() - 1 )
                command += TEXT( "*" );
        }
    }

    if ( pause )
        command += TEXT( " && pause" );

    command += TEXT( "\"" );

    // Debug
    //MessageBoxA(NULL, ws2s(command).c_str(), "Command to run", MB_OK);

    if ( !launchGit( command ) )
        MessageBox( NULL, TEXT( "Could not launch Git." ),
                    TEXT( "Update Failed" ), 0 );
}

///
/// Builds and executes command line string to send to CreateProcess
/// See http://tortoisesvn.net/docs/release/TortoiseSVN_en/tsvn-automation.html
/// for TortoiseSVN command line parameters.
///
/// @param cmd Command name to execute.
/// @param all Execute command on all files, or just the current file.
///
void ExecTortoiseCommand( const std::wstring &cmd, bool all = false,
                          bool ALL = false )
{
    std::wstring tortoiseLoc;
    bool tortoiseInstalled = getTortoiseLocation( tortoiseLoc );

    if ( !tortoiseInstalled )
    {
        MessageBox( NULL, TEXT( "Could not locate TortoiseGit" ),
                    TEXT( "Update Failed" ), 0 );
        return;
    }

    std::vector<std::wstring> files;

    if ( all )
        files = getAllFiles();
    else
        files.push_back( getCurrentFile() );

    std::wstring command = tortoiseLoc;
    command += TEXT( " /command:" ) + cmd + TEXT( " /path:\"" );

    if ( !ALL )
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

    command += TEXT( "\" /closeonend:0" );

    if ( !launchGit( command ) )
        MessageBox( NULL, TEXT( "Could not launch TortoiseGit." ),
                    TEXT( "Update Failed" ), 0 );
}

////////////////////////////////////////////////////////////////////////////
///
/// Execution commands:
///
void gitGui()
{
    ExecGitCommand( TEXT( "-gui" ), false, true, false );
}

void giTk()
{
    ExecGitCommand( TEXT( "k" ), false, true, false );
}

void statusAll()
{
    if ( useTortoise )
        ExecTortoiseCommand( TEXT( "repostatus" ), false, true );
    else
        ExecGitCommand( TEXT( " status" ), false, true );
}

void commitAll()
{
    if ( useTortoise )
        ExecTortoiseCommand( TEXT( "commit" ), false, true );
    else
        ExecGitCommand( TEXT( " commit" ), false, true );
}

void addFile()
{
    if ( useTortoise )
        ExecTortoiseCommand( TEXT( "add" ) );
    else
        ExecGitCommand( TEXT( " add" ), false, false, false );
}

void diffFile()
{
    if ( useTortoise )
        ExecTortoiseCommand( TEXT( "diff" ) );
    else
        ExecGitCommand( TEXT( " diff" ) );
}

void unstageFile()
{
    ExecGitCommand( TEXT( " reset HEAD" ), false, false, false );
}

void revertFile()
{
    if ( useTortoise )
        ExecTortoiseCommand( TEXT( "revert" ) );
    else
        ExecGitCommand( TEXT( " checkout --" ), false, false, false );
}

void logFile()
{
    if ( useTortoise )
        ExecTortoiseCommand( TEXT( "log" ) );
    else
        ExecGitCommand( TEXT( " log" ) );

}

void blameFile()
{
    if ( useTortoise )
        ExecTortoiseCommand( TEXT( "blame" ) );
    else
        ExecGitCommand( TEXT( " blame" ) );
}

void doTortoise()
{
    UINT state = ::GetMenuState( ::GetMenu( nppData._nppHandle ),
                                 funcItem[TORTOISE_INDEX]._cmdID,
                                 MF_BYCOMMAND );

    if ( state & MF_CHECKED )
        useTortoise = 0;
    else
        useTortoise = 1;

    ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                   funcItem[TORTOISE_INDEX]._cmdID, !( state & MF_CHECKED ) );
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
                                              LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT );
        data.pszModuleName = _gitPanel.getPluginFileName();

        // the dlgDlg should be the index of funcItem where the current function pointer is
        // in this case is DOCKABLE_INDEX
        data.dlgID = DOCKABLE_INDEX;
        ::SendMessage( nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0,
                       ( LPARAM )&data );
    }

    UINT state = ::GetMenuState( ::GetMenu( nppData._nppHandle ),
                                 funcItem[DOCKABLE_INDEX]._cmdID, MF_BYCOMMAND );

    if ( state & MF_CHECKED )
        _gitPanel.display( false );
    else
        _gitPanel.display();

    ::SendMessage( nppData._nppHandle, NPPM_SETMENUITEMCHECK,
                   funcItem[DOCKABLE_INDEX]._cmdID, !( state & MF_CHECKED ) );
}
