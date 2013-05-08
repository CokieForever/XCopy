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

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../XCopy/fonctions_process.h"
#include "../XCopy/fonctions_repertoire.h"

#define ICON_ID 1
#define MENU_QUIT 2
#define MENU_XCOPY 3
#define TIMERID_XCOPY 1
#define TIMERID_PROTECT 2

#define MAX_CHAINE 5000


LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
int print_icon_menu();
int is_item_selected(HWND listWnd, int item);
int get_selected(HWND listWnd);
void disprotect_all();
int protect_all();
int verify_xcopy();


HWND mainWnd;

HANDLE tableauHandles[MAX_CHAINE] = {NULL},
       sharedMemory = NULL;

HICON mainIcon;

char xCopyExecutable[MAX_CHAINE] = "xcopy.exe",
     listFileName[MAX_CHAINE] = "protection.dll";


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow
                     )

{
    //vérification de l'existence d'une éventuelle zone de mémoire partagée
    sharedMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "xcopy-protection");
    if (sharedMemory != NULL)
       return 0;

    //création de la zone de mémoire partagée
    sharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_CHAINE, "xcopy-protection");
    if (sharedMemory == NULL)
        return 0;

    char buffer[MAX_CHAINE] = "",
         dossierBase[MAX_CHAINE] = "",
         *argts = GetCommandLine(),
         appName[MAX_CHAINE] = "", *p;

    GetModuleFileName(NULL, dossierBase, MAX_CHAINE-1);
    p = strrchr(dossierBase, '\\');
    *p = '\0';
    strcpy(appName, p+1);

    chdir(dossierBase);  //rétablissement du dossier de lancement de l'application

    if ( (p = strrchr(argts, '/')) )
        strcpy(argts, p+1);

    strcpy(buffer, xCopyExecutable);
    sprintf(xCopyExecutable, "%s\\%s", dossierBase, buffer);
    strcpy(listFileName, argts);
    if (strchr(listFileName, '\\') == NULL)
       sprintf(listFileName, "%s\\%s", dossierBase, argts);


    if (test_exist(listFileName) != 1)
    {
        FILE *file = fopen(listFileName, "w");
        if (file != NULL)
           fclose(file);
    }

    mainIcon = LoadImage(hInstance, "mainIcon", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    protect_all();


    HANDLE currentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId()),
           token = NULL;
    OpenProcessToken(currentProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    LookupPrivilegeValue(NULL, "SeDebugPrivilege", &(tp.Privileges[0].Luid));
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(token, FALSE, &tp, 0, NULL, NULL);
    CloseHandle(token);
    CloseHandle(currentProcess);


    MSG message;

    WNDCLASSEX principale;
    principale.cbSize = sizeof(WNDCLASSEX);
    principale.style = CS_HREDRAW | CS_VREDRAW;
    principale.lpfnWndProc = WindowProcedure;
    principale.cbClsExtra = 0;
    principale.cbWndExtra = 0;
    principale.hInstance = hInstance;
    principale.hIcon = mainIcon;
    principale.hCursor = LoadCursor(NULL, IDC_ARROW);
    principale.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    principale.lpszMenuName = NULL;
    principale.lpszClassName = "std";
    principale.hIconSm = LoadIcon(NULL,IDI_APPLICATION);

    if (!RegisterClassEx(&principale))
       return 0;

    mainWnd = CreateWindowEx(
          WS_EX_CLIENTEDGE,
          "std",
          "Notre fenêtre",
          WS_OVERLAPPEDWINDOW,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          NULL,
          NULL,
          hInstance,
          NULL
       );
    ShowWindow(mainWnd, SW_HIDE);

    SetTimer(mainWnd, TIMERID_PROTECT, 500, NULL);
    SetTimer(mainWnd, TIMERID_XCOPY, 500, NULL);


    //mise en place de l'icone
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = mainWnd;
    nid.uID = ICON_ID;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_COMMAND;
    nid.hIcon = principale.hIcon;
    strcpy(nid.szTip, "XProtection");

    Shell_NotifyIcon(NIM_ADD, &nid);

    while (GetMessage (&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = mainWnd;
    nid.uID = ICON_ID;
    nid.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &nid);

    return message.wParam;
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int Select, notification;
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = mainWnd;
    nid.uID = ICON_ID;
    nid.uFlags = 0;

    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage (0);
            break;
        case WM_COMMAND:
             Select = LOWORD(wParam);
             switch (Select)
             {
                    case ICON_ID:
                        notification = lParam;

                        if (notification == WM_RBUTTONDOWN)
                            print_icon_menu();
                        return FALSE;
                    case MENU_XCOPY:
                    {
                        HWND *targetWnd = NULL;
                        HANDLE handle = OpenFileMapping(FILE_MAP_READ, FALSE, "xcopyshared");

                        if (handle != NULL)
                        {
                            targetWnd = (HWND*) MapViewOfFile(handle, FILE_MAP_READ, 0, 0, sizeof(HWND));
                            CloseHandle(handle);
                            if (IsWindow(*targetWnd))
                            {
                               ShowWindow(*targetWnd, SW_RESTORE);
                               SetForegroundWindow(*targetWnd);
                               UnmapViewOfFile(targetWnd);
                               return 1;
                            }

                            UnmapViewOfFile(targetWnd);
                        };

                        return FALSE;
                    }
             }
             return FALSE;
        case WM_TIMER:
             Select = LOWORD(wParam);
             switch (Select)
             {
                    case TIMERID_XCOPY:
                         if (!verify_xcopy())
                         {
                            KillTimer(hwnd, TIMERID_XCOPY);
                            ShellExecute(mainWnd, "open", xCopyExecutable, NULL, NULL, SW_HIDE);
                            SetTimer(hwnd, TIMERID_XCOPY, 500, NULL);
                         }
                         return FALSE;
                    case TIMERID_PROTECT:
                    {
                         KillTimer(hwnd, TIMERID_PROTECT);

                         char *command = NULL;
                         command = (char*) MapViewOfFile(sharedMemory, FILE_MAP_ALL_ACCESS, 0, 0, MAX_CHAINE);
                         if (command != NULL)
                         {
                             if (strcmp(command, "update") == 0)
                                protect_all();
                             else if (strcmp(command, "close") == 0)
                                  DestroyWindow(hwnd);
                             command[0] = '\0';
                             UnmapViewOfFile(command);
                         }

                         SetTimer(hwnd, TIMERID_PROTECT, 500, NULL);
                         return FALSE;
                    }
             }

             return FALSE;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}



int print_icon_menu()
{
      HMENU hMenu;
      POINT pt;
      GetCursorPos(&pt);

      hMenu = CreatePopupMenu();
      AppendMenu(hMenu, MF_STRING, MENU_XCOPY, "Ouvrir XCopy");

      if (TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, mainWnd, NULL))
         return 1;
      else return 0;
}


int verify_xcopy()
{
    char process[MAX_CHAINE];
    strcpy(process, strrchr(xCopyExecutable, '\\') + 1);
    if (!test_exist_process(process))
       return 0;

    HANDLE handle = OpenFileMapping(FILE_MAP_READ, FALSE, "xcopyshared");
    if (handle == NULL)
       return 0;
    CloseHandle(handle);

    return 1;
}


int protect_all()
{
    disprotect_all();

    FILE *file = NULL;
    if ((file = fopen(listFileName, "r")) == NULL)
    {
       char text[MAX_CHAINE];
       sprintf(text, "Impossible d'ouvrir le fichier %s.\nProtection non opérationnelle !", listFileName);
       MessageBox(NULL, text, "Erreur !", MB_OK | MB_ICONERROR);
       return 0;
    }

    int i = 0;
    char buffer[MAX_CHAINE], access[10];
    while(i < MAX_CHAINE && fgets(buffer, MAX_CHAINE, file) != NULL)
    {
         if (strchr(buffer, '\n') != NULL)
            *strchr(buffer, '\n') = '\0';

         fgets(access, 10, file);
         access[0] = strtol(access, NULL, 10);

         if (tableauHandles[i] != NULL && tableauHandles[i] != INVALID_HANDLE_VALUE)
            CloseHandle(tableauHandles[i]);

         tableauHandles[i] = CreateFile(buffer, GENERIC_READ, access[0], NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

         char text[MAX_CHAINE] = "\0";
         sprintf(text, "Impossible de protéger le fichier %s : erreur %ld.", buffer, GetLastError());
         if (tableauHandles[i] == INVALID_HANDLE_VALUE)
            MessageBox(NULL, text, "Erreur !", MB_OK | MB_ICONERROR);
         else i++;
    }

    fclose(file);
    return 1;
}


void disprotect_all()
{
    int i;
    for (i = 0 ; i < MAX_CHAINE ; i++)
    {
        if (tableauHandles[i] != NULL)
           CloseHandle(tableauHandles[i]);
        tableauHandles[i] = NULL;
    }

    return;
}


int is_item_selected(HWND listWnd, int item)
{
    int state = LOBYTE(SendMessage(listWnd, LVM_GETITEMSTATE, item, LVIS_SELECTED));
    if ((state & LVIS_SELECTED) == LVIS_SELECTED)
       return 1;
    else return 0;
}


int get_selected(HWND listWnd)
{
    int i, max = SendMessage(listWnd, LVM_GETITEMCOUNT, 0, 0);

    for (i = 0 ; i < max ; i++)
    {
        if (is_item_selected(listWnd, i))
           break;
    }

    if (is_item_selected(listWnd, i))
       return i;
    else return -1;

}
