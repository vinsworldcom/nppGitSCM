NAME

Notepad++ Git
Author:  Michael J. Vincent


DESCRIPTION

The original plugin was posted here:
https://forum.lowyat.net/topic/1358320/all

I modified the code found here:
https://github.com/alansbraga/NPPGit

... to use GIT SCM for Windows, not TortoiseGit.

You need to have GIT SCM (https://git-scm.com/) installed and in your 
PATH environment variable.  Usually C:\Program Files\git\cmd.


COMPILING

There was a Makefile written for gmake.exe from the MinGW distribution.

I've compiled it fine, but needed to statically include glibc so I could 
change my C compiler in my path.  This led to a much larger DLL.  Also, 
I couldn't get the docking interface to work when compiling with MinGW.

I compiled with MS Visual Studio Community 2017 and this seems to work 
OK.

[x86 Native Tools Command Prompt for VS 2017]
Configuration=Release
Platform=Win32

  msbuild


INSTALLATION

First you need to install GIT SCM for Windows, because this plugin simply 
uses the command line features of it.
<https://git-scm.com/>

It needs to be in your PATH environment variable.

  PATH=[..];<INSTALL_DIR>\git\cmd;

After that, take the GitSCM.dll in the ./bin folder of this repository 
and copy to the Notepad++ plugins folder:
  - In N++ <7.6, directly in the plugins/ folder
  - In N++ >=7.6, in a directory called GitSCM in the plugins/ folder

Next time you open Notepad++ you can access through menu Plugins->Git or 
quickly with Alt+P then G.
