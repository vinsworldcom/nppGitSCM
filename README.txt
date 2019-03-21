NAME

Notepad++ Git
Author:  Michael J. Vincent


DESCRIPTION

The original plugin was posted here:
https://forum.lowyat.net/topic/1358320/all

I modified the code found here:
https://github.com/alansbraga/NPPGit

... to use Git SCM for Windows, not TortoiseGit.  And then after seeing 
what TortoiseGit could do while using underlying Git for Windows, I 
added the TortoiseGit functionality back in.

You need to have Git SCM (https://git-scm.com/) installed and in your 
PATH environment variable.  Usually C:\Program Files\git\cmd.


COMPILING

There was a Makefile written for gmake.exe from the MinGW distribution.

I've compiled it fine, but needed to statically include glibc so I could 
change my C compiler in my path.  This led to a much larger DLL.  Also, 
I couldn't get the docking interface to work when compiling with MinGW.

I compiled with MS Visual Studio Community 2017 and this seems to work 
OK.

For 32-bit:
  [x86 Native Tools Command Prompt for VS 2017]
  C:\> set Configuration=Release
  C:\> set Platform=x86
  C:\> msbuild

For 64-bit:
  [x64 Native Tools Command Prompt for VS 2017]
  C:\> set Configuration=Release
  C:\> set Platform=x64
  C:\> msbuild

INSTALLATION

First you need to install Git SCM for Windows, because this plugin simply 
uses the command line features of it.
<https://git-scm.com/>

It needs to be in your PATH environment variable.

  PATH=[..];<INSTALL_DIR>\git\cmd;

Optionally install TortoiseGit <https://tortoisegit.org/>

After that, copy the:

32-bit:
    ./bin/GitSCM.dll
   
64-bit:
    ./bin64/GitSCM.dll

to the Notepad++ plugins folder:
  - In N++ <7.6, directly in the plugins/ folder
  - In N++ >=7.6, in a directory called GitSCM in the plugins/ folder
    (plugins/GitSCM/)

Next time you open Notepad++ you can access through menu Plugins->Git or 
quickly with Alt+P then G.
