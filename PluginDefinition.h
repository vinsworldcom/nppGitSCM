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

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H

//
// All difinitions of plugin interface
//
#include "PluginInterface.h"

#include <string>
#include <vector>

//-------------------------------------//
//-- STEP 1. DEFINE YOUR PLUGIN NAME --//
//-------------------------------------//
// Here define your plugin name
//
const TCHAR NPP_PLUGIN_NAME[] = TEXT( "&Git SCM" );

//-----------------------------------------------//
//-- STEP 2. DEFINE YOUR PLUGIN COMMAND NUMBER --//
//-----------------------------------------------//
//
// Here define the number of your plugin commands
//
const int nbFunc = 21;


//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit( HANDLE hModule );

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
//Initialization of your plugin commands
//
void commandMenuInit();

//
//Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command
//
bool setCommand( size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc,
                 ShortcutKey *sk = NULL, bool check0nInit = false );


//
// Your plugin command functions
//
#define LSV1_REFRESH_DELAY 500

HWND getCurScintilla();
bool getTortoiseLocation( std::wstring & );
std::wstring getGitLocation();
void updatePanelLoc();
void updatePanel();
void gitPrompt();
void gitGui();
void gitGuiFiles( std::vector<std::wstring> );
void giTk();
void giTkFiles( std::vector<std::wstring> );
void statusAll();
void statusAllFiles( std::vector<std::wstring> );
void commitAll();
void commitAllFiles( std::vector<std::wstring> );
void addFile();
void addFileFiles( std::vector<std::wstring> );
void diffFile();
void diffFileFiles( std::vector<std::wstring> );
void unstageFile();
void unstageFileFiles( std::vector<std::wstring> );
void restoreFile();
void restoreFileFiles( std::vector<std::wstring> );
void logFile();
void logFileFiles( std::vector<std::wstring> );
void blameFile();
void blameFileFiles( std::vector<std::wstring> );
void branchFile();
void branchFileFiles( std::vector<std::wstring> );
void pushFile();
void pushFileFiles( std::vector<std::wstring> );
void pullFile();
void pullFileFiles( std::vector<std::wstring> );
void doTortoise();
void DockableDlg();

#endif //PLUGINDEFINITION_H
