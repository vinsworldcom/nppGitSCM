#ifndef RESOURCE_H
#define RESOURCE_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* VERSION DEFINITIONS */
#define VER_MAJOR   1
#define VER_MINOR   4
#define VER_RELEASE 5
#define VER_BUILD   1
#define VER_STRING  STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_RELEASE) "." STR(VER_BUILD)

#define FILE_DESCRIPTION  "Notepad++ Plugin for Git SCM."
#define INTERNAL_NAME     "Notepad++ Git SCM"

#define COMPANY_NAME      "Vin's World"
#define FILE_VERSION      VER_STRING
#define LEGAL_COPYRIGHT   "Copyright (C) VinsWorld. All Rights Reserved."
#define LEGAL_TRADEMARKS  ""
#define ORIGINAL_FILENAME "GitSCM"
#define PRODUCT_NAME      "GITSCM"
#define PRODUCT_VERSION   VER_STRING

/* ADDITIONAL DEFINITIONS */

#define DOCKABLE_INDEX 4
#define TORTOISE_INDEX 19

#define IDB_TB_GIT 1001

#endif
