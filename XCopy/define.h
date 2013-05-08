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

#define IDP_FILES 1
#define IDT_TEXT 2
#define IDE_DIROUT 3
#define IDE_FILTER 5
#define IDP_SEARCH 6
#define IDC_SIZEMAX 7
#define IDE_SIZEMAX 8
#define IDC_TIMEMAX 9
#define IDE_TIMEMAX 10
#define IDC_READONLY 11
#define IDC_HIDDEN 12
#define IDC_SYSTEM 13
#define IDC_ARCHIVE 14
#define IDC_REPLACE 15
#define IDP_COPY 16
#define IDP_MOVE 17
#define IDP_DELETE 18
#define IDP_RENAME 19
#define IDLV_LIST 4
#define IDE_SEARCHDIR 20
#define IDC_NOCHANGEDIR 21
#define IDT_FILENAME 22
#define IDT_TIMELEFT 23
#define IDT_FILENUMBER 24
#define IDT_TIMELEFTCOPY 25
#define IDTB_PARTSIZE 26
#define IDTB_SPEED 27
#define IDPB_FILE 28
#define IDPB_COPY 29
#define IDT_SIZE 30
#define IDT_TOTALSIZE 31
#define IDT_NUMBERFILESFOUND 32
#define IDT_NUMBERFILESADDED 33
#define IDP_CANCEL 34
#define IDW_DRAGANDDROP 35
#define IDT_OCTET 36
#define IDE_PARTSIZE 37
#define IDE_TIMER 38
#define IDT_SECONDS 39
#define IDT_TIMER 40
#define IDT_PARTSIZE 41
#define IDT_SPEED 42
#define IDT_REALSPEED 43
#define IDT_TIMECOPY 44
#define IDT_MOYSPEED 45
#define IDP_ENCRYPT 46
#define IDP_DECRYPT 47
#define IDE_PWD 48
#define IDE_PWD2 49
#define IDR_REPLACEFILE 50
#define IDR_CREATENEWFILE 51
#define IDC_DELETEOLDFILE 52
#define IDE_EXTENSION 53
#define IDP_OK 54
#define IDT_EXTENSION 55
#define IDE_TEXTTOREPLACE 56
#define IDE_REPLACETEXT 57
#define IDC_EXTONLY 58
#define IDC_DELETEEXTENSION 59
#define IDT_PWD2 60
#define IDP_BILAN 61
#define IDP_STOP 62
#define IDP_PROTECT 63
#define IDP_DISPROTECT 64
#define IDC_READ 65
#define IDC_MODIF 66
#define IDC_DELETE 67
#define IDE_MEMORY 68
#define M_FILE_OPEN 69
#define M_FILE_SAVE 70
#define M_FILE_CLOSE 71
#define M_ACTION_COPY 72
#define M_ACTION_MOVE 73
#define M_ACTION_DELETE 74
#define M_ACTION_RENAME 75
#define M_ACTION_ENCRYPT 76
#define M_ACTION_DECRYPT 77
#define M_ACTION_PROTECT 78
#define M_ACTION_DISPROTECT 79
#define M_LIST_DELETE 80
#define M_LIST_DELETEALL 81
#define M_LIST_SORT 82
#define M_HELP_ABOUT 83
#define M_HELP_HELP 84
#define M_LIST_REFRESH 85
#define IDC_CASSSENSITIVE 86
#define IDC_WILDCARDS 87
#define IDC_RECURSIVE 88
#define IDP_ALL 89
#define IDP_ADD 90
#define ITEM_ADDSEARCHDIR 91
#define ITEM_ADDOUTDIR 92
#define IDR_INCLUDEDIRS 93
#define IDR_INCLUDEFILES 94
#define IDR_INCLUDEBOTH 95

#define MAX_CHAINE 5000
#define MAX_ITEMS 50000

#define IDCOL_FILENAME 0
#define IDCOL_FILESIZE 2
#define IDCOL_FILEPROTECTION 2
#define IDCOL_FILETYPE 1
#define IDCOL_FILEDATE 3
#define IDCOL_DIR 4
#define IDCOL_SECTION 0
#define IDCOL_ERROR 1
#define IDCOL_WHY 2
#define IDCOL_FILEERROR 3
#define IDCOL_HOUR 4

#define TIMERID_COPY 1
#define TIMERID_MOVE 2
#define TIMERID_DELETE 3
#define TIMERID_PLAY 4
#define TIMERID_SPEED 5
#define TIMERID_ENCRYPT 6
#define TIMERID_RENAME 7
#define TIMERID_CONTROL 8
#define TIMERID_PROTECTION 9
#define TIMERID_SHORTCUTS 10

#define ITEM_OPEN 100
#define ITEM_PLAY 101
#define ITEM_COPY 102
#define ITEM_MOVE 103
#define ITEM_DELETE 104
#define ITEM_CLEAR 105
#define ITEM_CLEARALL 106
#define ITEM_STOP 107
#define ITEM_ADDTOLIST 108

#define TW_COPY 1
#define TW_MOVE 2
#define TW_DELETE 3
#define TW_ENCRYPT 4
#define TW_RENAME 5
#define TW_DECRYPT 6
#define TW_PROTECT 7
#define TW_DISPROTECT 8
#define TW_NONE 0

#define ABT_GETLASTERROR 1
#define ABT_ISLASTERROR 2
#define ABT_GETERRNO 3
#define ABT_ISERRNO 4

#include <stdio.h>
extern FILE* errFile;
