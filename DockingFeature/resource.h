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

#ifndef DOCKINGFEATURE_RESOURCE_H
#define DOCKINGFEATURE_RESOURCE_H


#ifndef IDC_STATIC
#define IDC_STATIC  -1
#endif

// Docking
#define IDD_PLUGINGITPANEL 2500

#define IDI_PLUGINGITPANEL 2501

#define IDB_TOOLBAR1 1601
#define IDB_TOOLBAR2 1600

#define IDB_PAGER1   1701
#define IDB_PAGER2   1702

#define IDC_BTN_GITGUI    (IDB_TOOLBAR1 + 1)
#define IDC_BTN_GITK      (IDB_TOOLBAR1 + 2)
#define IDC_BTN_PROMPT    (IDB_TOOLBAR1 + 3)
#define IDC_BTN_PULL      (IDB_TOOLBAR1 + 4)
#define IDC_BTN_STATUS    (IDB_TOOLBAR1 + 5)
#define IDC_BTN_COMMIT    (IDB_TOOLBAR1 + 6)
#define IDC_BTN_PUSH      (IDB_TOOLBAR1 + 7)
#define IDC_BTN_ADD       (IDB_TOOLBAR1 + 8)
#define IDC_BTN_UNSTAGE   (IDB_TOOLBAR1 + 9)
#define IDC_BTN_RESTORE   (IDB_TOOLBAR1 + 10)
#define IDC_BTN_DIFF      (IDB_TOOLBAR1 + 11)
#define IDC_BTN_LOG       (IDB_TOOLBAR1 + 12)
#define IDC_BTN_BLAME     (IDB_TOOLBAR1 + 13)
#define IDC_BTN_SETTINGS  (IDB_TOOLBAR1 + 14)
#define IDC_CHK_TORTOISE  (IDD_PLUGINGITPANEL + 11)
#define IDC_EDT_DIR       (IDD_PLUGINGITPANEL + 12)
#define IDC_EDT_BRANCH    (IDD_PLUGINGITPANEL + 13)
#define IDB_BTN_BRANCH    (IDD_PLUGINGITPANEL + 14)
#define IDC_LSV1          (IDD_PLUGINGITPANEL + 15)

// Settings
#define IDD_SETTINGS      2600

#define IDB_OK            (IDD_SETTINGS + 1)
#define IDC_GRP1          (IDD_SETTINGS + 2)
#define IDC_BTN_GITPATH   (IDD_SETTINGS + 3)
#define IDC_EDT_GITPATH   (IDD_SETTINGS + 4)
#define IDC_EDT_GITPROMPT (IDD_SETTINGS + 5)
#define IDC_CHK_NPPCOLOR  (IDD_SETTINGS + 6)

#endif // RESOURCE_H
