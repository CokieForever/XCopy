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

#include "fonctions_repertoire.h"

int lister_tout(char dir[], char filePartName2[], char fileOutName[], WindowInfo *wi, int flags)
{
    int includeDirs = 0,
        includeSubDirs = 0,
        includeFiles = 0,
        cassSensitive = 0,
        useWildcards = 0,
        clearBeforeWrite = 0;

    if ((flags & LT_DIRS) == LT_DIRS)
       includeDirs = 1;
    if ((flags & LT_SUBDIRS) == LT_SUBDIRS)
       includeSubDirs = 1;
    if ((flags & LT_FILES) == LT_FILES)
       includeFiles = 1;
    if ((flags & LT_CASS) == LT_CASS)
       cassSensitive = 1;
    if ((flags & LT_WILDCARDS) == LT_WILDCARDS)
       useWildcards = 1;
    if ((flags & LT_CLEAR) == LT_CLEAR)
       clearBeforeWrite = 1;




    if (test_exist(dir) != 2)
       return -1;

    if (filePartName2[0] == '\0')
       filePartName2 = NULL;

    int itemFileName = 0;
    if (wi != NULL && wi->wndFileName != NULL)
       itemFileName = GetDlgCtrlID(wi->wndFileName);

    int itemNumberFilesFound = 0;
    if (wi != NULL && wi->wndNumberFilesFound != NULL)
       itemNumberFilesFound = GetDlgCtrlID(wi->wndNumberFilesFound);

    int itemNumberFilesAdded = 0;
    if (wi != NULL && wi->wndNumberFilesAdded != NULL)
       itemNumberFilesAdded = GetDlgCtrlID(wi->wndNumberFilesAdded);

    char buffer[MAX_CHAINE],
         directory[MAX_CHAINE],
         *position = NULL,
         text[MAX_CHAINE];

    strcpy(directory, dir);
    if (directory[strlen(directory) - 1] == '\\')
       directory[strlen(directory) - 1] = '\0';

    sprintf(buffer, "%s\\*", directory);

    WIN32_FIND_DATA wfd;
    HANDLE hSearch[100] = {NULL};
    hSearch[0] = FindFirstFile(buffer, &wfd);

    FILE *file = NULL;
    if (clearBeforeWrite)
       file = fopen(fileOutName, "w");
    else file = fopen(fileOutName, "a");

    if (file == NULL)
       return -1;


    int i = 0, j = 0, fileFound, countFound = 0, countAdded = 0,
        lastUpdate = GetTickCount(), quit = 0, cursorChanged = 0;
    HCURSOR previousCursor,
            cursorHand = LoadCursor(NULL, IDC_HAND);

    char *filePartName = NULL;
    if (filePartName2 != NULL)
    {
       filePartName = malloc(strlen(filePartName2) + 1);
       if (filePartName == NULL)
          return -1;
       strcpy(filePartName, filePartName2);

       for (j = 0 ; filePartName[j] != '\0' && !cassSensitive ; j++)
           filePartName[j] = toupper(filePartName[j]);
    }

    while (i >= 0 && hSearch[0] != INVALID_HANDLE_VALUE)
    {
          sprintf(buffer, "%s\\%s", directory, wfd.cFileName);
          countFound++;

          sprintf(text, "Fichier : %s", buffer);
          if (wi != NULL && wi->wndFileName != NULL)
             SetDlgItemText(GetParent(wi->wndFileName), itemFileName, text);

          for (j = 0 ; wfd.cFileName[j] != '\0' && !cassSensitive ; j++)
              wfd.cFileName[j] = toupper(wfd.cFileName[j]);

          if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
          {
             if (includeFiles && (filePartName == NULL || (!useWildcards && strstr(wfd.cFileName, filePartName) != NULL) || (useWildcards && comp_wildcards(filePartName, wfd.cFileName))))
             {
                countAdded++;
                fprintf(file, "%s\n", buffer);
             }

             fileFound = FindNextFile(hSearch[i], &wfd);
          }
          else if (strcmp(wfd.cFileName, ".") != 0 && strcmp(wfd.cFileName, "..") != 0)
          {
              if (includeDirs && (filePartName == NULL || (!useWildcards && strstr(wfd.cFileName, filePartName) != NULL) || (useWildcards && comp_wildcards(filePartName, wfd.cFileName))))
              {
                 countAdded++;
                 fprintf(file, "%s\n", buffer);
              }

              if (includeSubDirs && i < 100)
              {
                  i++;
                  strcpy(directory, buffer);
                  strcat(buffer, "\\*");
                  hSearch[i] = FindFirstFile(buffer, &wfd);
                  if (hSearch[i] == INVALID_HANDLE_VALUE)
                     fileFound = 0;
              }
              else fileFound = FindNextFile(hSearch[i], &wfd);
          }
          else fileFound = FindNextFile(hSearch[i], &wfd);

          sprintf(text, "Trouvés : %d", countFound);
          if (wi != NULL && wi->wndNumberFilesFound != NULL)
             SetDlgItemText(GetParent(wi->wndNumberFilesFound), itemNumberFilesFound, text);

          sprintf(text, "Ajoutés : %d", countAdded);
          if (wi != NULL && wi->wndNumberFilesAdded != NULL)
             SetDlgItemText(GetParent(wi->wndNumberFilesAdded), itemNumberFilesAdded, text);

          if (wi != NULL && wi->wndCancelButton != NULL)
          {
             RECT rect;
             GetWindowRect(wi->wndCancelButton, &rect);
             POINT pt;
             GetCursorPos(&pt);

             if (GetForegroundWindow() == GetParent(wi->wndCancelButton) && pt.x >= rect.left && pt.x <= rect.right && pt.y <= rect.bottom && pt.y >= rect.top)
             {
                 if (HIBYTE(GetAsyncKeyState(VK_LBUTTON)) || HIBYTE(GetKeyState(VK_LBUTTON)))
                    quit = 1;
                 if (!cursorChanged)
                 {
                    previousCursor = SetCursor(cursorHand);
                    cursorChanged = 1;
                 }
             }
             else if (cursorChanged)
             {
                  SetCursor(previousCursor);
                  cursorChanged = 0;
             }
          }

          if (wi != NULL && GetTickCount() - lastUpdate > 1000)
          {
               RECT rect;
               if (wi->wndFileName != NULL)
               {
                   GetClientRect(GetParent(wi->wndFileName), &rect);
                   InvalidateRect(GetParent(wi->wndFileName), &rect, FALSE);
                   UpdateWindow(GetParent(wi->wndFileName));
               }

               if (wi->wndNumberFilesAdded != NULL)
               {
                   GetClientRect(GetParent(wi->wndNumberFilesAdded), &rect);
                   InvalidateRect(GetParent(wi->wndNumberFilesAdded), &rect, FALSE);
                   UpdateWindow(GetParent(wi->wndNumberFilesAdded));
               }

               if (wi->wndNumberFilesFound != NULL)
               {
                   GetClientRect(GetParent(wi->wndNumberFilesFound), &rect);
                   InvalidateRect(GetParent(wi->wndNumberFilesFound), &rect, FALSE);
                   UpdateWindow(GetParent(wi->wndNumberFilesFound));
               }
          }


          while ((!fileFound || quit) && i >= 0)
          {
             FindClose(hSearch[i]);
             i--;

             if ((position = strrchr(directory, '\\')) != NULL)
                *position = '\0';

             if (i >= 0)
                fileFound = FindNextFile(hSearch[i], &wfd);
          }
    }

    FindClose(hSearch[0]);
    fclose(file);
    free(filePartName);
    DeleteObject(cursorHand);
    return 1;
}



int test_exist(char nomFichier[])
{
    int i;

    FILE *fichier = NULL;
    for (i = 0 ; (fichier = fopen(nomFichier, "r")) == NULL && i <= 10 ; i++);
    if (fichier != NULL)
    {
       while(fclose(fichier) == EOF);
       return 1;
    }

    DIR *repertoire = NULL;
    for (i = 0 ; (repertoire = opendir(nomFichier)) == NULL && i <= 10 ; i++);
    if (repertoire != NULL)
    {
       closedir(repertoire);
       return 2;
    }

    return 0;
}


int comp_wildcards(char string[], char refString[])
{
    //REMARQUE : seule "string" soit avoir des wildcards.
    //Sont pris comme des caractères normaux dans "refString".

    int i, j = 0;
    char *position = NULL;

    for (i = 0 ; string[i] != '\0' && refString[j] != '\0' ; i++)
    {
        if (string[i] == '*')
        {
            position = strchr(refString + j, string[i + 1]);
            if (position == NULL)
               return 0;
            j = position - refString - 1;
        }
        else if (string[i] != '?')
        {
             if (string[i] != refString[j])
                return 0;
        }

        j++;
    }

    if (string[i] != refString[j])
       return 0;
    else return 1;
}

