# Notepad++ Git

Author:  Michael J. Vincent


## Description

The original plugin was posted here: https://forum.lowyat.net/topic/1358320/all

I modified the code found here: https://github.com/alansbraga/NPPGit

... to use Git SCM for Windows, not TortoiseGit.  And then after seeing 
what TortoiseGit could do while using underlying Git for Windows, I 
added the TortoiseGit functionality back in.

You need to have [Git SCM for Windows](https://git-scm.com/) installed and in your 
`PATH` environment variable.  Usually `C:\Program Files\git\cmd`.


## Compiling

There was a Makefile written for gmake.exe from the MinGW distribution.

I've compiled it fine, but needed to statically include glibc so I could 
change my C compiler in my path.  This led to a much larger DLL.  Also, 
I couldn't get the docking interface to work when compiling with MinGW.

I compiled with MS Visual Studio Community 2017 and this seems to work 
OK.

For 32-bit:
```
    [x86 Native Tools Command Prompt for VS 2017]
    C:\> set Configuration=Release
    C:\> set Platform=x86
    C:\> msbuild
```

For 64-bit:
```
    [x64 Native Tools Command Prompt for VS 2017]
    C:\> set Configuration=Release
    C:\> set Platform=x64
    C:\> msbuild
```


## Installation

First you need to install [Git SCM for Windows](https://git-scm.com/), because 
this plugin simply uses the command line features of it.

It needs to be in your `PATH` environment variable.

    PATH=[..];<INSTALL_DIR>\git\cmd;

Optionally install [TortoiseGit](https://tortoisegit.org/)

After that, copy the:

+ 32-bit:  ./bin/GitSCM.dll
+ 64-bit:  ./bin64/GitSCM.dll

to the Notepad++ plugins folder:
  + In N++ <7.6, directly in the plugins/ folder
  + In N++ >=7.6, in a directory called GitSCM in the plugins/ folder (plugins/GitSCM/)

Next time you open Notepad++ you can access through menu Plugins->Git.


## Usage

This plugin relies on Git SCM for Windows](https://git-scm.com/) and 
optionally [TortoiseGit](https://tortoisegit.org/) for a nicer GUI 
experience.

The plugin relies on Notepad++ feature of "Follow current directory".  This 
can be set from Settings => Preferences => Default Directory => 
Follow current directory.

All Plugin => Git SCM menu options operate on the current Notepad++ file in 
the edit view.

All Plugins => Git SCM => Git Docking Panel => Toolbar buttons operate on 
the current Notepad++ file in the edit view ***UNLESS*** a file or files are 
selected in the List View of the Git Docking Panel in which case those files 
are operated on.

The Git Docking Panel Toolbar buttons have tool tips if hovered over.


### Panel
In the Panel:

---

Branch [Branch]

Directory

I | W | File
--|---|------
Status | Status | File_name

---

Branch shows the current branch for the current repo.

[Branch] button allows one to create a new branch.  **NOTE:** this is only 
supported with TortoiseGit.

The list view shows the parsed output of `git status --porcelain --branch`.  
See https://git-scm.com/docs/git-status for more information.

+ Right-clicking any item in the List View will bring up a context menu.

+ Multiple items in the List View can be selected by holding CTRL or Shift.

+ Double-clicking any filename in the "File" column will open the file in 
Notepad++; as will pressing "Enter" or "Spacebar".

+ Double-clicking any "M" in the "W" (working) column will `git add` the file 
to the index, moving the "M" to the "I" (index) column.

+ Double-clicking any "M" in the "I" (index) column will `git reset HEAD` 
the file out of the index, moving the "M" to the "W" (working) column.


### Settings

Git Directory Path is only required if `git` is not found in your system 
PATH environment variable or if you want to use a different Git.  Only the 
directory to the Git executable is required - use the "git..." button to 
help locate and enter the directory only if required.

Git Prompt is to launch a terminal prompt in the current repo directory.  
The default shell is PowerShell.

Use N++ Colors simply changes the colors of the List View background and 
foreground text in the Git Docking Panel to match that of the current 
Notepad++ theme.
