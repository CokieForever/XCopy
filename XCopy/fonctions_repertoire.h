/*
XCopy - Software for copy management
Copyright (C) 2008-2013  Cokie

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef FONCTIONS_REPERTOIRE

#define FONCTIONS_REPERTOIRE

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <dirent.h>
#include <shlobj.h>
#include <Windowsx.h>

#ifndef MAX_CHAINE
#define MAX_CHAINE 5000
#endif

#define LT_DIRS 1
#define LT_SUBDIRS 2
#define LT_FILES 4
#define LT_CASS 8
#define LT_WILDCARDS 16
#define LT_CLEAR 32
#define LT_ALLFILES 7
#define LT_STANDARD 54


typedef struct WindowInfo
{
        HWND wndFileName;
        HWND wndNumberFilesFound;
        HWND wndNumberFilesAdded;
        HWND wndCancelButton;
} WindowInfo;


int lister_tout(char dir[], char filePartName[], char fileOutName[], WindowInfo *wi, int flags);
int test_exist(char nomFichier[]);
int comp_wildcards(char string[], char refString[]);


#endif
