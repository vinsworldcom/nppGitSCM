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
PATH environment variable.


DEPENDENCIES

The Makefile is written for gmake.exe from the MinGW distribution.  There 
is a Dev-Cpp project file (.dev) included.

I compiled with the MinGW released with Strawberry Perl version:

This is perl 5, version 18, subversion 4 (v5.18.4) built for 
MSWin32-x86-multi-thread-64int

g++ (release with patches / build 20130526 by strawberryperl.com) 4.7.3

  - I need to compile as 32-bit to match my Notepad++ install
  - I have UNICODE enabled to match my Notepad++ install
  
You can edit the Makefile to suit your needs.


INSTALLATION

First you need to install GIT SCM for Windows, because this plugin simply 
uses the command line features of it.
<https://git-scm.com/>

It needs to be in your PATH environment variable.

After that, take the DLL NPPGit.dll in the ./bin folder of this repository 
and copy to the Notepad++ plugins folder.

Next time you open Notepad++ you can access through menu Plugins->Git or 
quickly with Alt+P then G.
