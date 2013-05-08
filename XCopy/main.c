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

#define _WIN32_IE 0x0600
#define _WIN32_NT 0x0502
#define _WIN32_WINNT 0x0502
#define NTDDI_VERSION NTDDI_WINXP

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <dirent.h>
#include <shlobj.h>
#include "fmod.h"
#include <Shellapi.h>

#include "define.h"
#include "fonctions_repertoire.h"
#include "fonctions_cryptage.h"
#include "fonctions_fichier.h"
#include "fonctions_process.h"
#include "fonctions_registre.h"

FILE *errFile = NULL;



//structures
typedef struct LVITEM_2
{
        LVITEM lvi;
        char pszText[MAX_CHAINE];
} LVITEM_2;


// fonctions
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);    // callback de la fenetre principale
LRESULT CALLBACK OptnDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK StateDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK IsSearchingDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EncryptionProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RenameProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BilanDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ProtectionProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DisprotectionProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PasswordProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK BeginSearchProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int CALLBACK CompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

BOOL
WINAPI
ConvertStringSecurityDescriptorToSecurityDescriptorA(
    IN  LPCSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    );
#define ConvertStringSecurityDescriptorToSecurityDescriptor  ConvertStringSecurityDescriptorToSecurityDescriptorA


int ajouter_fichier(char nom[], int update, HWND listWnd);
int ajouter_fichier_special(char nom[], HWND listWnd, int protectionAttributes);
int ajouter_liste(char nomFichier[], HWND listWnd);
int is_file_enabled_to_copy(char name[], char destination[]);
int draw_progress_bar(int percent, int number);
int is_item_selected(HWND listWnd, int item);
int get_selected_items(HWND listWnd, int **ptOut);
void retrieve_system_time(char text[], SYSTEMTIME *st);
void enable_buttons(BOOL state);
void delete_second_extension(char name[]);
int add_bilan_text(char section[], char error[], char why[], char file[], SYSTEMTIME *hour, int flags);
void attrib_lParams(HWND listWnd);
int find_item(char fileAdress[]);
int delete_item(char adressFile[]);
void free_table();
void retrieve_error_text_for_SHFileOperation(int errorCode, char text[]);
int PL_stop_protection();
int PL_verify_protection();
int PL_launch_protection(int forcer);
int PL_remove_file(char fileName[]);
int PL_add_file(char fileName[], int attributes);
int create_toolTips();
void centrer_fenetre(HWND hwnd1, HWND hwnd2);
int is_item_directory (int index);
int correct_adress(char dirIn[], char dirOut[]);
int is_dir_included(char dir[]);
int adjust_file_name(char path[]);
int ready_to_work(int typeWork);


//variales globales, classées par type
HWND mainWnd,     //fenêtre principale
     optnDlgWnd,
     stateDlgWnd,
     dragAndDropWnd,
     bilanWnd,
     tableauToolTips[20] = {NULL};

RECT tailleMainWnd,
     tailleOptnDlgWnd,
     tailleStateDlgWnd;

HIMAGELIST imgList;

HICON iconMove,
      iconDelete,
      iconCopy,
      iconCopyBig,
      iconRename,
      iconEncrypt,
      iconDecrypt,
      iconStop,
      iconShield,
      iconNoShield,
      iconSearch;

HINSTANCE mainInstance;

HMENU menuFile,
      menuAction,
      menuList,
      menuHelp,
      mainMenu;

HANDLE PL_processHandle = NULL;

int icon1,
    numeroFichier = 0,
    tailleACopier = 100,          //en ko
    nombreFichiersACopier = 0,
    heureDebutCopie = 0,
    tailleMaxCopie = -1,           //en ko
    tempsMaxCopie = -1,            //en ms
    vitesseCopie = 0,              //en octets par sec
    caption, edge, ymenu,
    isCopying = 0, isMoving = 0, isDeleting = 0,
    etapeDeplacement = 0,
    tempsTotalSuppression = 0,     //en ms
    itemSelectionne = 0,
    fmodOK = 0,
    onlySelected = 0,
    nombreTotalItems = 0,
    timerMoveActive = 0,
    timerCopyActive = 0,
    derniereTailleCopiee = 0,
    vitesseReelleCopie = 0,
    deleteOldFiles = 0,
    createNewFiles = 0,
    timerEncryptActive = 0,
    isEncrypting = 0,
    renameExtensionsOnly = 0,
    timerRenameActive = 0,
    deleteOldExtension = 0,
    isDecrypting = 0,
    stopNow = 0,
    dernierItemVerifie = 0,
    protection = 0,
    canModifyPassword = 0,
    keyPressed = 0,
    cassSensitiveResearch = 0,
    wildcardsResearch = 0,
    recursiveResearch = 1,
    dirsResearch = 0,
    filesResearch = 1;

double tempsTimer = 0.01,                    //en sec
       tailleFichierACopier = 0,            //en ko
       tailleFichiersCopies = 0,            //en ko
       tailleTotaleFichiersACopier = 0;     //en ko

char configFileName[MAX_CHAINE] = "config.cfg",
     *tableauFichiersACopier[MAX_ITEMS] = {NULL},
     listFileName[MAX_CHAINE] = "list.txt",
     nomItemSelectionne[MAX_CHAINE],
     oldTextItem[MAX_CHAINE],
     encryptionExtension[MAX_CHAINE],
     password[MAX_CHAINE],
     nomFichierACopier[MAX_CHAINE],
     PL_listFileName[MAX_CHAINE] = "protection.dll",
     PL_executable[MAX_CHAINE] = "xprotection.exe",
     protectionPassword[MAX_CHAINE] = "\0",
     dossierBase[MAX_CHAINE] = "",
     *tableauDossiersACopier[MAX_ITEMS] = {NULL};

FILE *fichierACopier = NULL,
     *copieDuFichier = NULL;

FSOUND_STREAM *stream = NULL;

LVITEM_2 tableauTotalItems[MAX_ITEMS];



int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow
                     )

{

    INITCOMMONCONTROLSEX icce;
    icce.dwICC = ICC_BAR_CLASSES;
    icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCommonControlsEx(&icce);

    //vérification de l'existence d'une éventuelle zone de mémoire partagée
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
    }

    //création de la zone de mémoire partagée
    handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND), "xcopyshared");
    if (handle == NULL)
    {
       if (MessageBox(NULL, "Impossible de créer la mémoire partagée. Continuer ?", "Erreur ! ", MB_YESNO | MB_ICONERROR) == IDNO)
          return 0;
    }

    void *ptMap = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND));
    if (ptMap == NULL)
    {
      if (MessageBox(NULL, "Impossible de créer la mémoire partagée. Continuer ?", "Erreur ! ", MB_YESNO | MB_ICONERROR) == IDNO)
      {
          CloseHandle(handle);
          return 0;
      }
    }

    mainInstance = hInstance;

    char buffer[MAX_CHAINE];
    tableauFichiersACopier[0] = malloc(MAX_CHAINE);
    tableauFichiersACopier[0][0] = '\0';

    caption = GetSystemMetrics(SM_CYCAPTION);
    edge = GetSystemMetrics(SM_CXEDGE);
    ymenu = GetSystemMetrics(SM_CYMENU);

    iconRename = LoadImage(hInstance, "iconRename", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconDelete = LoadImage(hInstance, "iconDelete", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconCopy = LoadImage(hInstance, "iconCopy", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconCopyBig = LoadImage(hInstance, "iconCopy", IMAGE_ICON, 24, 24, LR_DEFAULTCOLOR);
    iconMove = LoadImage(hInstance, "iconMove", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconEncrypt = LoadImage(hInstance, "iconEncrypt", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconDecrypt = LoadImage(hInstance, "iconDecrypt", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconStop = LoadImage(hInstance, "iconStop", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconShield = LoadImage(hInstance, "iconShield", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconNoShield = LoadImage(hInstance, "iconNoShield", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    iconSearch = LoadImage(hInstance, "iconSearch", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    char *argts = GetCommandLine(),
         appName[MAX_CHAINE] = "", *p;

    GetModuleFileName(NULL, dossierBase, MAX_CHAINE-1);
    p = strrchr(dossierBase, '\\');
    *p = '\0';
    strcpy(appName, p+1);

    chdir(dossierBase);  //rétablissement du dossier de lancement de l'application

    if ( (p = strrchr(argts, '/')) )
        strcpy(argts, p+1);
    if (strcmp(argts, "silent") == 0)
       nCmdShow = SW_HIDE;

    strcpy(buffer, configFileName);
    sprintf(configFileName, "%s\\%s", dossierBase, buffer);
    strcpy(buffer, listFileName);
    sprintf(listFileName, "%s\\%s", dossierBase, buffer);
    strcpy(buffer, PL_listFileName);
    sprintf(PL_listFileName, "%s\\%s", dossierBase, buffer);
    strcpy(buffer, PL_executable);
    sprintf(PL_executable, "%s\\%s", dossierBase, buffer);

    char command[MAX_CHAINE];
    sprintf(command, "\"%s\\%s\" /silent", dossierBase, appName);
    sprintf(buffer, "%s\\%s", dossierBase, appName);


    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = FALSE;

    char secu[MAX_CHAINE] = "D:(D;CIOI;SDWOKW;;;WD)";
    PSECURITY_DESCRIPTOR psd = NULL;
    int size = 0;

    ConvertStringSecurityDescriptorToSecurityDescriptor(secu, 1, &psd, (PULONG)&size);
    sa.lpSecurityDescriptor = psd;
    create_autostart(buffer, command, &sa, TRUE);

    LocalFree(psd);


    errFile = fopen("stderr.txt", "w");


    if (!FSOUND_Init(44100, 32, FSOUND_INIT_ACCURATEVULEVELS)) // Initialisation de FMOD. Si erreur alors...
    {
        if (MessageBox(NULL, "Impossible de démarrer FMOD.\nCertaines fonctions (audio) ne seront pas disponibles.\nContinuer malgré tout ?", "Attention", MB_ICONWARNING | MB_YESNO) == IDNO)
           exit(EXIT_FAILURE);
    }
    else fmodOK = 1;

    CRYPT_init();  //Initialisation de la librairie de cryptage


    menuFile = CreatePopupMenu();
    AppendMenu(menuFile, MF_STRING, M_FILE_OPEN, "Ouvrir une liste");
    AppendMenu(menuFile, MF_STRING, M_FILE_SAVE, "Enregistrer la liste sous...");
    AppendMenu(menuFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(menuFile, MF_STRING, M_FILE_CLOSE, "Fermer");

    menuAction = CreatePopupMenu();
    AppendMenu(menuAction, MF_STRING, M_ACTION_COPY, "Copier");
    AppendMenu(menuAction, MF_STRING, M_ACTION_MOVE, "Déplacer");
    AppendMenu(menuAction, MF_STRING, M_ACTION_DELETE, "Supprimer");
    AppendMenu(menuAction, MF_STRING, M_ACTION_RENAME, "Renommer");
    AppendMenu(menuAction, MF_SEPARATOR, 0, NULL);
    AppendMenu(menuAction, MF_STRING, M_ACTION_ENCRYPT, "Crypter");
    AppendMenu(menuAction, MF_STRING, M_ACTION_DECRYPT, "Décrypter");
    AppendMenu(menuAction, MF_SEPARATOR, 0, NULL);
    AppendMenu(menuAction, MF_STRING, M_ACTION_PROTECT, "Protéger");
    AppendMenu(menuAction, MF_STRING, M_ACTION_DISPROTECT, "Déprotéger");

    menuList = CreatePopupMenu();
    AppendMenu(menuList, MF_STRING, M_LIST_DELETE, "Effacer les fichiers sélectionnés");
    AppendMenu(menuList, MF_STRING, M_LIST_DELETEALL, "Vider la liste");
    AppendMenu(menuList, MF_SEPARATOR, 0, NULL);
    AppendMenu(menuList, MF_STRING, M_LIST_SORT, "Trier les fichiers par nom");
    AppendMenu(menuList, MF_STRING, M_LIST_REFRESH, "Actualiser");

    menuHelp = CreatePopupMenu();
    AppendMenu(menuHelp, MF_STRING, M_HELP_ABOUT, "A propos...");
    AppendMenu(menuHelp, MF_STRING, M_HELP_HELP, "Aide sur XCopy");

    mainMenu = CreateMenu();
    AppendMenu(mainMenu, MF_POPUP | MF_STRING, (UINT) menuFile, "Fichier");
    AppendMenu(mainMenu, MF_POPUP | MF_STRING, (UINT) menuAction, "Action");
    AppendMenu(mainMenu, MF_POPUP | MF_STRING, (UINT) menuList, "Liste");
    AppendMenu(mainMenu, MF_POPUP | MF_STRING, (UINT) menuHelp, "?");


    WNDCLASSEX wincl;    // classe de la fenêtre principale
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wincl.lpfnWndProc = WndProc;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hInstance = hInstance;
    wincl.hIcon = iconCopyBig;
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wincl.lpszMenuName = NULL;
    wincl.lpszClassName = "std";
    wincl.hIconSm = iconCopy;

    if (!RegisterClassEx (&wincl))
        return 0;

    mainWnd = CreateWindowEx(
          WS_EX_CLIENTEDGE,
          "std",
          "XCopy",
          WS_OVERLAPPEDWINDOW,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          NULL,
          mainMenu,
          hInstance,
          NULL
       );
    ShowWindow(mainWnd, nCmdShow);

    if (handle != NULL && ptMap != NULL)
    {
       CopyMemory(ptMap, &mainWnd, sizeof(HWND));
       UnmapViewOfFile(ptMap);
    }


    optnDlgWnd = CreateDialog(hInstance, "OptnDlg", mainWnd, (DLGPROC) OptnDlgProc);
    SetWindowPos(optnDlgWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

    SendMessage(GetDlgItem(optnDlgWnd, IDP_COPY), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconCopy);
    SendMessage(GetDlgItem(optnDlgWnd, IDP_MOVE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconMove);
    SendMessage(GetDlgItem(optnDlgWnd, IDP_DELETE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconDelete);
    SendMessage(GetDlgItem(optnDlgWnd, IDP_RENAME), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconRename);
    SendMessage(GetDlgItem(optnDlgWnd, IDP_ENCRYPT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconEncrypt);
    SendMessage(GetDlgItem(optnDlgWnd, IDP_DECRYPT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconDecrypt);
    SendMessage(GetDlgItem(optnDlgWnd, IDP_STOP), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconStop);
    SendMessage(GetDlgItem(optnDlgWnd, IDP_PROTECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconShield);
    SendMessage(GetDlgItem(optnDlgWnd, IDP_DISPROTECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)iconNoShield);

    GetClientRect(optnDlgWnd, &tailleOptnDlgWnd);
    GetClientRect(mainWnd, &tailleMainWnd);
    SetWindowPos(mainWnd, NULL, 0, 0, tailleMainWnd.right, tailleOptnDlgWnd.bottom - tailleOptnDlgWnd.top, SWP_NOMOVE | SWP_NOZORDER);
    GetClientRect(mainWnd, &tailleMainWnd);

    stateDlgWnd = CreateDialog(hInstance, "StateDlg", mainWnd, (DLGPROC) StateDlgProc);
    GetClientRect(stateDlgWnd, &tailleStateDlgWnd);
    SetWindowPos(stateDlgWnd, NULL, tailleOptnDlgWnd.right, tailleMainWnd.bottom - (tailleStateDlgWnd.bottom - tailleStateDlgWnd.top), tailleMainWnd.right - tailleOptnDlgWnd.right, tailleStateDlgWnd.bottom - tailleStateDlgWnd.top, SWP_NOZORDER | SWP_SHOWWINDOW);
    GetClientRect(stateDlgWnd, &tailleStateDlgWnd);

    bilanWnd = CreateDialog(hInstance, "Bilan", NULL, (DLGPROC)BilanDlgProc);


    dragAndDropWnd = CreateWindowEx(
          WS_EX_ACCEPTFILES,
          "std",
          "",
          WS_CHILD | WS_VISIBLE,
          tailleOptnDlgWnd.right,
          0,
          tailleMainWnd.right - tailleOptnDlgWnd.right,
          tailleMainWnd.bottom - tailleStateDlgWnd.bottom,
          mainWnd,
          (HMENU) IDW_DRAGANDDROP,
          hInstance,
          NULL
       );

    HWND listWnd = CreateWindowEx(
          WS_EX_CLIENTEDGE,
          WC_LISTVIEW,
          "",
          WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_EDITLABELS,
          0,
          0,
          tailleMainWnd.right - tailleOptnDlgWnd.right,
          tailleMainWnd.bottom - tailleStateDlgWnd.bottom,
          dragAndDropWnd,
          (HMENU) IDLV_LIST,
          hInstance,
          NULL
       );

    strcpy(buffer, "Nom");
    LVCOLUMN lvcol;
    lvcol.mask = LVCF_WIDTH | LVCF_TEXT;
    lvcol.cx = 220;
    lvcol.pszText = buffer;
    lvcol.cchTextMax = strlen(buffer) + 1;
    ListView_InsertColumn(listWnd, IDCOL_FILENAME, &lvcol);

    lvcol.cx = 100;
    strcpy(buffer, "Type");
    lvcol.cchTextMax = strlen(buffer) + 1;
    ListView_InsertColumn(listWnd, IDCOL_FILETYPE, &lvcol);

    lvcol.cx = 80;
    strcpy(buffer, "Taille");
    lvcol.cchTextMax = strlen(buffer) + 1;
    ListView_InsertColumn(listWnd, IDCOL_FILESIZE, &lvcol);

    lvcol.cx = 140;
    strcpy(buffer, "Modifié le");
    lvcol.cchTextMax = strlen(buffer) + 1;
    ListView_InsertColumn(listWnd, IDCOL_FILEDATE, &lvcol);

    lvcol.cx = 200;
    strcpy(buffer, "Répertoire");
    lvcol.cchTextMax = strlen(buffer) + 1;
    ListView_InsertColumn(listWnd, IDCOL_DIR, &lvcol);


    CreateWindowEx(
            0,
            PROGRESS_CLASS,
            "",
            WS_CHILD | PBS_SMOOTH | WS_VISIBLE,
            15,
            62,
            300,
            20,
            stateDlgWnd,
            (HMENU) IDPB_FILE,
            hInstance,
            NULL
       );

    CreateWindowEx(
            0,
            PROGRESS_CLASS,
            "",
            WS_CHILD | PBS_SMOOTH | WS_VISIBLE,
            15,
            145,
            300,
            20,
            stateDlgWnd,
            (HMENU) IDPB_COPY,
            hInstance,
            NULL
       );

    SetWindowPos(GetDlgItem(stateDlgWnd, IDT_PARTSIZE), NULL, 345, 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(stateDlgWnd, IDE_PARTSIZE), NULL, 345, 50, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(stateDlgWnd, IDT_OCTET), NULL, 440, 50, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(stateDlgWnd, IDT_TIMER), NULL, 345, 85, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(stateDlgWnd, IDE_TIMER), NULL, 345, 115, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(stateDlgWnd, IDT_SECONDS), NULL, 440, 115, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(stateDlgWnd, IDT_SPEED), NULL, 345, 150, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(stateDlgWnd, IDT_REALSPEED), NULL, 345, 165, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(GetDlgItem(stateDlgWnd, IDT_MOYSPEED), NULL, 345, 180, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    create_toolTips();

    sprintf(buffer, "%.3lf", tempsTimer);
    SetDlgItemText(stateDlgWnd, IDE_TIMER, buffer);
    sprintf(buffer, "%d", tailleACopier);
    SetDlgItemText(stateDlgWnd, IDE_PARTSIZE, buffer);
    sprintf(buffer, "Vitesse voulue : %.3lf Mo/s", vitesseCopie / (1.0 * pow(2,20)));
    SetDlgItemText(stateDlgWnd, IDT_SPEED, buffer);

    SetTimer(mainWnd, TIMERID_SPEED, 1000, NULL);
    SetTimer(mainWnd, TIMERID_CONTROL, 1000, NULL);
    if (test_vide(PL_listFileName))
       SetTimer(mainWnd, TIMERID_PROTECTION, 500, NULL);
    SetTimer(mainWnd, TIMERID_SHORTCUTS, 50, NULL);

    MSG messages;
    while (GetMessage (&messages, NULL, 0, 0))
    {
        if (IsDialogMessage(optnDlgWnd, &messages) == 0)
        {
               TranslateMessage(&messages);
               DispatchMessage(&messages);
        }
    }

    CloseHandle(handle);
    return messages.wParam;
}





LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE];
    int Select, notification;
    FILE *file = NULL;

    switch (msg)
    {
        case WM_DROPFILES:
        {
             HDROP hDrop = (HDROP)wParam;
             int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0), i;
             for (i = 0 ; i < count ; i++)
             {
                 DragQueryFile(hDrop, i, buffer, MAX_CHAINE);
                 ajouter_fichier(buffer, 0, NULL);
             }
             DragFinish(hDrop);
             return FALSE;
        }
        case WM_CLOSE:
             if ((file = fopen(configFileName, "w")) != NULL)
             {
                  GetDlgItemText(optnDlgWnd, IDE_FILTER, buffer, MAX_CHAINE);
                  fprintf(file, "%s\n", buffer);

                  GetDlgItemText(optnDlgWnd, IDE_SEARCHDIR, buffer, MAX_CHAINE);
                  fprintf(file, "%s\n", buffer);

                  GetDlgItemText(optnDlgWnd, IDE_DIROUT, buffer, MAX_CHAINE);
                  fprintf(file, "%s\n", buffer);

                  fprintf(file, "%d\n", IsDlgButtonChecked(optnDlgWnd, IDC_NOCHANGEDIR));

                  fprintf(file, "%d\n", IsDlgButtonChecked(optnDlgWnd, IDC_SIZEMAX));
                  GetDlgItemText(optnDlgWnd, IDE_SIZEMAX, buffer, MAX_CHAINE);
                  fprintf(file, "%s\n", buffer);

                  fprintf(file, "%d\n", IsDlgButtonChecked(optnDlgWnd, IDC_TIMEMAX));
                  GetDlgItemText(optnDlgWnd, IDE_TIMEMAX, buffer, MAX_CHAINE);
                  fprintf(file, "%s\n", buffer);

                  fprintf(file, "%d\n", IsDlgButtonChecked(optnDlgWnd, IDC_READONLY));
                  fprintf(file, "%d\n", IsDlgButtonChecked(optnDlgWnd, IDC_HIDDEN));
                  fprintf(file, "%d\n", IsDlgButtonChecked(optnDlgWnd, IDC_SYSTEM));
                  fprintf(file, "%d\n", IsDlgButtonChecked(optnDlgWnd, IDC_ARCHIVE));
                  fprintf(file, "%d\n", IsDlgButtonChecked(optnDlgWnd, IDC_REPLACE));

                  fprintf(file, "%s\n", protectionPassword);

                  fprintf(file, "%d\n%lf\n", tailleACopier, tempsTimer);
                  fprintf(file, "%d\n%d\n%d\n", cassSensitiveResearch, wildcardsResearch, recursiveResearch);
                  fprintf(file, "%d\n%d\n", dirsResearch, filesResearch);
                  fclose(file);
             }

             if (protection)
                ShowWindow(hwnd, SW_HIDE);
             else
             {
                 if (MessageBox(hwnd, "Voulez-fermer réellement le programme ?\nCliquez sur Oui pour fermer le programme\nou sur Non pour simplement masquer la fenêtre.", "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDYES)
                    DestroyWindow(hwnd);
                 else ShowWindow(hwnd, SW_HIDE);
             }
             return FALSE;
        case WM_DESTROY:
             PostQuitMessage (0);
             break;
        /*case WM_PAINT:
        {
             hdc = BeginPaint(hwnd, &paintst);

             HWND progressBarWnd = GetDlgItem(stateDlgWnd, IDPB_FILE);
             RECT tailleProgressBar;
             GetWindowRect(progressBarWnd, &tailleProgressBar);
             RECT rect;
             GetWindowRect(mainWnd, &rect);
             int percent = tailleFichiersCopies * 1.0 / tailleTotaleFichiersACopier * 100;
             sprintf(buffer, "%d %%", percent);
             TextOut(hdc, (tailleProgressBar.right - tailleProgressBar.left) / 2 - rect.left - edge + tailleProgressBar.left, tailleProgressBar.top - rect.top - caption - edge + 1, buffer, strlen(buffer));

             progressBarWnd = GetDlgItem(stateDlgWnd, IDPB_COPY);
             GetWindowRect(progressBarWnd, &tailleProgressBar);
             percent = tailleFichiersCopies * 1.0 / tailleTotaleFichiersACopier * 100;

             EndPaint(hwnd, &paintst);
        }*/
        case WM_NOTIFY:
        {
             Select = LOWORD(wParam);
             switch (Select)
             {
                    case IDLV_LIST:
                    {
                         HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);

                         NMITEMACTIVATE *nmia;
                         nmia = (NMITEMACTIVATE*) lParam;

                         notification = nmia->hdr.code;
                         switch (notification)
                         {
                                case NM_RCLICK:
                                {
                                     if (nmia->iSubItem != IDCOL_FILENAME)
                                        return FALSE;

                                     char name[MAX_CHAINE];
                                     itemSelectionne = nmia->iItem;
                                     ListView_GetItemText(listWnd, nmia->iItem, IDCOL_FILENAME, name, MAX_CHAINE)
                                     int i;
                                     for (i = 0 ; name[i] != '\0' ; i++)
                                         name[i] = toupper(name[i]);

                                     char directory[MAX_CHAINE];
                                     ListView_GetItemText(listWnd, itemSelectionne, IDCOL_DIR, directory, MAX_CHAINE);
                                     sprintf(nomItemSelectionne, "%s\\%s", directory, name);


                                     HMENU menu = CreatePopupMenu();

                                     if (ListView_GetSelectedCount(listWnd) > 0)
                                     {
                                         AppendMenu(menu, MF_STRING, ITEM_OPEN, "Ouvrir");
                                         char *position = NULL;
                                         if (!is_item_directory(nmia->iItem) && (position = strrchr(name, '.')) != NULL)
                                         {
                                             if (strcmp(position, ".MP3") == 0 ||
                                                 strcmp(position, ".WAV") == 0 ||
                                                 strcmp(position, ".WMA") == 0 ||
                                                 strcmp(position, ".OGG") == 0 ||
                                                 strcmp(position, ".AAC") == 0)
                                               AppendMenu(menu, MF_STRING, ITEM_PLAY, "Lire");
                                         }

                                         if (is_item_directory(nmia->iItem))
                                         {
                                             HMENU subMenu = CreatePopupMenu();
                                             AppendMenu(subMenu, MF_STRING, ITEM_ADDSEARCHDIR, "Répertoire de recherche");
                                             AppendMenu(subMenu, MF_STRING, ITEM_ADDOUTDIR, "Répertoire de sortie");

                                             AppendMenu(menu, MF_POPUP, (UINT)subMenu, "Définir comme");
                                         }

                                         if (stream != NULL)
                                            AppendMenu(menu, MF_STRING, ITEM_STOP, "Stop");

                                         AppendMenu(menu, MF_SEPARATOR, 0, NULL);
                                         if (!is_item_directory(nmia->iItem))
                                         {
                                             AppendMenu(menu, MF_STRING, ITEM_COPY, "Copier");
                                             AppendMenu(menu, MF_STRING, ITEM_MOVE, "Déplacer");
                                         }
                                         AppendMenu(menu, MF_STRING, ITEM_DELETE, "Corbeille");
                                         AppendMenu(menu, MF_SEPARATOR, 0, NULL);
                                         AppendMenu(menu, MF_STRING, ITEM_CLEAR, "Supprimer de la liste");
                                     }

                                     if (ListView_GetSelectedCount(listWnd) <= 0 && stream != NULL)
                                     {
                                         AppendMenu(menu, MF_STRING, ITEM_STOP, "Stop");
                                         if (ListView_GetItemCount(listWnd) > 0)
                                            AppendMenu(menu, MF_SEPARATOR, 0, NULL);
                                     }

                                     if (ListView_GetItemCount(listWnd) > 0)
                                        AppendMenu(menu, MF_STRING, ITEM_CLEARALL, "Vider la liste");

                                     POINT pt;
                                     GetCursorPos(&pt);
                                     TrackPopupMenu(menu, 0, pt.x, pt.y, 0, mainWnd, NULL);

                                     return FALSE;
                                }
                                case LVN_BEGINLABELEDIT:
                                {
                                     NMLVDISPINFO *nmlvdi;
                                     nmlvdi = (NMLVDISPINFO*) lParam;
                                     HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);

                                     char name[MAX_CHAINE], directory[MAX_CHAINE];
                                     ListView_GetItemText(listWnd, nmlvdi->item.iItem, IDCOL_FILENAME, name, MAX_CHAINE);
                                     ListView_GetItemText(listWnd, nmlvdi->item.iItem, IDCOL_DIR, directory, MAX_CHAINE);
                                     sprintf(nomItemSelectionne, "%s\\%s", directory, name);
                                     strcpy(oldTextItem, name);

                                     return FALSE;
                                }
                                case LVN_ENDLABELEDIT:
                                {
                                     NMLVDISPINFO *nmlvdi;
                                     nmlvdi = (NMLVDISPINFO*) lParam;
                                     HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);

                                     char *extension = strrchr(oldTextItem, '.'),
                                          *nouvelleExtension = NULL,
                                          adress[MAX_CHAINE] = "\0";

                                     if (nmlvdi->item.pszText == NULL)
                                        return FALSE;

                                     ListView_GetItemText(listWnd, nmlvdi->item.iItem, IDCOL_DIR, adress, MAX_CHAINE);
                                     sprintf(buffer, "%s\\%s", adress, nmlvdi->item.pszText);

                                     if (strcmp(nomItemSelectionne, buffer) == 0)
                                        return FALSE;

                                     if (extension != NULL)
                                     {
                                         nouvelleExtension = strrchr(nmlvdi->item.pszText, '.');
                                         if (nouvelleExtension == NULL || strcmp(nouvelleExtension, extension) != 0)
                                         {
                                               if (MessageBox(mainWnd, "Voulez-vous vraiment changer l'extension de ce fichier ?\nCela peut empêcher ou gêner son utilisation.", "Attention", MB_ICONWARNING | MB_YESNO) == IDYES)
                                               {
                                                  if (rename(nomItemSelectionne, buffer))
                                                     add_bilan_text("Renommage par édition", "Impossible de renommer le fichier", NULL, nomItemSelectionne, NULL, ABT_GETERRNO);
                                                  else return TRUE;
                                               }
                                               else return FALSE;
                                         }
                                     }

                                     if (rename(nomItemSelectionne, buffer))
                                        add_bilan_text("Renommage par édition", "Impossible de renommer le fichier", NULL, nomItemSelectionne, NULL, ABT_GETERRNO);
                                     else return TRUE;
                                }
                                case LVN_COLUMNCLICK:
                                {
                                     NMLISTVIEW *nmlv = (NMLISTVIEW*) lParam;
                                     HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);
                                     nombreTotalItems = ListView_GetItemCount(listWnd);
                                     if (nombreTotalItems > MAX_ITEMS)
                                        nombreTotalItems = MAX_ITEMS;

                                     int i;
                                     for (i = 0 ; i < nombreTotalItems ; i++)
                                     {
                                         tableauTotalItems[i].lvi.mask = LVIF_PARAM | LVIF_IMAGE;
                                         tableauTotalItems[i].lvi.iItem = i;
                                         tableauTotalItems[i].lvi.iSubItem = nmlv->iSubItem;
                                         ListView_GetItem(listWnd, &(tableauTotalItems[i].lvi));
                                         ListView_GetItemText(listWnd, i, nmlv->iSubItem, tableauTotalItems[i].pszText, MAX_CHAINE);
                                     }

                                     ListView_SortItems(listWnd, (PFNLVCOMPARE)CompareFunction, nmlv->iSubItem);
                                     return FALSE;
                                }
                                case LVN_KEYDOWN:
                                {
                                     NMLVKEYDOWN *pnkd = (LPNMLVKEYDOWN) lParam;

                                     int *tab = NULL;
                                     if (get_selected_items(pnkd->hdr.hwndFrom, &tab) <= 0)
                                        return FALSE;
                                     itemSelectionne = tab[0];

                                     char name[MAX_CHAINE];
                                     ListView_GetItemText(listWnd, itemSelectionne, IDCOL_FILENAME, name, MAX_CHAINE)

                                     char directory[MAX_CHAINE];
                                     ListView_GetItemText(listWnd, itemSelectionne, IDCOL_DIR, directory, MAX_CHAINE);
                                     sprintf(nomItemSelectionne, "%s\\%s", directory, name);

                                     free(tab);

                                     switch (pnkd->wVKey)
                                     {
                                            case VK_DELETE:
                                                 SendMessage(hwnd, WM_COMMAND, M_LIST_DELETE, 0);
                                                 return FALSE;
                                            case VK_RETURN:
                                                 SendMessage(hwnd, WM_COMMAND, ITEM_OPEN, 0);
                                                 return FALSE;
                                            case VK_SPACE:
                                                 ListView_EditLabel(pnkd->hdr.hwndFrom, itemSelectionne);
                                                 return FALSE;
                                            case 'A':
                                            {
                                                 if (!HIBYTE(GetKeyState(VK_CONTROL)))
                                                    return FALSE;

                                                 LVITEM lvi;
                                                 lvi.iSubItem = 0;
                                                 lvi.iItem = -1;
                                                 lvi.state = LVIS_SELECTED;
                                                 lvi.stateMask = LVIS_SELECTED;
                                                 lvi.mask = LVIF_STATE;
                                                 SendMessage(pnkd->hdr.hwndFrom, LVM_SETITEMSTATE, -1, (LPARAM)&lvi);

                                                 return FALSE;
                                            }
                                     }

                                     return FALSE;
                                }
                         }
                         return FALSE;
                    }
             }

             NMHDR *nmhdr = (NMHDR*)lParam;

             switch (nmhdr->code)
             {
                 case TTN_SHOW:
                      return FALSE;
                 case TTN_POP:
                      create_toolTips();
                      return FALSE;
             }

             return FALSE;
        }
        case WM_COMMAND:
             Select = LOWORD(wParam);
             switch (Select)
             {
                    case ITEM_OPEN:
                         ShellExecute(hwnd, "open", nomItemSelectionne, NULL, NULL, SW_SHOW);
                         return FALSE;
                    case ITEM_PLAY:
                         if (stream != NULL)
                         {
                            FSOUND_Stream_Stop(stream);
                            FSOUND_Stream_Close(stream);
                         }

                         stream = FSOUND_Stream_Open(nomItemSelectionne, FSOUND_LOOP_OFF | FSOUND_NONBLOCKING | FSOUND_MPEGACCURATE, 0, 0);
                         SetTimer(mainWnd, TIMERID_PLAY, 1000, NULL);
                         return FALSE;
                    case ITEM_ADDSEARCHDIR:
                         SetDlgItemText(optnDlgWnd, IDE_SEARCHDIR, nomItemSelectionne);
                         return FALSE;
                    case ITEM_ADDOUTDIR:
                         SetDlgItemText(optnDlgWnd, IDE_DIROUT, nomItemSelectionne);
                         return FALSE;
                    case ITEM_STOP:
                         if (stream == NULL)
                            return FALSE;
                         FSOUND_Stream_Stop(stream);
                         FSOUND_Stream_Close(stream);
                         stream = NULL;

                         return FALSE;
                    case M_LIST_DELETEALL:
                    case ITEM_CLEARALL:
                         if (MessageBox(hwnd, "Suppression de tous les items de la liste.\nCette opération est irréversible.\nContinuer ?", "Attention", MB_ICONWARNING | MB_YESNO) == IDYES)
                            ListView_DeleteAllItems(GetDlgItem(dragAndDropWnd, IDLV_LIST));
                         return FALSE;
                    case M_LIST_DELETE:
                    case ITEM_CLEAR:
                    {
                         HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);

                         int *selectedItems = NULL;
                         int numberOfSelectedItems = get_selected_items(listWnd, &selectedItems);

                         if (numberOfSelectedItems > 1)
                            sprintf(buffer, "Voulez-vous vraiment supprimer de la liste ces %d éléments ?", numberOfSelectedItems);
                         else if (numberOfSelectedItems == 1)
                              strcpy(buffer, "Voulez-vous vraiment supprimer de la liste cet élément ?");
                         else return FALSE;

                         if (MessageBox(hwnd, buffer, "Confirmation", MB_ICONQUESTION | MB_YESNO) != IDYES)
                            return FALSE;

                         int i;
                         for (i = 0 ; i < numberOfSelectedItems ; i++)
                             ListView_DeleteItem(listWnd, selectedItems[i]);

                         free(selectedItems);
                         attrib_lParams(listWnd);
                         return FALSE;
                    }
                    case ITEM_COPY:
                         onlySelected = 1;
                         SendMessage(GetDlgItem(optnDlgWnd, IDP_COPY), BM_CLICK, 0, 0);
                         return FALSE;
                    case ITEM_MOVE:
                         onlySelected = 1;
                         SendMessage(GetDlgItem(optnDlgWnd, IDP_MOVE), BM_CLICK, 0, 0);
                         return FALSE;
                    case ITEM_DELETE:
                         onlySelected = 1;
                         SendMessage(GetDlgItem(optnDlgWnd, IDP_DELETE), BM_CLICK, 0, 0);
                         return FALSE;
                    case M_FILE_CLOSE:
                         CloseWindow(hwnd);
                         return FALSE;
                    case M_ACTION_COPY:
                    case M_ACTION_MOVE:
                    case M_ACTION_DELETE:
                    case M_ACTION_RENAME:
                    case M_ACTION_ENCRYPT:
                    case M_ACTION_DECRYPT:
                    case M_ACTION_PROTECT:
                         if (ListView_GetSelectedCount(GetDlgItem(dragAndDropWnd, IDLV_LIST)) <= 0)
                            onlySelected = 0;
                         else
                         {
                             if (MessageBox(mainWnd, "Traiter aussi les fichiers non sélectionnés ?", "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
                                onlySelected = 1;
                             else onlySelected = 0;
                         }

                         if (Select == M_ACTION_COPY)
                            SendMessage(optnDlgWnd, WM_COMMAND, IDP_COPY, 0);
                         else if (Select == M_ACTION_MOVE)
                              SendMessage(optnDlgWnd, WM_COMMAND, IDP_MOVE, 0);
                         else if (Select == M_ACTION_DELETE)
                              SendMessage(optnDlgWnd, WM_COMMAND, IDP_DELETE, 0);
                         else if (Select == M_ACTION_RENAME)
                              SendMessage(optnDlgWnd, WM_COMMAND, IDP_RENAME, 0);
                         else if (Select == M_ACTION_ENCRYPT)
                              SendMessage(optnDlgWnd, WM_COMMAND, IDP_ENCRYPT, 0);
                         else if (Select == M_ACTION_DECRYPT)
                              SendMessage(optnDlgWnd, WM_COMMAND, IDP_DECRYPT, 0);
                         else if (Select == M_ACTION_PROTECT)
                              SendMessage(optnDlgWnd, WM_COMMAND, IDP_PROTECT, 0);

                         return FALSE;
                    case M_ACTION_DISPROTECT:
                         SendMessage(optnDlgWnd, WM_COMMAND, IDP_DISPROTECT, 0);
                         return FALSE;
                    case M_LIST_SORT:
                    {
                         HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);
                         nombreTotalItems = ListView_GetItemCount(listWnd);
                         if (nombreTotalItems > MAX_ITEMS)
                            nombreTotalItems = MAX_ITEMS;

                         int i;
                         for (i = 0 ; i < nombreTotalItems ; i++)
                         {
                             tableauTotalItems[i].lvi.mask = LVIF_PARAM | LVIF_IMAGE;
                             tableauTotalItems[i].lvi.iItem = i;
                             tableauTotalItems[i].lvi.iSubItem = IDCOL_FILENAME;
                             ListView_GetItem(listWnd, &(tableauTotalItems[i].lvi));
                             ListView_GetItemText(listWnd, i, IDCOL_FILENAME, tableauTotalItems[i].pszText, MAX_CHAINE);
                         }

                         ListView_SortItems(listWnd, (PFNLVCOMPARE)CompareFunction, IDCOL_FILENAME);
                         return FALSE;
                    }
                    case M_LIST_REFRESH:
                    {
                         char name[MAX_CHAINE];
                         HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);
                         int number = ListView_GetItemCount(listWnd), i;

                         for (i = 0 ; i < number ; i++)
                         {
                             ListView_GetItemText(listWnd, i, IDCOL_DIR, name, MAX_CHAINE);
                             ListView_GetItemText(listWnd, i, IDCOL_FILENAME, buffer, MAX_CHAINE);
                             sprintf(name, "%s\\%s", name, buffer);
                             if (!test_exist(name))
                                ListView_DeleteItem(listWnd, i);
                             else ajouter_fichier(name, 1, NULL);
                         }

                         return FALSE;
                    }
                    case M_FILE_OPEN:
                    {
                         OPENFILENAME ofn;

                         memset(&ofn, 0, sizeof(OPENFILENAME));
                         ofn.lStructSize = sizeof(OPENFILENAME);
                         ofn.hwndOwner = hwnd;
                         ofn.nMaxFile = MAX_CHAINE;
                         ofn.lpstrFilter = "Fichiers texte  (*.txt)\0*.txt\0Fichiers liste  (*.list)\0*.list\0Tous les fichiers  (*.*)\0*.*\0\0";
                         ofn.lpstrFile = buffer;
                         ofn.lpstrTitle = "Ouvrir une liste";
                         ofn.Flags = OFN_FILEMUSTEXIST;
                         ofn.lpstrInitialDir = NULL;

                         if (!GetOpenFileName(&ofn))
                            return FALSE;

                         int count = compter_lignes(buffer);
                         if (count <= 0)
                         {
                             MessageBox(mainWnd, "Liste invalide.", "Erreur !", MB_OK | MB_ICONERROR);
                             return FALSE;
                         }

                         char text[MAX_CHAINE] = "\0";
                         sprintf(text, "Cette liste contient environ %d éléments. Voulez-vous...\n=> Les ajouter à la liste actuelle ? (oui)\n=> Remplacer la liste actuelle ? (non)", count);
                         if (MessageBox(mainWnd, text, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
                            ListView_DeleteAllItems(GetDlgItem(dragAndDropWnd, IDLV_LIST));

                         ShowWindow(mainWnd, SW_HIDE);
                         ajouter_liste(buffer, NULL);
                         ShowWindow(mainWnd, SW_SHOW);

                         return FALSE;
                    }
                    case M_FILE_SAVE:
                    {
                         OPENFILENAME ofn;

                         memset(&ofn, 0, sizeof(OPENFILENAME));
                         ofn.lStructSize = sizeof(OPENFILENAME);
                         ofn.hwndOwner = hwnd;
                         ofn.nMaxFile = MAX_CHAINE;
                         ofn.lpstrFilter = "Fichiers texte  (*.txt)\0*.txt\0Fichiers liste  (*.list)\0*.list\0Tous les fichiers  (*.*)\0*.*\0\0";
                         ofn.lpstrFile = buffer;
                         ofn.lpstrTitle = "Enregistrer sous";
                         ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
                         ofn.lpstrInitialDir = NULL;
                         ofn.lpstrDefExt = ".txt";

                         if (!GetSaveFileName(&ofn))
                            return FALSE;

                         FILE *file = fopen(buffer, "w");
                         if (file == NULL)
                         {
                             strcat(buffer, "\nImpossible de créer le fichier.");
                             MessageBox(mainWnd, buffer, "Erreur !", MB_OK);
                             return FALSE;
                         }

                         HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);

                         if (ListView_GetSelectedCount(listWnd) <= 0)
                            onlySelected = 0;
                         else
                         {
                             if (MessageBox(mainWnd, "Traiter aussi les fichiers non sélectionnés ?", "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
                                onlySelected = 1;
                             else onlySelected = 0;
                         }

                         int count = ListView_GetItemCount(listWnd), i;
                         char name[MAX_CHAINE] = "\0", directory[MAX_CHAINE] = "\0";

                         for (i = 0 ; i < count ; i++)
                         {
                             if (!onlySelected || is_item_selected(listWnd, i))
                             {
                                 ListView_GetItemText(listWnd, i, IDCOL_FILENAME, name, MAX_CHAINE);
                                 ListView_GetItemText(listWnd, i, IDCOL_DIR, directory, MAX_CHAINE);
                                 sprintf(buffer, "%s\\%s\n", directory, name);
                                 fputs(buffer, file);
                             }
                         }

                         fclose(file);
                         MessageBox(mainWnd, "Liste créée avec succès !", "Information", MB_OK | MB_ICONINFORMATION);

                         return FALSE;
                    }
             }

             return FALSE;
        case WM_TIMER:
             Select = LOWORD(wParam);
             switch (Select)
             {
                    case TIMERID_PLAY:
                         if (stream == NULL)
                         {
                            KillTimer(mainWnd, TIMERID_PLAY);
                            return FALSE;
                         }

                         if (FSOUND_Stream_GetOpenState(stream) != 0)
                                return FALSE;

                         FSOUND_Stream_Play(FSOUND_FREE, stream);
                         KillTimer(mainWnd, TIMERID_PLAY);
                         return FALSE;
                    case TIMERID_SPEED:
                         if (!isCopying)
                            return FALSE;

                         vitesseReelleCopie = (tailleFichiersCopies - derniereTailleCopiee) * pow(2,10);
                         derniereTailleCopiee = tailleFichiersCopies;
                         return FALSE;
                    case TIMERID_COPY:
                    {
                         if (timerCopyActive)
                            return FALSE;
                         timerCopyActive = 1;

                         isCopying = 1;

                         char typeError[MAX_CHAINE];
                         if (isEncrypting)
                            strcpy(typeError, "Décryptage - copie");
                         else if (isEncrypting)
                              strcpy(typeError, "Cryptage - copie");
                         else if (isMoving)
                              strcpy(typeError, "Déplacement - copie");
                         else strcpy(typeError, "Copie");


                         if (fichierACopier == NULL || copieDuFichier == NULL)
                         {
                             if (fichierACopier != NULL)
                                fclose(fichierACopier);
                             if (copieDuFichier != NULL)
                                fclose(copieDuFichier);

                             numeroFichier++;
                             if (numeroFichier >= MAX_ITEMS || tableauFichiersACopier[numeroFichier] == NULL)
                             {
                                 KillTimer(hwnd, TIMERID_COPY);
                                 isCopying = 0;

                                 if (!isMoving && !isEncrypting)
                                 {
                                     free_table();
                                     enable_buttons(TRUE);
                                 }

                                 fichierACopier = NULL;
                                 copieDuFichier = NULL;

                                 tailleFichiersCopies = 0;
                                 numeroFichier = 0;
                                 timerCopyActive = 0;
                                 return FALSE;
                             }

                             if ((fichierACopier = fopen(tableauFichiersACopier[numeroFichier], "rb")) == NULL)
                                add_bilan_text(typeError, "Impossible d'ouvrir le fichier source", NULL, tableauFichiersACopier[numeroFichier], NULL, ABT_GETERRNO);
                             if (tableauFichiersACopier[0][0] != '\0')
                             {
                                strcpy(buffer, tableauFichiersACopier[numeroFichier]);
                                correct_adress(buffer, tableauFichiersACopier[0]);
                             }
                             else strcpy(buffer, tableauFichiersACopier[numeroFichier]);

                             if (isDecrypting)
                             {
                                if (deleteOldExtension)
                                   delete_second_extension(buffer);
                                else if (encryptionExtension[0] != '\0')
                                     sprintf(buffer, "%s.%s", buffer, encryptionExtension);
                             }
                             else if (isEncrypting && encryptionExtension[0] != '\0')
                                sprintf(buffer, "%s.%s", buffer, encryptionExtension);

                             if (strcmp(buffer, tableauFichiersACopier[numeroFichier]) == 0)
                                adjust_file_name(buffer);

                             if (strcmp(buffer, tableauFichiersACopier[numeroFichier]) != 0)
                             {
                                if ((copieDuFichier = fopen(buffer, "wb")) == NULL)
                                   add_bilan_text(typeError, "Impossible d'ouvrir le fichier cible", NULL, buffer, NULL, ABT_GETERRNO);
                             }
                             else
                             {
                                  copieDuFichier = NULL;
                                  add_bilan_text(typeError, "Fichiers source et cible identiques.", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);
                             }
                             strcpy(nomFichierACopier, buffer);

                             if (fichierACopier != NULL)
                             {
                                 fseek(fichierACopier, 0, SEEK_END);
                                 tailleFichierACopier = ftell(fichierACopier) / (1.0 * pow(2,10));
                                 if (isEncrypting)
                                    CRYPT_reset_count();
                                 rewind(fichierACopier);
                             }


                             if (isEncrypting && !isDecrypting && copieDuFichier != NULL)
                             {
                                 int tailleHeader = CRYPT_encoder_header(password, buffer, MAX_CHAINE);
                                 if (fwrite(buffer, tailleHeader, 1, copieDuFichier) <= 0)
                                    add_bilan_text("Cryptage", "Impossible d'écrire le header", NULL, tableauFichiersACopier[numeroFichier], NULL, ABT_GETERRNO);
                             }

                             if (isDecrypting && fichierACopier != NULL && !CRYPT_tester_header(fichierACopier, password) && copieDuFichier != NULL)
                             {
                                  fclose(copieDuFichier);
                                  remove(nomFichierACopier);
                                  copieDuFichier = NULL;
                                  add_bilan_text("Décryptage", "Mot de passe incorrect", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);
                             }

                             timerCopyActive = 0;
                             return FALSE;
                         }

                         void *ptv = malloc(tailleACopier * pow(2,10));
                         int bytesCopied = fread(ptv, 1, tailleACopier * pow(2,10), fichierACopier);
                         int readError = 0;
                         if (bytesCopied <= 0)
                         {
                             int errNumber = errno;
                             sprintf(buffer, "Erreur de lecture au bit %ld", ftell(fichierACopier));
                             add_bilan_text(typeError, buffer, (char*)errNumber, tableauFichiersACopier[numeroFichier], NULL, ABT_ISERRNO);
                             readError = 1;
                         }

                         tailleFichiersCopies += bytesCopied / (1.0 * pow(2,10));
                         int pos = ftell(fichierACopier);

                         //remise à jour des informations
                         int percent = ftell(fichierACopier) / (1.0 * pow(2,10)) / tailleFichierACopier * 100;
                         draw_progress_bar(percent, 1);

                         sprintf(buffer, "Fichier : %s", strrchr(tableauFichiersACopier[numeroFichier], '\\') + 1);
                         SetDlgItemText(stateDlgWnd, IDT_FILENAME, buffer);
                         sprintf(buffer, "[%.0lf / %.0lf ko]", pos / (1.0 * pow(2,10)), tailleFichierACopier);
                         SetDlgItemText(stateDlgWnd, IDT_SIZE, buffer);

                         percent = tailleFichiersCopies * 1.0 / tailleTotaleFichiersACopier * 100;
                         draw_progress_bar(percent, 2);

                         if (isDecrypting)
                            sprintf(buffer, "Décryptage du fichier %d / %d", numeroFichier, nombreFichiersACopier);
                         else if (isEncrypting)
                            sprintf(buffer, "Cryptage du fichier %d / %d", numeroFichier, nombreFichiersACopier);
                         else sprintf(buffer, "Copie du fichier %d / %d", numeroFichier, nombreFichiersACopier);
                         SetDlgItemText(stateDlgWnd, IDT_FILENUMBER, buffer);
                         sprintf(buffer, "[%.0lf / %.0lf ko]", tailleFichiersCopies, tailleTotaleFichiersACopier);
                         SetDlgItemText(stateDlgWnd, IDT_TOTALSIZE, buffer);

                         if (isDecrypting)
                            CRYPT_decrypter(ptv, bytesCopied, password, CRYPT_CONTINUATE);
                         else if (isEncrypting)
                            CRYPT_crypter(ptv, bytesCopied, password, CRYPT_CONTINUATE);

                         bytesCopied = fwrite(ptv, bytesCopied, 1, copieDuFichier);
                         if (bytesCopied <= 0 && !readError)
                         {
                             int errNumber = errno;
                             sprintf(buffer, "Erreur d'écriture au bit %d", pos);
                             add_bilan_text(typeError, buffer, (char*)errNumber, tableauFichiersACopier[numeroFichier], NULL, ABT_ISERRNO);
                             readError = 1;
                         }

                         int tempsTotalCopie = GetTickCount() - heureDebutCopie;
                         int vitesseMoyenneCopie = tailleFichiersCopies * 1.0 * pow(2,10) / (tempsTotalCopie / pow(2,10));
                         if (vitesseReelleCopie <= 0)
                            vitesseReelleCopie = vitesseMoyenneCopie;

                         sprintf(buffer, "Temps restant estimé : %.0lf sec", (tailleFichierACopier * pow(2,10) - pos) / vitesseMoyenneCopie);
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFT, buffer);

                         sprintf(buffer, "Temps restant estimé : %.0lf sec", (tailleTotaleFichiersACopier - tailleFichiersCopies) * pow(2,10) / vitesseMoyenneCopie);
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFTCOPY, buffer);

                         sprintf(buffer, "Vitesse réelle : %.3lf Mo/s", vitesseReelleCopie / (1.0 * pow(2,20)));
                         SetDlgItemText(stateDlgWnd, IDT_REALSPEED, buffer);

                         sprintf(buffer, "Vitesse moyenne : %.3lf Mo/s", vitesseMoyenneCopie / (1.0 * pow(2,20)));
                         SetDlgItemText(stateDlgWnd, IDT_MOYSPEED, buffer);

                         sprintf(buffer, "Temps écoulé : %.3lf sec", (GetTickCount() - heureDebutCopie) / 1000.0);
                         SetDlgItemText(stateDlgWnd, IDT_TIMECOPY, buffer);


                         if (feof(fichierACopier) ||
                             bytesCopied <= 0 ||
                            (tailleMaxCopie > 0 && tailleFichiersCopies > tailleMaxCopie) ||
                            (tempsMaxCopie > 0 && tempsMaxCopie < GetTickCount() - heureDebutCopie) ||
                             stopNow)
                         {
                              strcpy(buffer, tableauFichiersACopier[numeroFichier]);
                              fclose(fichierACopier);
                              fclose(copieDuFichier);

                              fichierACopier = NULL;
                              copieDuFichier = NULL;

                              if (tailleMaxCopie > 0 && tailleFichiersCopies > tailleMaxCopie)
                              {
                                  add_bilan_text(typeError, "Taille maximale atteinte", "\0", buffer, NULL, 0);
                                  numeroFichier = MAX_ITEMS;
                              }
                              if (tempsMaxCopie > 0 && tempsMaxCopie < GetTickCount() - heureDebutCopie)
                              {
                                  add_bilan_text(typeError, "Temps imparti écoulé", "\0", buffer, NULL, 0);
                                  numeroFichier = MAX_ITEMS;
                              }
                              if (stopNow)
                              {
                                  add_bilan_text(typeError, "Opération annulée par l'utilisateur", "\0", buffer, NULL, 0);
                                  numeroFichier = MAX_ITEMS;
                                  stopNow = 0;
                              }

                              ajouter_fichier(nomFichierACopier, 0, NULL);
                         }

                         free(ptv);
                         timerCopyActive = 0;
                         return FALSE;
                    }
                    case TIMERID_MOVE:
                    {
                         if (timerMoveActive)
                            return FALSE;
                         timerMoveActive = 1;

                         isMoving = 1;
                         int i;

                         if (etapeDeplacement == 0 && !isCopying)
                         {
                            SetTimer(mainWnd, TIMERID_COPY, tempsTimer * 1000, NULL);
                            isCopying = 1;
                            etapeDeplacement = 1;
                            timerMoveActive = 0;
                            return FALSE;
                         }

                         if (isCopying && etapeDeplacement == 1)
                         {
                               timerMoveActive = 0;
                               return FALSE;
                         }
                         else if (etapeDeplacement == 1)
                         {
                              numeroFichier = 0;
                              isDeleting = 1;
                              etapeDeplacement = 2;
                         }

                         numeroFichier++;
                         if (numeroFichier >= MAX_ITEMS)
                         {
                              KillTimer(hwnd, TIMERID_MOVE);

                              for (i = 0 ; i < MAX_ITEMS && tableauDossiersACopier[i] != NULL ; i++)
                              {
                                  strcpy(buffer, tableauDossiersACopier[i]);
                                  correct_adress(buffer, tableauFichiersACopier[0]);
                                  if (test_exist(buffer) == 2)
                                  {
                                      if (!RemoveDirectory(tableauDossiersACopier[i]))
                                         add_bilan_text("Déplacement - suppression", "Impossible de supprimer le dossier", NULL, tableauDossiersACopier[i], NULL, ABT_GETLASTERROR);
                                  }
                              }

                              enable_buttons(TRUE);

                              free_table();

                              isMoving = 0;
                              isDeleting = 0;

                              timerMoveActive = 0;
                              return FALSE;
                         }


                         if (tableauFichiersACopier[numeroFichier] == NULL)
                         {
                              if (etapeDeplacement == 2)
                              {
                                   isDeleting = 0;
                                   etapeDeplacement = 3;
                              }
                              else numeroFichier = MAX_ITEMS;
                              timerMoveActive = 0;
                              return FALSE;
                         }

                         HWND progressBarWnd = GetDlgItem(stateDlgWnd, IDPB_FILE);
                         SendMessage(progressBarWnd, PBM_SETPOS, 0, 0);

                         if (isDeleting)
                         {
                            if (!DeleteFile(tableauFichiersACopier[numeroFichier]))
                               add_bilan_text("Déplacement - suppression", "Impossible de supprimer le fichier", NULL, tableauFichiersACopier[numeroFichier], NULL, ABT_GETLASTERROR);
                            else delete_item(tableauFichiersACopier[numeroFichier]);
                         }
                         else
                         {
                             strcpy(buffer, tableauFichiersACopier[numeroFichier]);
                             correct_adress(buffer, tableauFichiersACopier[0]);

                             if (!MoveFileEx(tableauFichiersACopier[numeroFichier], buffer, MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING))
                                add_bilan_text("Déplacement", "Impossible de déplacer le fichier", NULL, tableauFichiersACopier[numeroFichier], NULL, ABT_GETLASTERROR);
                             else
                             {
                                ajouter_fichier(buffer, 0, NULL);
                                delete_item(tableauFichiersACopier[numeroFichier]);
                             }
                         }

                         SendMessage(progressBarWnd, PBM_SETPOS, 100, 0);
                         sprintf(buffer, "Fichier : %s", strrchr(tableauFichiersACopier[numeroFichier], '\\') + 1);
                         SetDlgItemText(stateDlgWnd, IDT_FILENAME, buffer);
                         SetDlgItemText(stateDlgWnd, IDT_SIZE, "[--- / ---] ko");
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFT, "Temps restant estimé : inconnu");


                         for (i = 1 ; i < MAX_ITEMS - 1 && (tableauFichiersACopier[i] != NULL || tableauFichiersACopier[i + 1] != NULL) ; i++);
                         i -= 2;
                         int percent = 0;
                         if (isDeleting)
                         {
                            percent = numeroFichier * 100.0 / i;
                            sprintf(buffer, "Suppression du fichier %d / %d", numeroFichier, i);
                         }
                         else
                         {
                             percent = (numeroFichier - 1) * 100.0 / i;
                             sprintf(buffer, "Déplacement du fichier %d / %d", numeroFichier - 1, i);
                         }
                         SetDlgItemText(stateDlgWnd, IDT_FILENUMBER, buffer);

                         draw_progress_bar(percent, 2);
                         SetDlgItemText(stateDlgWnd, IDT_TOTALSIZE, "[--- / --- ko]");
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFTCOPY, "Temps restant estimé : inconnu");

                         sprintf(buffer, "Temps écoulé : %.3lf sec", (GetTickCount() - heureDebutCopie) / 1000.0);
                         SetDlgItemText(stateDlgWnd, IDT_TIMECOPY, buffer);

                         if (stopNow)
                         {
                             add_bilan_text("Déplacement", "Opération annulée par l'utilisateur", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);
                             numeroFichier = MAX_ITEMS;
                         }

                         timerMoveActive = 0;
                         return FALSE;
                    }
                    case TIMERID_DELETE:
                    {
                         KillTimer(hwnd, TIMERID_DELETE);
                         isDeleting = 1;
                         int i;

                         HWND progressBarWnd = GetDlgItem(stateDlgWnd, IDPB_FILE);
                         SendMessage(progressBarWnd, PBM_SETPOS, 0, 0);

                         numeroFichier++;
                         if (numeroFichier >= MAX_ITEMS || tableauFichiersACopier[numeroFichier] == NULL)
                         {
                              for (i = 0 ; i < MAX_ITEMS && tableauDossiersACopier[i] != NULL ; i++)
                              {
                                  if (!RemoveDirectory(tableauDossiersACopier[i]))
                                     add_bilan_text("Suppression", "Impossible de supprimer le dossier", NULL, tableauDossiersACopier[i], NULL, ABT_GETLASTERROR);
                              }

                              enable_buttons(TRUE);
                              free_table();

                              isDeleting = 0;
                              return FALSE;
                         }

                         SHFILEOPSTRUCT shfo;
                         shfo.hwnd = NULL;
                         shfo.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
                         shfo.wFunc = FO_DELETE;
                         shfo.pFrom = tableauFichiersACopier[numeroFichier];

                         sprintf(buffer, "Fichier : %s", strrchr(tableauFichiersACopier[numeroFichier], '\\') + 1);
                         SetDlgItemText(stateDlgWnd, IDT_FILENAME, buffer);
                         SetDlgItemText(stateDlgWnd, IDT_SIZE, "[--- / ---] ko");

                         double moyenneTempsSuppression = 0;
                         if (numeroFichier > 0)
                            moyenneTempsSuppression = tempsTotalSuppression / 1000.0 / numeroFichier;
                         sprintf(buffer, "Temps restant estimé : %.0lf sec", moyenneTempsSuppression);
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFT, buffer);

                         int time = GetTickCount(),
                             errorCode = SHFileOperation(&shfo);

                         if (errorCode)
                         {
                            retrieve_error_text_for_SHFileOperation(errorCode, buffer);
                            add_bilan_text("Envoi à la corbeille", "Impossible de supprimer le fichier", buffer, tableauFichiersACopier[numeroFichier], NULL, 0);
                         }
                         else delete_item(tableauFichiersACopier[numeroFichier]);

                         tempsTotalSuppression += GetTickCount() - time;

                         SendMessage(progressBarWnd, PBM_SETPOS, 100, 0);

                         int percent = (numeroFichier + 1) * 100 / nombreFichiersACopier;
                         sprintf(buffer, "Envoi à la corbeille du fichier %d / %d", numeroFichier + 1, nombreFichiersACopier);
                         SetDlgItemText(stateDlgWnd, IDT_FILENUMBER, buffer);

                         draw_progress_bar(percent, 2);

                         SetDlgItemText(stateDlgWnd, IDT_TOTALSIZE, "[--- / --- ko]");
                         for (i = 0 ; i < MAX_ITEMS && tableauFichiersACopier[i] != NULL ; i++);
                         sprintf(buffer, "Temps restant estimé : %.0lf sec", moyenneTempsSuppression * (i - numeroFichier - 1));
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFTCOPY, buffer);

                         sprintf(buffer, "Temps écoulé : %.3lf sec", (GetTickCount() - heureDebutCopie) / 1000.0);
                         SetDlgItemText(stateDlgWnd, IDT_TIMECOPY, buffer);

                         if (stopNow)
                         {
                             add_bilan_text("Envoi à la corbeille", "Opération annulée par l'utilisateur", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);
                             numeroFichier = MAX_ITEMS;
                         }

                         SetTimer(hwnd, TIMERID_DELETE, tempsTimer * 1000, NULL);
                         return FALSE;
                    }
                    case TIMERID_ENCRYPT:
                    {
                         if (timerEncryptActive || isCopying)
                            return FALSE;
                         timerEncryptActive = 1;

                         if (!isEncrypting)
                         {
                             isEncrypting = 1;
                             isCopying = 1;
                             numeroFichier = 0;
                             SetTimer(mainWnd, TIMERID_COPY, tempsTimer * 1000, NULL);
                             timerEncryptActive = 0;
                             return FALSE;
                         }

                         HWND progressBarWnd = GetDlgItem(optnDlgWnd, IDPB_FILE);
                         SendMessage(progressBarWnd, PBM_SETPOS, 0, 0);

                         if (!isDeleting)
                         {
                            numeroFichier = 1;
                            if (!deleteOldFiles)
                               numeroFichier = MAX_ITEMS;
                            else isDeleting = 1;
                         }

                         if (numeroFichier >= MAX_ITEMS || tableauFichiersACopier[numeroFichier] == NULL)
                         {
                             KillTimer(hwnd, TIMERID_ENCRYPT);
                             numeroFichier = 0;
                             int i;

                             if (!createNewFiles)
                             {
                                 sprintf(tableauFichiersACopier[0], ".%s", encryptionExtension);
                                 for (i = 0 ; i < MAX_ITEMS - 1 && tableauFichiersACopier[i] != NULL ; i++);
                                 if (tableauFichiersACopier[i] == NULL)
                                    tableauFichiersACopier[i] = malloc(MAX_CHAINE);
                                 for (i = i ; i > 1 ; i--)
                                 {
                                     strcpy(tableauFichiersACopier[i], tableauFichiersACopier[i - 1]);
                                     sprintf(tableauFichiersACopier[i], "%s.%s", tableauFichiersACopier[i], encryptionExtension);
                                 }
                                 tableauFichiersACopier[1][0] = '\0';
                                 renameExtensionsOnly = 1;
                                 numeroFichier = 2;
                                 SetTimer(hwnd, TIMERID_RENAME, tempsTimer * 1000, NULL);
                             }
                             else
                             {
                                 free_table();
                                 enable_buttons(TRUE);
                             }

                             timerEncryptActive = 0;
                             isEncrypting = 0;
                             isDeleting = 0;
                             isDecrypting = 0;
                             return FALSE;
                         }

                         if (!DeleteFile(tableauFichiersACopier[numeroFichier]))
                         {
                            if (!isDecrypting)
                               add_bilan_text("Cryptage - suppression", "Impossible de supprimer le fichier", NULL, tableauFichiersACopier[numeroFichier], NULL, ABT_GETLASTERROR);
                            else add_bilan_text("Déryptage - suppression", "Impossible de supprimer le fichier", NULL, tableauFichiersACopier[numeroFichier], NULL, ABT_GETLASTERROR);
                         }
                         else delete_item(tableauFichiersACopier[numeroFichier]);
                         SendMessage(progressBarWnd, PBM_SETPOS, 100, 0);

                         sprintf(buffer, "Fichier : %s", strrchr(tableauFichiersACopier[numeroFichier], '\\') + 1);
                         SetDlgItemText(stateDlgWnd, IDT_FILENAME, buffer);
                         SetDlgItemText(stateDlgWnd, IDT_SIZE, "[--- / --- ko]");
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFT, "Temps restant estimé : inconnu");

                         sprintf(buffer, "Suppression du fichier %d / %d", numeroFichier, nombreFichiersACopier);
                         SetDlgItemText(stateDlgWnd, IDT_FILENUMBER, buffer);
                         SetDlgItemText(stateDlgWnd, IDT_TOTALSIZE, "[--- / --- ko]");
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFTCOPY, "Temps restant total estimé : inconnu");

                         int percent = numeroFichier * 100 / nombreFichiersACopier;
                         draw_progress_bar(percent, 2);

                         if (stopNow)
                         {
                             if (isDecrypting)
                                add_bilan_text("Décryptage", "Opération annulée par l'utilisateur", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);
                             else add_bilan_text("Cryptage", "Opération annulée par l'utilisateur", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);
                             numeroFichier = MAX_ITEMS;
                         }

                         numeroFichier++;
                         timerEncryptActive = 0;
                         return FALSE;
                    }
                    case TIMERID_RENAME:
                    {
                         if (timerRenameActive)
                            return FALSE;
                         timerRenameActive = 1;

                         if (numeroFichier >= MAX_ITEMS || tableauFichiersACopier[numeroFichier] == NULL)
                         {
                              free_table();
                              KillTimer(hwnd, TIMERID_RENAME);
                              enable_buttons(TRUE);
                              numeroFichier = 0;
                              timerRenameActive = 0;
                              return FALSE;
                         }

                         HWND progressBarWnd = GetDlgItem(optnDlgWnd, IDPB_FILE);
                         SendMessage(progressBarWnd, PBM_SETPOS, 0, 0);

                         strcpy(buffer, tableauFichiersACopier[numeroFichier]);
                         char *position = NULL;

                         if (renameExtensionsOnly)
                         {
                            position = strchr(buffer, '.');
                            if (position != NULL)
                               strcpy(buffer, position);
                            else buffer[0] = '\0';
                         }

                         if ((position = strstr(buffer, tableauFichiersACopier[0])) != NULL)
                         {
                             char part1[MAX_CHAINE], part2[MAX_CHAINE];
                             strcpy(part2, position + strlen(tableauFichiersACopier[0]));
                             *position = '\0';
                             strcpy(part1, buffer);
                             sprintf(buffer, "%s%s%s", part1, tableauFichiersACopier[1], part2);
                         }

                         if (renameExtensionsOnly && buffer[0] != '\0')
                         {
                             char buffer2[MAX_CHAINE];
                             strcpy(buffer2, buffer);
                             strcpy(buffer, tableauFichiersACopier[numeroFichier]);
                             strcpy(strchr(buffer, '.'), buffer2);
                         }

                         char errorType[MAX_CHAINE];
                         if (isDecrypting)
                            strcpy(errorType, "Décryptage - renommage");
                         else if (isEncrypting)
                              strcpy(errorType, "Cryptage - renommage");
                         else strcpy(errorType, "Renommage");

                         if (buffer[0] != '\0' && strcmp(buffer, tableauFichiersACopier[numeroFichier]) != 0)
                         {
                            if (is_file_enabled_to_copy(tableauFichiersACopier[numeroFichier], buffer))
                            {
                                if (rename(tableauFichiersACopier[numeroFichier], buffer))
                                   add_bilan_text(errorType, "Impossible de renommer le fichier", NULL, tableauFichiersACopier[numeroFichier], NULL, ABT_GETERRNO);
                                else
                                {
                                    delete_item(tableauFichiersACopier[numeroFichier]);
                                    ajouter_fichier(buffer, 0, NULL);
                                }
                            }
                         }
                         else if (buffer[0] == '\0')
                              add_bilan_text(errorType, "Extension introuvable", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);
                         else add_bilan_text(errorType, "Noms source et cible identiques", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);

                         SendMessage(progressBarWnd, PBM_SETPOS, 100, 0);

                         sprintf(buffer, "Fichier : %s", strrchr(tableauFichiersACopier[numeroFichier], '\\') + 1);
                         SetDlgItemText(stateDlgWnd, IDT_FILENAME, buffer);
                         SetDlgItemText(stateDlgWnd, IDT_SIZE, "[--- / --- ko]");
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFT, "Temps restant estimé : inconnu");

                         sprintf(buffer, "Renommage du fichier %d / %d", numeroFichier - 1, nombreFichiersACopier);
                         SetDlgItemText(stateDlgWnd, IDT_FILENUMBER, buffer);
                         SetDlgItemText(stateDlgWnd, IDT_TOTALSIZE, "[--- / --- ko]");
                         SetDlgItemText(stateDlgWnd, IDT_TIMELEFTCOPY, "Temps restant total estimé : inconnu");

                         int percent = (numeroFichier - 1) * 100 / nombreFichiersACopier;
                         draw_progress_bar(percent, 2);

                         if (stopNow)
                         {
                             add_bilan_text(errorType, "Opération annulée par l'utilisateur", "\0", tableauFichiersACopier[numeroFichier], NULL, 0);
                             numeroFichier = MAX_ITEMS;
                         }

                         numeroFichier++;
                         timerRenameActive = 0;
                         return FALSE;
                    }
                    case TIMERID_CONTROL:
                    {
                         if (isCopying || isDecrypting || isDeleting || isEncrypting || isMoving)
                            return FALSE;

                         if (GetForegroundWindow() == hwnd)
                         {
                             char name[MAX_CHAINE];
                             HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);
                             int number = ListView_GetItemCount(listWnd), i;

                             int itemAVerifier = dernierItemVerifie + 20;
                             if (itemAVerifier > number)
                                itemAVerifier = number;

                             for (i = dernierItemVerifie ; i < itemAVerifier ; i++)
                             {
                                 ListView_GetItemText(listWnd, i, IDCOL_DIR, name, MAX_CHAINE);
                                 ListView_GetItemText(listWnd, i, IDCOL_FILENAME, buffer, MAX_CHAINE);
                                 sprintf(name, "%s\\%s", name, buffer);
                                 if (!test_exist(name))
                                    ListView_DeleteItem(listWnd, i);
                                 else ajouter_fichier(name, 1, NULL);
                             }

                             if (itemAVerifier >= number)
                                dernierItemVerifie = 0;
                             else dernierItemVerifie = itemAVerifier;

                         }

                         return FALSE;
                    }
                    case TIMERID_PROTECTION:
                         KillTimer(hwnd, TIMERID_PROTECTION);
                         protection = 1;

                         if (!PL_verify_protection())
                         {
                            if (!PL_launch_protection(1))
                               protection = 0;
                         }

                         if (protection)
                            SetTimer(hwnd, TIMERID_PROTECTION, 500, NULL);
                         return FALSE;
                    case TIMERID_SHORTCUTS:
                    {
                         if (keyPressed)
                         {
                             if (!HIBYTE(GetKeyState(VK_CONTROL)) || !HIBYTE(GetKeyState(VK_SHIFT)) || !HIBYTE(GetKeyState('C')))
                                 keyPressed = 0;
                             return FALSE;
                         }
                         else if (!HIBYTE(GetKeyState(VK_CONTROL)) || !HIBYTE(GetKeyState(VK_SHIFT)) || !HIBYTE(GetKeyState('C')))
                              return FALSE;

                         keyPressed = 1;


                         HWND explorerWnd = GetForegroundWindow();
                         ShowWindow(mainWnd, SW_RESTORE);
                         SetForegroundWindow(mainWnd);

                         if (explorerWnd == NULL)
                            return FALSE;

                         char directory[MAX_CHAINE] = "\0", fileName[MAX_CHAINE] = "\0";
                         GetWindowText(explorerWnd, directory, MAX_CHAINE);

                         if (strcmp(directory, "Mes documents") == 0)
                            SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, directory);
                         if (test_exist(directory) != 2)
                            return FALSE;

                         HWND listWnd = GetDlgItem(explorerWnd, 0);
                         if (listWnd != NULL)
                            listWnd = GetDlgItem(listWnd, 0);
                         if (listWnd != NULL)
                            listWnd = GetDlgItem(listWnd, 0);
                         if (listWnd != NULL)
                            listWnd = GetDlgItem(listWnd, 0);
                         if (listWnd != NULL)
                            listWnd = GetDlgItem(listWnd, 1);
                         if (listWnd == NULL)
                            return FALSE;

                         int selected = ListView_GetSelectedCount(listWnd);
                         if (selected <= 0)
                            return FALSE;
                         KillTimer(hwnd, TIMERID_SHORTCUTS);


                         int i, numberItems = ListView_GetItemCount(listWnd), j = 0;
                         int *tab = malloc(selected * sizeof(int));

                         for (i = 0 ; i < numberItems ; i++)
                         {
                             if (is_item_selected(listWnd, i))
                             {
                                tab[j] = i;
                                j++;
                             }
                         }

                         DWORD processID = 0;
                         GetWindowThreadProcessId(explorerWnd, &processID);
                         HANDLE process = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, processID);
                         if (process == NULL)
                         {
                            SetTimer(hwnd, TIMERID_SHORTCUTS, 50, NULL);
                            return FALSE;
                         }

                         char *_string = (char*) VirtualAllocEx(process, NULL, MAX_CHAINE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                         LVITEM *_lvi = (LVITEM*) VirtualAllocEx(process, NULL, sizeof(LVITEM), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

                         if (_string == NULL || _lvi == NULL)
                         {
                            if (_string != NULL)
                               VirtualFree(_string, 0, MEM_RELEASE);
                            if (_lvi != NULL)
                               VirtualFree(_lvi, 0, MEM_RELEASE);

                            CloseHandle(process);
                            SetTimer(hwnd, TIMERID_SHORTCUTS, 50, NULL);
                            return FALSE;
                         }

                         LVITEM lvi;
                         lvi.mask = LVIF_TEXT;
                         lvi.pszText = _string;
                         lvi.iSubItem = 0;
                         lvi.cchTextMax = MAX_CHAINE;

                         for (i = 0 ; i < selected ; i++)
                         {
                             lvi.iItem = tab[i];
                             WriteProcessMemory(process, _lvi, &lvi, sizeof(LVITEM), NULL);
                             SendMessage(listWnd, LVM_GETITEMTEXT, tab[i], (LPARAM)_lvi);

                             ReadProcessMemory(process, _string, fileName, MAX_CHAINE, NULL);
                             sprintf(buffer, "%s\\%s", directory, fileName);

                             if (!test_exist(buffer))
                             {
                                WIN32_FIND_DATA wfd;
                                strcat(buffer, "*");
                                HANDLE hSearch = FindFirstFile(buffer, &wfd);
                                sprintf(buffer, "%s\\%s", directory, wfd.cFileName);
                                CloseHandle(hSearch);
                             }

                             ajouter_fichier(buffer, 0, NULL);
                         }

                         free(tab);

                         VirtualFree(_string, 0, MEM_RELEASE);
                         VirtualFree(_lvi, 0, MEM_RELEASE);
                         CloseHandle(process);

                         SetTimer(hwnd, TIMERID_SHORTCUTS, 50, NULL);
                         return FALSE;
                    }
             }

        default:
            return DefWindowProc (hwnd, msg, wParam, lParam);
    }

    return 0;
}



LRESULT CALLBACK OptnDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select, i, notification;
    OPENFILENAME ofn;
    char buffer[MAX_CHAINE] = "\0",
         title[MAX_CHAINE] = "\0";

    memset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.nMaxFile = MAX_CHAINE;

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                FILE *file = NULL;
                if ((file = fopen(configFileName, "r")) != NULL)
                {
                    char *position = NULL;
                    int state;

                    fgets(buffer, MAX_CHAINE, file);
                    if ((position = strchr(buffer, '\n')) != NULL)
                       *position = '\0';
                    SetDlgItemText(hwnd, IDE_FILTER, buffer);

                    fgets(buffer, MAX_CHAINE, file);
                    if ((position = strchr(buffer, '\n')) != NULL)
                       *position = '\0';
                    SetDlgItemText(hwnd, IDE_SEARCHDIR, buffer);

                    fgets(buffer, MAX_CHAINE, file);
                    if ((position = strchr(buffer, '\n')) != NULL)
                       *position = '\0';
                    SetDlgItemText(hwnd, IDE_DIROUT, buffer);

                    fgets(buffer, MAX_CHAINE, file);
                    state = strtol(buffer, NULL, 10);
                    CheckDlgButton(hwnd, IDC_NOCHANGEDIR, state);

                    fgets(buffer, MAX_CHAINE, file);
                    state = strtol(buffer, NULL, 10);
                    CheckDlgButton(hwnd, IDC_SIZEMAX, state);

                    fgets(buffer, MAX_CHAINE, file);
                    if ((position = strchr(buffer, '\n')) != NULL)
                       *position = '\0';
                    SetDlgItemText(hwnd, IDE_SIZEMAX, buffer);

                    fgets(buffer, MAX_CHAINE, file);
                    state = strtol(buffer, NULL, 10);
                    CheckDlgButton(hwnd, IDC_TIMEMAX, state);

                    fgets(buffer, MAX_CHAINE, file);
                    if ((position = strchr(buffer, '\n')) != NULL)
                       *position = '\0';
                    SetDlgItemText(hwnd, IDE_TIMEMAX, buffer);

                    fgets(buffer, MAX_CHAINE, file);
                    state = strtol(buffer, NULL, 10);
                    CheckDlgButton(hwnd, IDC_READONLY, state);

                    fgets(buffer, MAX_CHAINE, file);
                    state = strtol(buffer, NULL, 10);
                    CheckDlgButton(hwnd, IDC_HIDDEN, state);

                    fgets(buffer, MAX_CHAINE, file);
                    state = strtol(buffer, NULL, 10);
                    CheckDlgButton(hwnd, IDC_SYSTEM, state);

                    fgets(buffer, MAX_CHAINE, file);
                    state = strtol(buffer, NULL, 10);
                    CheckDlgButton(hwnd, IDC_ARCHIVE, state);

                    fgets(buffer, MAX_CHAINE, file);
                    state = strtol(buffer, NULL, 10);
                    CheckDlgButton(hwnd, IDC_REPLACE, state);

                    fgets(protectionPassword, MAX_CHAINE, file);
                    if (strchr(protectionPassword, '\n') != NULL)
                       *strchr(protectionPassword, '\n') = '\0';

                    fgets(buffer, MAX_CHAINE, file);
                    tailleACopier = strtol(buffer, NULL, 10);

                    fgets(buffer, MAX_CHAINE, file);
                    tempsTimer = strtod(buffer, NULL);
                    vitesseCopie = tailleACopier * pow(2,10) / tempsTimer;

                    fgets(buffer, MAX_CHAINE, file);
                    cassSensitiveResearch = strtod(buffer, NULL);

                    fgets(buffer, MAX_CHAINE, file);
                    wildcardsResearch = strtod(buffer, NULL);

                    fgets(buffer, MAX_CHAINE, file);
                    recursiveResearch = strtod(buffer, NULL);

                    fgets(buffer, MAX_CHAINE, file);
                    dirsResearch = strtod(buffer, NULL);

                    fgets(buffer, MAX_CHAINE, file);
                    filesResearch = strtod(buffer, NULL);

                    fclose(file);
                }

                if (IsDlgButtonChecked(hwnd, IDC_NOCHANGEDIR))
                   EnableWindow(GetDlgItem(hwnd, IDE_DIROUT), FALSE);
                if (!IsDlgButtonChecked(hwnd, IDC_SIZEMAX))
                   EnableWindow(GetDlgItem(hwnd, IDE_SIZEMAX), FALSE);
                if (!IsDlgButtonChecked(hwnd, IDC_TIMEMAX))
                   EnableWindow(GetDlgItem(hwnd, IDE_TIMEMAX), FALSE);

                return FALSE;
           }
           case WM_NOTIFY:
           {
                NMHDR *nmhdr = (NMHDR*)lParam;

                switch (nmhdr->code)
                {
                       case TTN_SHOW:
                            return FALSE;
                       case TTN_POP:
                            create_toolTips();
                            return FALSE;
                }

                return FALSE;
           }
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_FILES:
                      {
                           ofn.lpstrFilter = "Tous les fichiers  (*.*)\0*.*\0\0";
                           ofn.lpstrFile = buffer;
                           ofn.lpstrTitle = "Ouvrir";
                           ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
                           ofn.lpstrInitialDir = NULL;

                           if (!GetOpenFileName(&ofn))
                               return FALSE;

                           char dir[MAX_CHAINE];
                           strcpy(dir, buffer);
                           char *position = buffer + ofn.nFileOffset;
                           *(dir + ofn.nFileOffset) = '\0';

                           if (dir[strlen(dir) - 1] == '\\')
                              dir[strlen(dir) - 1] = '\0';
                           if (*position == '\\')
                              position += 1;

                           char nom[MAX_CHAINE];
                           for (i = 0 ; i < MAX_CHAINE && position != NULL && *position != '\0' ; i++)
                           {
                                    sprintf(nom, "%s\\%s", dir, position);
                                    ajouter_fichier(nom, 0, NULL);

                                    position = strchr(position, '\0');
                                    if (position != NULL)
                                       position += 1;
                           }

                           return FALSE;
                      }
                      case IDE_SEARCHDIR:
                           strcpy(title, "Choisissez un répertoire de recherche.");
                      case IDE_DIROUT:
                      {
                           notification = HIWORD(wParam);
                           if (notification != EN_SETFOCUS)
                              return FALSE;

                           if (Select == IDE_DIROUT)
                              strcpy(title, "Choisissez un répertoire de sortie pour les fichiers traités.");

                           GetDlgItemText(hwnd, Select, buffer, MAX_CHAINE);

                           BROWSEINFO bi;
                           bi.hwndOwner = hwnd;
                           bi.pidlRoot = NULL;
                           bi.pszDisplayName = buffer;
                           bi.lpszTitle = title;
                           bi.ulFlags = BIF_NEWDIALOGSTYLE;
                           bi.lpfn = NULL;
                           bi.lParam = 0;
                           bi. iImage = 0;
                           ITEMIDLIST *idPath = SHBrowseForFolder(&bi);

                           if (idPath != NULL)
                           {
                               SHGetPathFromIDList(idPath, buffer);
                               SetDlgItemText(hwnd, Select, buffer);
                               CoTaskMemFree(idPath);

                               create_toolTips();
                           }

                           return FALSE;
                      }
                      case IDC_NOCHANGEDIR:
                           if (IsDlgButtonChecked(hwnd, Select))
                              EnableWindow(GetDlgItem(hwnd, IDE_DIROUT), FALSE);
                           else EnableWindow(GetDlgItem(hwnd, IDE_DIROUT), TRUE);

                           return FALSE;
                      case IDC_SIZEMAX:
                           if (IsDlgButtonChecked(hwnd, Select))
                              EnableWindow(GetDlgItem(hwnd, IDE_SIZEMAX), TRUE);
                           else EnableWindow(GetDlgItem(hwnd, IDE_SIZEMAX), FALSE);

                           return FALSE;
                      case IDC_TIMEMAX:
                           if (IsDlgButtonChecked(hwnd, Select))
                              EnableWindow(GetDlgItem(hwnd, IDE_TIMEMAX), TRUE);
                           else EnableWindow(GetDlgItem(hwnd, IDE_TIMEMAX), FALSE);

                           return FALSE;
                      case IDE_FILTER:
                           notification = HIWORD(wParam);
                           if (notification == EN_SETFOCUS)
                              SendMessage(hwnd, DM_SETDEFID, IDP_SEARCH, 0);
                           else if (notification == EN_KILLFOCUS)
                                SendMessage(hwnd, DM_SETDEFID, IDP_COPY, 0);

                           return FALSE;
                      case IDP_SEARCH:
                      {
                           char filter[MAX_CHAINE],
                                directory[MAX_CHAINE];
                           GetDlgItemText(hwnd, IDE_FILTER, filter, MAX_CHAINE);
                           GetDlgItemText(hwnd, IDE_SEARCHDIR, directory, MAX_CHAINE);

                           if (!DialogBox(mainInstance, "BeginSearch", mainWnd, (DLGPROC)BeginSearchProc))
                              return FALSE;

                           WindowInfo wi;
                           wi.wndFileName = CreateDialog(mainInstance, "IsSearchingDlg", NULL, (DLGPROC)IsSearchingDlgProc);
                           wi.wndNumberFilesFound = GetDlgItem(wi.wndFileName, IDT_NUMBERFILESFOUND);
                           wi.wndNumberFilesAdded = GetDlgItem(wi.wndFileName, IDT_NUMBERFILESADDED);
                           wi.wndCancelButton = GetDlgItem(wi.wndFileName, IDP_CANCEL);
                           wi.wndFileName = GetDlgItem(wi.wndFileName, IDT_FILENAME);

                           ShowWindow(mainWnd, SW_HIDE);

                           int result = lister_tout(directory, filter, listFileName, &wi, (filesResearch * LT_FILES) | (dirsResearch * LT_DIRS) | (recursiveResearch * LT_SUBDIRS) | (cassSensitiveResearch * LT_CASS) | (wildcardsResearch * LT_WILDCARDS) | LT_CLEAR);

                           DestroyWindow(GetParent(wi.wndFileName));
                           if (result > 0)
                              ajouter_liste(listFileName, NULL);

                           ShowWindow(mainWnd, SW_SHOW);
                           return FALSE;
                      }
                      case IDP_MOVE:
                      case IDP_COPY:
                      {
                           CheckDlgButton(hwnd, IDC_NOCHANGEDIR, BST_UNCHECKED);
                           EnableWindow(GetDlgItem(hwnd, IDE_DIROUT), TRUE);

                           int j;
                           if (Select == IDP_COPY)
                              j = ready_to_work(TW_COPY);
                           else j = ready_to_work(TW_MOVE);
                           j++;

                           if (tableauFichiersACopier[0][0] == '\0')
                           {
                                MessageBox(hwnd, "Le dossier de destination n'est pas spécifié.", "Attention", MB_ICONWARNING | MB_OK);
                                return FALSE;
                           }

                           char destination[MAX_CHAINE];
                           strcpy(destination, tableauFichiersACopier[0]);

                           HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);
                           char name[MAX_CHAINE],
                                directory[MAX_CHAINE];

                           if (Select == IDP_MOVE)
                           {
                               nombreFichiersACopier = ListView_GetItemCount(listWnd);
                               for (i = 0 ; i < nombreFichiersACopier ; i++)
                               {
                                   if (!is_item_directory(i) && (!onlySelected || is_item_selected(listWnd, i)))
                                   {
                                       ListView_GetItemText(listWnd, i, IDCOL_FILENAME, name, MAX_CHAINE);
                                       ListView_GetItemText(listWnd, i, IDCOL_DIR, directory, MAX_CHAINE);
                                       sprintf(buffer, "%s\\%s", directory, name);
                                       strcpy(directory, buffer);

                                       correct_adress(buffer, tableauFichiersACopier[0]);

                                       if (j < MAX_ITEMS && destination[0] == directory[0] && is_file_enabled_to_copy(directory, buffer))
                                       {
                                             tableauFichiersACopier[j] = malloc(MAX_CHAINE);
                                             strcpy(tableauFichiersACopier[j], directory);
                                             j++;
                                       }
                                   }
                               }

                               for (i = 1 ; i < MAX_ITEMS && tableauFichiersACopier[i] != NULL ; i++);
                               nombreFichiersACopier = i;
                           }


                           for (i = 0 ; i < MAX_ITEMS && tableauDossiersACopier[i] != NULL ; i++)
                           {
                               strcpy(buffer, tableauDossiersACopier[i]);
                               correct_adress(buffer, tableauFichiersACopier[0]);
                               if (!CreateDirectory(buffer, NULL))
                                  add_bilan_text("Début de copie", "Impossible de créer le dossier", NULL, buffer, NULL, ABT_GETLASTERROR);
                           }

                           onlySelected = 0;

                           if (Select == IDP_COPY)
                              SetTimer(mainWnd, TIMERID_COPY, tempsTimer * 1000, NULL);
                           else
                           {
                               etapeDeplacement = 0;
                               isCopying = 0;
                               SetTimer(mainWnd, TIMERID_MOVE, tempsTimer * 1000, NULL);
                           }
                           return FALSE;
                      }
                      case IDP_DELETE:
                      {
                           HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);
                           if (onlySelected)
                              nombreFichiersACopier = ListView_GetSelectedCount(listWnd);
                           else nombreFichiersACopier = ListView_GetItemCount(listWnd);

                           if (nombreFichiersACopier > 1)
                              sprintf(buffer, "Voulez-vous vraiment envoyer à la corbeille ces %d éléments ?", nombreFichiersACopier);
                           else if (nombreFichiersACopier == 1)
                                strcpy(buffer, "Voulez-vous vraiment envoyer à la corbeille cet élément ?");
                           else return FALSE;

                           if (MessageBox(mainWnd, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) != IDYES)
                              return FALSE;


                           ready_to_work(TW_DELETE);

                           numeroFichier = -1;
                           tempsTotalSuppression = 0;
                           onlySelected = 0;

                           SetTimer(mainWnd, TIMERID_DELETE, tempsTimer * 1000, NULL);
                           return FALSE;
                      }
                      case IDP_ENCRYPT:
                           if (!DialogBoxParam(mainInstance, "BeginEncryption", mainWnd, (DLGPROC)EncryptionProc, 1))
                              enable_buttons(TRUE);
                           return FALSE;
                      case IDP_RENAME:
                           if (!DialogBox(mainInstance, "BeginRename", mainWnd, (DLGPROC)RenameProc))
                              enable_buttons(TRUE);
                           return FALSE;
                      case IDP_DECRYPT:
                           if (!DialogBoxParam(mainInstance, "BeginEncryption", mainWnd, (DLGPROC)EncryptionProc, 2))
                              enable_buttons(TRUE);
                           return FALSE;
                      case IDP_PROTECT:
                           if (protectionPassword[0] == '\0' || DialogBoxParam(mainInstance, "Password", mainWnd, (DLGPROC)PasswordProc, (LPARAM)protectionPassword))
                           {
                              if (DialogBox(mainInstance, "BeginProtection", mainWnd, (DLGPROC)ProtectionProc))
                              {
                                  if (!test_vide(PL_listFileName))
                                     PL_stop_protection();
                                  else if (!protection)
                                     SetTimer(mainWnd, TIMERID_PROTECTION, 500, NULL);
                              }
                           }
                           else MessageBox(mainWnd, "Mot de passe incorrect !", "Erreur", MB_OK | MB_ICONWARNING);
                           enable_buttons(TRUE);
                           return FALSE;
                      case IDP_DISPROTECT:
                           DialogBox(mainInstance, "BeginDisprotection", mainWnd, (DLGPROC)DisprotectionProc);
                           return FALSE;
                      case IDP_BILAN:
                           ShowWindow(bilanWnd, SW_SHOW);
                           return FALSE;
                      case IDP_STOP:
                           stopNow = 1;
                           return FALSE;
                }

                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK StateDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select, notification, number;
    double decimal;
    char buffer[MAX_CHAINE];

    switch (msg)
    {
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDE_PARTSIZE:
                           notification = HIWORD(wParam);
                           switch (notification)
                           {
                                  case EN_CHANGE:
                                       GetDlgItemText(hwnd, IDE_PARTSIZE, buffer, MAX_CHAINE);
                                       number = strtol(buffer, NULL, 10);
                                       if (number > 1)
                                          SetDlgItemText(hwnd, IDT_OCTET, "kos");
                                       else SetDlgItemText(hwnd, IDT_OCTET, "ko");

                                       return FALSE;
                                  case EN_KILLFOCUS:
                                       GetDlgItemText(hwnd, IDE_PARTSIZE, buffer, MAX_CHAINE);
                                       number = strtol(buffer, NULL, 10);
                                       if (number > 0)
                                          tailleACopier = number;
                                       sprintf(buffer, "%d", tailleACopier);
                                       SetDlgItemText(hwnd, IDE_PARTSIZE, buffer);

                                       vitesseCopie = tailleACopier * pow(2,10) / tempsTimer;
                                       sprintf(buffer, "Vitesse voulue : %.3lf Mo/s", vitesseCopie / (1.0 *pow(2,20)));
                                       SetDlgItemText(hwnd, IDT_SPEED, buffer);

                                       return FALSE;
                           }

                           return FALSE;
                      case IDE_TIMER:
                           notification = HIWORD(wParam);
                           if (notification != EN_KILLFOCUS)
                              return FALSE;

                           GetDlgItemText(hwnd, IDE_TIMER, buffer, MAX_CHAINE);
                           decimal = strtod(buffer, NULL);
                           if (decimal > 0)
                              tempsTimer = decimal;
                           sprintf(buffer, "%.3lf", tempsTimer);
                           SetDlgItemText(hwnd, IDE_TIMER, buffer);

                           vitesseCopie = tailleACopier * pow(2,10) / tempsTimer;
                           sprintf(buffer, "Vitesse voulue : %.3lf Mo/s", vitesseCopie / (1.0 * pow(2,20)));
                           SetDlgItemText(hwnd, IDT_SPEED, buffer);

                           return FALSE;
                }
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK IsSearchingDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
           case WM_INITDIALOG:
                centrer_fenetre(hwnd, NULL);
                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK EncryptionProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE];

    switch (msg)
    {
           case WM_INITDIALOG:
                if (lParam == 2)
                {
                    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconDecrypt);
                    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconDecrypt);
                }
                else
                {
                    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconEncrypt);
                    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconEncrypt);
                }

                CheckDlgButton(hwnd, IDR_REPLACEFILE, BST_CHECKED);
                EnableWindow(GetDlgItem(hwnd, IDC_DELETEOLDFILE), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDE_EXTENSION), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDT_EXTENSION), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_DELETEEXTENSION), FALSE);

                if (lParam == 2)
                {
                    ShowWindow(GetDlgItem(hwnd, IDT_EXTENSION), FALSE);
                    ShowWindow(GetDlgItem(hwnd, IDE_EXTENSION), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDE_PWD2), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDT_PWD2), FALSE);
                    SetWindowText(hwnd, "Décrypter fichier");
                    if (IsDlgButtonChecked(optnDlgWnd, IDC_NOCHANGEDIR))
                       CheckDlgButton(hwnd, IDC_DELETEEXTENSION, BST_CHECKED);
                }
                else ShowWindow(GetDlgItem(hwnd, IDC_DELETEEXTENSION), FALSE);
                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDR_REPLACEFILE:
                           if (IsDlgButtonChecked(hwnd, IDR_REPLACEFILE))
                           {
                              EnableWindow(GetDlgItem(hwnd, IDC_DELETEOLDFILE), FALSE);
                              EnableWindow(GetDlgItem(hwnd, IDE_EXTENSION), FALSE);
                              EnableWindow(GetDlgItem(hwnd, IDT_EXTENSION), FALSE);
                              EnableWindow(GetDlgItem(hwnd, IDC_DELETEEXTENSION), FALSE);
                           }
                           else
                           {
                               EnableWindow(GetDlgItem(hwnd, IDC_DELETEOLDFILE), TRUE);
                               EnableWindow(GetDlgItem(hwnd, IDE_EXTENSION), TRUE);
                               EnableWindow(GetDlgItem(hwnd, IDT_EXTENSION), TRUE);
                               if (!IsDlgButtonChecked(optnDlgWnd, IDC_NOCHANGEDIR))
                                  EnableWindow(GetDlgItem(hwnd, IDC_DELETEEXTENSION), TRUE);
                           }

                           return FALSE;
                      case IDR_CREATENEWFILE:
                           if (!IsDlgButtonChecked(hwnd, IDR_CREATENEWFILE))
                           {
                              EnableWindow(GetDlgItem(hwnd, IDC_DELETEOLDFILE), FALSE);
                              EnableWindow(GetDlgItem(hwnd, IDE_EXTENSION), FALSE);
                              EnableWindow(GetDlgItem(hwnd, IDT_EXTENSION), FALSE);
                              EnableWindow(GetDlgItem(hwnd, IDC_DELETEEXTENSION), FALSE);
                           }
                           else
                           {
                               EnableWindow(GetDlgItem(hwnd, IDC_DELETEOLDFILE), TRUE);
                               EnableWindow(GetDlgItem(hwnd, IDE_EXTENSION), TRUE);
                               EnableWindow(GetDlgItem(hwnd, IDT_EXTENSION), TRUE);
                               if (!IsDlgButtonChecked(optnDlgWnd, IDC_NOCHANGEDIR))
                                  EnableWindow(GetDlgItem(hwnd, IDC_DELETEEXTENSION), TRUE);
                           }

                           return FALSE;
                      case IDP_CANCEL:
                           EndDialog(hwnd, 0);
                           return FALSE;
                      case IDP_OK:
                      {
                           isDecrypting = IsWindowVisible(GetDlgItem(hwnd, IDC_DELETEEXTENSION));

                           GetDlgItemText(hwnd, IDE_PWD, password, MAX_CHAINE);
                           GetDlgItemText(hwnd, IDE_PWD2, buffer, MAX_CHAINE);
                           if (!isDecrypting && strcmp(password, buffer) != 0)
                           {
                               MessageBox(mainWnd, "Les mots de passe sont différents.", "Attention", MB_OK | MB_ICONWARNING);
                               return FALSE;
                           }

                           GetDlgItemText(hwnd, IDE_EXTENSION, encryptionExtension, MAX_CHAINE);
                           if (encryptionExtension[0] == '.')
                              strcpy(encryptionExtension, encryptionExtension + 1);
                           if (encryptionExtension[0] == '\0')
                              strcpy(encryptionExtension, "crpt");

                           deleteOldFiles = 0;
                           createNewFiles = 0;
                           if (IsDlgButtonChecked(hwnd, IDR_CREATENEWFILE))
                           {
                              createNewFiles = 1;
                              if (IsDlgButtonChecked(hwnd, IDC_DELETEOLDFILE))
                                 deleteOldFiles = 1;
                              if (IsDlgButtonChecked(hwnd, IDC_DELETEEXTENSION))
                                 deleteOldExtension = 1;
                              if (isDecrypting)
                                 ready_to_work(TW_DECRYPT);
                              else ready_to_work(TW_ENCRYPT);
                              if (IsDlgButtonChecked(optnDlgWnd, IDC_NOCHANGEDIR))
                                 tableauFichiersACopier[0][0] = '\0';
                              if (encryptionExtension[0] == '\0')
                                 strcpy(encryptionExtension, "crpt");
                           }
                           else
                           {
                               CheckDlgButton(optnDlgWnd, IDC_NOCHANGEDIR, BST_CHECKED);
                               EnableWindow(GetDlgItem(hwnd, IDE_DIROUT), FALSE);

                               ready_to_work(TW_COPY);
                               deleteOldFiles = 1;
                               strcpy(encryptionExtension, "crpt$!");
                               if (isDecrypting)
                                  strcpy(encryptionExtension, "dcrpt$!");
                               tableauFichiersACopier[0][0] = '\0';
                           }

                           SetTimer(mainWnd, TIMERID_ENCRYPT, tempsTimer * 1000, NULL);
                           EndDialog(hwnd, 1);
                           return FALSE;
                      }
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK RenameProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;

    switch (msg)
    {
           case WM_INITDIALOG:
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconRename);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconRename);

                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_CANCEL:
                           EndDialog(hwnd, 0);
                           return FALSE;
                      case IDP_OK:
                           tableauFichiersACopier[0] = malloc(MAX_CHAINE);
                           GetDlgItemText(hwnd, IDE_TEXTTOREPLACE, tableauFichiersACopier[0], MAX_CHAINE);
                           tableauFichiersACopier[1] = malloc(MAX_CHAINE);
                           GetDlgItemText(hwnd, IDE_REPLACETEXT, tableauFichiersACopier[1], MAX_CHAINE);
                           if (IsDlgButtonChecked(hwnd, IDC_EXTONLY))
                              renameExtensionsOnly = 1;
                           else renameExtensionsOnly = 0;

                           ready_to_work(TW_RENAME);
                           numeroFichier = 2;
                           SetTimer(mainWnd, TIMERID_RENAME, tempsTimer * 1000, NULL);
                           EndDialog(hwnd, 1);
                           return FALSE;

                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK BilanDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select, notification;
    char buffer[MAX_CHAINE];

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconCopy);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconCopy);

                RECT rect;
                GetClientRect(hwnd, &rect);

                HWND listWnd = CreateWindowEx(
                      WS_EX_CLIENTEDGE,
                      WC_LISTVIEW,
                      "",
                      WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,
                      0,
                      0,
                      rect.right,
                      rect.bottom,
                      hwnd,
                      (HMENU) IDLV_LIST,
                      mainInstance,
                      NULL
                   );

                strcpy(buffer, "Section");
                LVCOLUMN lvcol;
                lvcol.mask = LVCF_WIDTH | LVCF_TEXT;
                lvcol.cx = 150;
                lvcol.pszText = buffer;
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_SECTION, &lvcol);

                lvcol.cx = 250;
                strcpy(buffer, "Erreur");
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_ERROR, &lvcol);

                lvcol.cx = 250;
                strcpy(buffer, "Raison");
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_WHY, &lvcol);

                lvcol.cx = 300;
                strcpy(buffer, "Fichier");
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_FILEERROR, &lvcol);

                lvcol.cx = 75;
                strcpy(buffer, "Heure");
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_HOUR, &lvcol);

                ListView_SetExtendedListViewStyle(listWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

                return FALSE;
           }
           case WM_NOTIFY:
           {
                 Select = LOWORD(wParam);
                 if (Select != IDLV_LIST)
                    return FALSE;

                 HWND listWnd = GetDlgItem(hwnd, IDLV_LIST);

                 NMITEMACTIVATE *nmia;
                 nmia = (NMITEMACTIVATE*) lParam;

                 notification = nmia->hdr.code;
                 switch (notification)
                 {
                     case NM_RCLICK:
                     {
                         HMENU menu = CreatePopupMenu();
                         if (ListView_GetSelectedCount(listWnd) > 0)
                         {
                             AppendMenu(menu, MF_STRING, ITEM_ADDTOLIST, "Ajouter le fichier à la liste de travail");
                             AppendMenu(menu, MF_SEPARATOR, 0, NULL);
                             AppendMenu(menu, MF_STRING, ITEM_DELETE, "Supprimer de la liste");
                         }
                         if (ListView_GetItemCount(listWnd) > 0)
                            AppendMenu(menu, MF_STRING, ITEM_CLEAR, "Vider la liste");

                         POINT pt;
                         GetCursorPos(&pt);
                         TrackPopupMenu(menu, 0, pt.x, pt.y, 0, hwnd, NULL);

                         return FALSE;
                     }
                     case LVN_COLUMNCLICK:
                     {
                         NMLISTVIEW *nmlv = (NMLISTVIEW*) lParam;
                         HWND listWnd = GetDlgItem(hwnd, IDLV_LIST);
                         nombreTotalItems = ListView_GetItemCount(listWnd);
                         if (nombreTotalItems > MAX_ITEMS)
                            nombreTotalItems = MAX_ITEMS;

                         int i;
                         for (i = 0 ; i < nombreTotalItems ; i++)
                         {
                             tableauTotalItems[i].lvi.mask = LVIF_PARAM | LVIF_IMAGE;
                             tableauTotalItems[i].lvi.iItem = i;
                             tableauTotalItems[i].lvi.iSubItem = nmlv->iSubItem;
                             ListView_GetItem(listWnd, &(tableauTotalItems[i].lvi));
                             ListView_GetItemText(listWnd, i, nmlv->iSubItem, tableauTotalItems[i].pszText, MAX_CHAINE);
                         }

                         ListView_SortItems(listWnd, (PFNLVCOMPARE)CompareFunction, nmlv->iSubItem + 5);
                         return FALSE;
                     }
                 }
                 return FALSE;
           }
           case WM_COMMAND:
                Select = LOWORD(wParam);
                switch (Select)
                {
                    case ITEM_CLEAR:
                         if (MessageBox(hwnd, "Suppression de tous les items de la liste.\nCette opération est irréversible.\nContinuer ?", "Attention", MB_ICONWARNING | MB_YESNO) == IDYES)
                            ListView_DeleteAllItems(GetDlgItem(hwnd, IDLV_LIST));
                         return FALSE;
                    case ITEM_DELETE:
                    {
                         HWND listWnd = GetDlgItem(hwnd, IDLV_LIST);

                         int *selectedItems = NULL;
                         int numberOfSelectedItems = get_selected_items(listWnd, &selectedItems);

                         if (numberOfSelectedItems > 1)
                            sprintf(buffer, "Voulez-vous vraiment supprimer de la liste ces %d éléments ?", numberOfSelectedItems);
                         else if (numberOfSelectedItems == 1)
                              strcpy(buffer, "Voulez-vous vraiment supprimer de la liste cet élément ?");
                         else return FALSE;

                         if (MessageBox(hwnd, buffer, "Confirmation", MB_ICONQUESTION | MB_YESNO) != IDYES)
                            return FALSE;

                         int i;
                         for (i = 0 ; i < numberOfSelectedItems ; i++)
                             ListView_DeleteItem(listWnd, selectedItems[i]);

                         free(selectedItems);
                         attrib_lParams(listWnd);
                         return FALSE;
                    }
                    case ITEM_ADDTOLIST:
                    {
                         HWND listWnd = GetDlgItem(hwnd, IDLV_LIST);

                         int *selectedItems = NULL;
                         int numberOfSelectedItems = get_selected_items(listWnd, &selectedItems);

                         if (numberOfSelectedItems <= 0)
                            return FALSE;

                         int i;
                         for (i = 0 ; i < numberOfSelectedItems ; i++)
                         {
                             ListView_GetItemText(listWnd, selectedItems[i], IDCOL_FILEERROR, buffer, MAX_CHAINE);
                             ajouter_fichier(buffer, 0, NULL);
                         }

                         free(selectedItems);
                         return FALSE;
                    }
                }
                return FALSE;
           case WM_CLOSE:
                ShowWindow(hwnd, SW_HIDE);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK ProtectionProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select, notification;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconShield);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconShield);

                canModifyPassword = 1;
                SetDlgItemText(hwnd, IDE_PWD, protectionPassword);
                SetDlgItemText(hwnd, IDE_PWD2, protectionPassword);
                canModifyPassword = 0;
                SetFocus(hwnd);
                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDE_PWD2:
                      case IDE_PWD:
                           notification = HIWORD(wParam);

                           if (canModifyPassword || notification != EN_CHANGE)
                              return FALSE;

                           if (MessageBox(hwnd, "Vous vous apprêtez à changer votre mot de passe de protection des fichiers.\nCe changement sera valable pour TOUS les fichiers protégés jusqu'à maintenant et dans l'avenir.\nConfirmez-vous le changement ?", "Attention", MB_ICONWARNING | MB_YESNO) == IDNO)
                           {
                               canModifyPassword = 1;
                               SetDlgItemText(hwnd, IDE_PWD, protectionPassword);
                               SetDlgItemText(hwnd, IDE_PWD2, protectionPassword);
                               canModifyPassword = 0;
                               return FALSE;
                           }

                           canModifyPassword = 1;
                           SetFocus(GetDlgItem(hwnd, IDE_PWD));
                           return FALSE;
                      case IDP_CANCEL:
                           if (canModifyPassword)
                           {
                               MessageBox(hwnd, "Votre ancien mot de passe a été rétabli.", "Attention", MB_OK | MB_ICONWARNING);
                               canModifyPassword = 0;
                           }
                           EndDialog(hwnd, 0);
                           return FALSE;
                      case IDP_OK:
                      {
                           char pwd[MAX_CHAINE] = "\0";
                           GetDlgItemText(hwnd, IDE_PWD, pwd, MAX_CHAINE);
                           GetDlgItemText(hwnd, IDE_PWD2, buffer, MAX_CHAINE);
                           if (strcmp(pwd, buffer) != 0)
                           {
                               MessageBox(hwnd, "Vos mots de passe sont différents !", "Attention", MB_OK | MB_ICONWARNING);
                               return FALSE;
                           }

                           int attributes = 0;
                           if (IsDlgButtonChecked(hwnd, IDC_READ))
                              attributes = FILE_SHARE_READ;
                           if (IsDlgButtonChecked(hwnd, IDC_MODIF))
                              attributes = attributes | FILE_SHARE_WRITE;
                           if (IsDlgButtonChecked(hwnd, IDC_DELETE))
                              attributes = attributes | FILE_SHARE_DELETE;

                           strcpy(protectionPassword, buffer);

                           ready_to_work(TW_PROTECT);
                           int i;
                           for (i = 0 ; tableauFichiersACopier[i] != NULL ; i++)
                               PL_add_file(tableauFichiersACopier[i], attributes);

                           if (PL_launch_protection(0))
                              EndDialog(hwnd, 1);

                           canModifyPassword = 0;
                           return FALSE;
                      }
                }

                return FALSE;
           case WM_CLOSE:
                SendMessage(hwnd, WM_COMMAND, IDP_CANCEL, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK DisprotectionProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                HWND listWnd = CreateWindowEx(
                      WS_EX_CLIENTEDGE,
                      WC_LISTVIEW,
                      "",
                      WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_SORTASCENDING,
                      15,
                      15,
                      435,
                      270,
                      hwnd,
                      (HMENU) IDLV_LIST,
                      mainInstance,
                      NULL
                   );

                strcpy(buffer, "Nom");
                LVCOLUMN lvcol;
                lvcol.mask = LVCF_WIDTH | LVCF_TEXT;
                lvcol.cx = 220;
                lvcol.pszText = buffer;
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_FILENAME, &lvcol);

                lvcol.cx = 100;
                strcpy(buffer, "Type");
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_FILETYPE, &lvcol);

                lvcol.cx = 80;
                strcpy(buffer, "Autorisation");
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_FILEPROTECTION, &lvcol);

                lvcol.cx = 200;
                strcpy(buffer, "Répertoire");
                lvcol.cchTextMax = strlen(buffer) + 1;
                ListView_InsertColumn(listWnd, IDCOL_FILEDATE, &lvcol);

                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconNoShield);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconNoShield);

                if (protectionPassword[0] != '\0' && !DialogBoxParam(mainInstance, "Password", mainWnd, (DLGPROC)PasswordProc, (LPARAM)protectionPassword))
                {
                   MessageBox(hwnd, "Mot de passe incorrect !", "Erreur", MB_OK | MB_ICONWARNING);
                   EndDialog(hwnd, 0);
                   return FALSE;
                }


                FILE *file = fopen(PL_listFileName, "r");
                if (file == NULL)
                {
                         EndDialog(hwnd, 0);
                         add_bilan_text("Ajout de fichier protégé", "Impossible d'ouvrir la liste", NULL, PL_listFileName, NULL, ABT_GETERRNO);
                         MessageBox(hwnd, "Impossible de lister les fichiers protégés !", "Erreur !", MB_OK | MB_ICONERROR);
                         return FALSE;
                }

                char buffer2[MAX_CHAINE] = "\0";
                int attributes = 0;
                while (fgets(buffer, MAX_CHAINE, file) != NULL)
                {
                      if (strchr(buffer, '\n') != NULL)
                         *strchr(buffer, '\n') = '\0';

                      fgets(buffer2, MAX_CHAINE, file);
                      attributes = strtol(buffer2, NULL, 10);
                      ajouter_fichier_special(buffer, listWnd, attributes);
                }

                fclose(file);
                return FALSE;
           }
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_OK:
                           EndDialog(hwnd, 1);
                           return FALSE;
                      case IDP_ALL:
                           ListView_SetItemState(GetDlgItem(hwnd, IDLV_LIST), -1, LVIS_SELECTED, LVIS_SELECTED);
                           return FALSE;
                      case IDP_DISPROTECT:
                      {
                           HWND listWnd = GetDlgItem(hwnd, IDLV_LIST);
                           int count = ListView_GetSelectedCount(listWnd);

                           if (count > 1)
                              sprintf(buffer, "Voulez-vous vraiment déprotéger ces %d éléments ?", count);
                           else if (count == 1)
                                strcpy(buffer, "Voulez-vous vraiment déprotéger cet élément ?");
                           else return FALSE;

                           if (MessageBox(hwnd, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) != IDYES)
                              return FALSE;

                           int i;
                           char name[MAX_CHAINE] = "\0", directory[MAX_CHAINE] = "\0";

                           int *tab = NULL;
                           count = get_selected_items(listWnd, &tab);

                           for (i = 0 ; i < count ; i++)
                           {
                               ListView_GetItemText(listWnd, tab[i], IDCOL_FILENAME, name, MAX_CHAINE);
                               ListView_GetItemText(listWnd, tab[i], IDCOL_FILEDATE, directory, MAX_CHAINE);
                               sprintf(buffer, "%s\\%s", directory, name);
                               PL_remove_file(buffer);

                               ListView_DeleteItem(listWnd, tab[i]);
                           }

                           free(tab);

                           if (!test_vide(PL_listFileName) && PL_stop_protection(0))
                              MessageBox(mainWnd, "Protection arrêtée avec succès !", "Information", MB_OK | MB_ICONINFORMATION);

                           return FALSE;
                      }
                      case IDP_ADD:
                      {
                           HWND listWnd = GetDlgItem(hwnd, IDLV_LIST);
                           int count = ListView_GetItemCount(listWnd);

                           int i;
                           char name[MAX_CHAINE] = "\0", directory[MAX_CHAINE] = "\0";

                           for (i = 0 ; i < count ; i++)
                           {
                               if (is_item_selected(listWnd, i))
                               {
                                   ListView_GetItemText(listWnd, i, IDCOL_FILENAME, name, MAX_CHAINE);
                                   ListView_GetItemText(listWnd, i, IDCOL_FILEDATE, directory, MAX_CHAINE);
                                   sprintf(buffer, "%s\\%s", directory, name);
                                   ajouter_fichier(buffer, 0, NULL);
                               }
                           }

                           return FALSE;
                      }
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}



LRESULT CALLBACK PasswordProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconEncrypt);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconEncrypt);

                SetDlgItemText(hwnd, IDE_MEMORY, (char*) lParam);
                SetFocus(GetDlgItem(hwnd, IDE_PWD));
                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_OK:
                      {
                           char comparedString[MAX_CHAINE] = "\0";
                           GetDlgItemText(hwnd, IDE_PWD, comparedString, MAX_CHAINE);
                           GetDlgItemText(hwnd, IDE_MEMORY, buffer, MAX_CHAINE);
                           if (strcmp(comparedString, buffer) != 0)
                              EndDialog(hwnd, 0);
                           else EndDialog(hwnd, 1);

                           return FALSE;
                      }
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK BeginSearchProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int Select;

    switch (msg)
    {
           case WM_INITDIALOG:
                SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)iconSearch);
                SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)iconSearch);

                if (cassSensitiveResearch)
                   CheckDlgButton(hwnd, IDC_CASSSENSITIVE, BST_CHECKED);
                if (wildcardsResearch)
                   CheckDlgButton(hwnd, IDC_WILDCARDS, BST_CHECKED);
                if (recursiveResearch)
                   CheckDlgButton(hwnd, IDC_RECURSIVE, BST_CHECKED);

                if (dirsResearch && filesResearch)
                   CheckDlgButton(hwnd, IDR_INCLUDEBOTH, BST_CHECKED);
                else if (dirsResearch)
                     CheckDlgButton(hwnd, IDR_INCLUDEDIRS, BST_CHECKED);
                else CheckDlgButton(hwnd, IDR_INCLUDEFILES, BST_CHECKED);

                return FALSE;
           case WM_COMMAND:
                Select = LOWORD(wParam);

                switch(Select)
                {
                      case IDP_OK:
                           if (IsDlgButtonChecked(hwnd, IDC_CASSSENSITIVE))
                              cassSensitiveResearch = 1;
                           else cassSensitiveResearch = 0;

                           if (IsDlgButtonChecked(hwnd, IDC_WILDCARDS))
                              wildcardsResearch = 1;
                           else wildcardsResearch = 0;

                           if (IsDlgButtonChecked(hwnd, IDC_RECURSIVE))
                              recursiveResearch = 1;
                           else recursiveResearch = 0;

                           if (IsDlgButtonChecked(hwnd, IDR_INCLUDEBOTH) || IsDlgButtonChecked(hwnd, IDR_INCLUDEDIRS))
                              dirsResearch = 1;
                           else dirsResearch = 0;

                           if (IsDlgButtonChecked(hwnd, IDR_INCLUDEBOTH) || IsDlgButtonChecked(hwnd, IDR_INCLUDEFILES))
                              filesResearch = 1;
                           else filesResearch = 0;

                           EndDialog(hwnd, 1);
                           return FALSE;
                      case IDP_CANCEL:
                           EndDialog(hwnd, 0);
                           return FALSE;
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}



int CALLBACK CompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    char text1[MAX_CHAINE],
         text2[MAX_CHAINE],
         buffer[MAX_CHAINE];
    int i;


    for (i = 0 ; i < nombreTotalItems && tableauTotalItems[i].lvi.lParam != lParam1 ; i++);
    if (i < nombreTotalItems)
       strcpy(text1, tableauTotalItems[i].pszText);
    else text1[0] = '\0';

    for (i = 0 ; i < nombreTotalItems && tableauTotalItems[i].lvi.lParam != lParam2 ; i++);
    if (i < nombreTotalItems)
       strcpy(text2, tableauTotalItems[i].pszText);
    else text2[0] = '\0';



    switch (lParamSort)
    {
           case IDCOL_SECTION + 5:
           case IDCOL_ERROR + 5:
           case IDCOL_WHY + 5:
           case IDCOL_FILEERROR + 5:
           case IDCOL_FILETYPE:
           case IDCOL_DIR:
           case IDCOL_FILENAME:
                for (i = 0 ; text1[i] != '\0' && text2[i] != '\0'; i++)
                {
                    text1[i] = toupper(text1[i]);
                    text2[i] = toupper(text2[i]);
                    if (text1[i] > text2[i])
                       return 1;
                    if (text1[i] < text2[i])
                       return -1;
                }

                if (text1[i] == '\0' && text2[i] != '\0')
                   return -1;
                if (text1[i] != '\0' && text2[i] == '\0')
                   return 1;

                return 0;
           case IDCOL_FILESIZE:
           {
                int size1 = strtol(text1, NULL, 10),
                    size2 = strtol(text2, NULL, 10);

                if (size1 > size2)
                   return 1;
                if (size2 > size1)
                   return -1;

                return 0;
           }
           case IDCOL_HOUR + 5:
                sprintf(buffer, "1 1 1 %s", text1);
                strcpy(text1, buffer);
                sprintf(buffer, "1 1 1 %s", text2);
                strcpy(text2, buffer);
           case IDCOL_FILEDATE:
           {
                SYSTEMTIME st1, st2;
                retrieve_system_time(text1, &st1);
                retrieve_system_time(text2, &st2);

                sprintf(buffer, "%04d%02d%02d", st1.wYear, st1.wMonth, st1.wDay);
                int date1 = strtol(buffer, NULL, 10);
                sprintf(buffer, "%02d%02d%02d", st1.wHour, st1.wMinute, st1.wSecond);
                int hour1 = strtol(buffer, NULL, 10);
                sprintf(buffer, "%04d%02d%02d", st2.wYear, st2.wMonth, st2.wDay);
                int date2 = strtol(buffer, NULL, 10);
                sprintf(buffer, "%02d%02d%02d", st2.wHour, st2.wMinute, st2.wSecond);
                int hour2 = strtol(buffer, NULL, 10);


                if (date1 > date2)
                   return 1;
                if (date1 < date2)
                   return -1;
                if (hour1 > hour2)
                   return 1;
                if (hour1 < hour2)
                   return -1;

                return 0;
           }
    }

    return 0;
}



int ajouter_fichier(char nom[], int update, HWND listWnd)
{
    if (nom == NULL)
       return 0;

    if (listWnd == NULL)
       listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);

    char section[MAX_CHAINE] = "Ajout de fichier";
    if (update)
       strcpy(section, "Mise à jour de fichier");

    int itemNumber = find_item(nom);

    if (!update && itemNumber >= 0)
    {
        add_bilan_text(section, "Fichier déjà en place", "\0", nom, NULL, 0);
        return 0;
    }
    if (update && itemNumber < 0)
    {
        add_bilan_text(section, "Fichier absent de la liste", "\0", nom, NULL, 0);
        return 0;
    }

    if (!update)
       itemNumber = 100000;

    HANDLE handle = CreateFile(nom, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    char buffer[MAX_CHAINE], *position = NULL;

    if (handle == INVALID_HANDLE_VALUE)
    {
       add_bilan_text(section, "Impossible d'ouvrir le fichier", NULL, nom, NULL, ABT_GETLASTERROR);
       return 0;
    }

    int size = GetFileSize(handle, NULL);

    SHFILEINFO shfi;
    imgList = (HIMAGELIST) SHGetFileInfo(nom, 0, &shfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_SYSICONINDEX);
    ListView_SetImageList(listWnd, imgList, LVSIL_SMALL);

    BY_HANDLE_FILE_INFORMATION bhfi;
    GetFileInformationByHandle(handle, &bhfi);

    SYSTEMTIME st;
    FileTimeToSystemTime(&bhfi.ftLastWriteTime, &st);

    strcpy(buffer, nom);
    if ((position = strrchr(buffer, '\\')) != NULL)
       strcpy(buffer, position + 1);


    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iItem = itemNumber;
    lvi.iSubItem = 0;
    lvi.pszText = buffer;
    lvi.iImage = shfi.iIcon;

    if (!update)
       lvi.iItem = ListView_InsertItem(listWnd, &lvi);
    else ListView_SetItem(listWnd, &lvi);

    lvi.mask = LVIF_TEXT;
    strcpy(buffer, shfi.szTypeName);
    lvi.iSubItem = IDCOL_FILETYPE;
    ListView_SetItem(listWnd, &lvi);

    sprintf(buffer, "%d Ko", size / 1024);
    lvi.iSubItem = IDCOL_FILESIZE;
    ListView_SetItem(listWnd, &lvi);

    sprintf(buffer, "%02d/%02d/%d à %02d:%02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
    lvi.iSubItem = IDCOL_FILEDATE;
    ListView_SetItem(listWnd, &lvi);

    strcpy(buffer, nom);
    if ((position = strrchr(buffer, '\\')) != NULL)
       *position = '\0';
    else buffer[0] = '\0';
    lvi.iSubItem = IDCOL_DIR;
    ListView_SetItem(listWnd, &lvi);

    if (!update)
       attrib_lParams(listWnd);

    CloseHandle(handle);
    return 1;
}



int ajouter_fichier_special(char nom[], HWND listWnd, int attributes)
{
    if (nom == NULL || listWnd == NULL)
       return 0;

    char section[MAX_CHAINE] = "Ajout de fichier (protégé)";

    int itemNumber = 100000;

    if ((GetFileAttributes(nom) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
       add_bilan_text(section, "Le fichier est un répertoire", "\0", nom, NULL, 0);
       return 0;
    }

    char buffer[MAX_CHAINE], *position = NULL;

    char *extension = strrchr(nom, '.');
    sprintf(buffer, "%s\\bingo", dossierBase);
    if (extension != NULL)
       strcat(buffer, extension);
    FILE *file = fopen(buffer, "w");
    if (file != NULL)
       fclose(file);
    else
    {
        add_bilan_text(section, "Impossible de créer le fichier tampon", NULL, buffer, NULL, ABT_GETERRNO);
        strcpy(buffer, nom);
    }

    SHFILEINFO shfi;
    imgList = (HIMAGELIST) SHGetFileInfo(buffer, 0, &shfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
    ListView_SetImageList(listWnd, imgList, LVSIL_SMALL);

    strcpy(buffer, nom);
    if ((position = strrchr(buffer, '\\')) != NULL)
       strcpy(buffer, position + 1);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iItem = itemNumber;
    lvi.iSubItem = 0;
    lvi.pszText = buffer;
    lvi.iImage = shfi.iIcon;

    lvi.iItem = ListView_InsertItem(listWnd, &lvi);

    lvi.mask = LVIF_TEXT;
    strcpy(buffer, shfi.szTypeName);
    lvi.iSubItem = IDCOL_FILETYPE;
    ListView_SetItem(listWnd, &lvi);

    buffer[0] = '\0';
    if ((attributes & FILE_SHARE_READ) == FILE_SHARE_READ)
       strcpy(buffer, "L");
    if ((attributes & FILE_SHARE_WRITE) == FILE_SHARE_WRITE)
       strcat(buffer, "E");
    if ((attributes & FILE_SHARE_DELETE) == FILE_SHARE_DELETE)
       strcat(buffer, "S");
    if (buffer[0] == '\0')
       strcpy(buffer, "Aucune");
    lvi.iSubItem = IDCOL_FILEPROTECTION;
    ListView_SetItem(listWnd, &lvi);

    strcpy(buffer, nom);
    if ((position = strrchr(buffer, '\\')) != NULL)
       *position = '\0';
    else buffer[0] = '\0';
    lvi.iSubItem = IDCOL_FILEDATE;
    ListView_SetItem(listWnd, &lvi);

    return 1;
}



int ajouter_liste(char nomFichier[], HWND listWnd)
{
    HWND wndStatus = CreateDialog(mainInstance, "IsSearchingDlg", NULL, (DLGPROC)IsSearchingDlgProc);

    char buffer[MAX_CHAINE];
    SetDlgItemText(wndStatus, IDT_FILENAME, "Veuillez patienter, ajout à la liste en cours...");

    FILE *file = NULL;
    if ((file = fopen(nomFichier, "r")) == NULL)
    {
       add_bilan_text("Ajout de fichier", "Impossible d'ouvrir la liste de fichiers", NULL, nomFichier, NULL, ABT_GETERRNO);
       return 0;
    }

    char *position = NULL, fileName[MAX_CHAINE];

    int count;
    for (count = 0 ; fgets(fileName, MAX_CHAINE, file) != NULL ; count++);
    rewind(file);

    sprintf(buffer, "Trouvés : %d", count);
    SetDlgItemText(wndStatus, IDT_NUMBERFILESFOUND, buffer);
    count = 0;

    int lastUpdate = GetTickCount(), quit = 0, cursorChanged = 0;
    HCURSOR previousCursor,
            cursorHand = LoadCursor(NULL, IDC_HAND);


    while (fgets(fileName, MAX_CHAINE, file) != NULL && !quit)
    {
        if ((position = strchr(fileName, '\n')) != NULL)
           *position = '\0';

        ajouter_fichier(fileName, 0, listWnd);

        count++;
        sprintf(buffer, "Ajoutés : %d", count);
        SetDlgItemText(wndStatus, IDT_NUMBERFILESADDED, buffer);

        RECT rect;
        GetWindowRect(GetDlgItem(wndStatus, IDP_CANCEL), &rect);
        POINT pt;
        GetCursorPos(&pt);

        if (GetForegroundWindow() == wndStatus && pt.x >= rect.left && pt.x <= rect.right && pt.y <= rect.bottom && pt.y >= rect.top)
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

        if (GetTickCount() - lastUpdate > 1000)
        {
               GetClientRect(wndStatus, &rect);
               InvalidateRect(wndStatus, &rect, FALSE);
               UpdateWindow(wndStatus);
        }
    }

    fclose(file);
    DestroyWindow(wndStatus);
    DeleteObject(cursorHand);
    return 1;
}


int is_file_enabled_to_copy(char name[], char destination[])
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    int enable = 1, state;
    char buffer[MAX_CHAINE];

    GetFileAttributesEx(name, GetFileExInfoStandard, &wfad);

    if ((wfad.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE)
    {
        state = IsDlgButtonChecked(optnDlgWnd, IDC_ARCHIVE);
        if (state == BST_UNCHECKED)
           enable = 0;
        else if (state == BST_INDETERMINATE)
        {
            sprintf(buffer, "Voulez-vous traiter le fichier\n%s\nclassé archive ?", name);
            if (MessageBox(mainWnd, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
            {
               add_bilan_text("Début de traitement", "Fichier archive annulé par l'utilisateur", "\0", name, NULL, 0);
               enable = 0;
            }
        }
    }

    if ((wfad.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
    {
        state = IsDlgButtonChecked(optnDlgWnd, IDC_HIDDEN);
        if (state == BST_UNCHECKED)
           enable = 0;
        else if (state == BST_INDETERMINATE)
        {
            sprintf(buffer, "Voulez-vous traiter le fichier caché\n%s ?", name);
            if (MessageBox(mainWnd, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
            {
               add_bilan_text("Début de traitement", "Fichier caché annulé par l'utilisateur", "\0", name, NULL, 0);
               enable = 0;
            }
        }
    }

    if ((wfad.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
    {
        state = IsDlgButtonChecked(optnDlgWnd, IDC_READONLY);
        if (state == BST_UNCHECKED)
           enable = 0;
        else if (state == BST_INDETERMINATE)
        {
            sprintf(buffer, "Voulez-vous traiter le fichier\n%s\nen lecture seule ?", name);
            if (MessageBox(mainWnd, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
            {
               add_bilan_text("Début de traitement", "Fichier en lecture seule annulé par l'utilisateur", "\0", name, NULL, 0);
               enable = 0;
            }
        }
    }

    if ((wfad.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
    {
        state = IsDlgButtonChecked(optnDlgWnd, IDC_SYSTEM);
        if (state == BST_UNCHECKED)
           enable = 0;
        else if (state == BST_INDETERMINATE)
        {
            sprintf(buffer, "Voulez-vous traiter le fichier système\n%s ?", name);
            if (MessageBox(mainWnd, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
            {
               add_bilan_text("Début de traitement", "Fichier système annulé par l'utilisateur", "\0", name, NULL, 0);
               enable = 0;
            }
        }
    }

    if (destination != NULL && destination[0] != '\0' && strcmp(destination, name) != 0 && test_exist(destination) == 1)
    {
        state = IsDlgButtonChecked(optnDlgWnd, IDC_REPLACE);
        if (state == BST_UNCHECKED)
           enable = 0;
        else if (state == BST_INDETERMINATE)
        {
            sprintf(buffer, "%s\nCe fichier existe déjà. Remplacer ?", destination);
            if (MessageBox(mainWnd, buffer, "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDNO)
            {
               add_bilan_text("Début de traitement", "Remplacement de fichier annulé par l'utilisateur", "\0", name, NULL, 0);
               enable = 0;
            }
        }
    }

    return enable;
}


int draw_progress_bar(int percent, int number)
{
     if (percent > 100)
        percent = 100;
     if (percent < 0)
        percent = 100;

     HWND progressBarWnd = NULL;
     char buffer[MAX_CHAINE];
     HDC hdc;

     if (number == 1)
        progressBarWnd = GetDlgItem(stateDlgWnd, IDPB_FILE);
     else if (number == 2)
          progressBarWnd = GetDlgItem(stateDlgWnd, IDPB_COPY);
     else return 0;

     hdc = GetDC(mainWnd);
     if (hdc == NULL)
        return 0;

     SetTextAlign(hdc, TA_RIGHT);
     RECT tailleProgressBar;
     GetWindowRect(progressBarWnd, &tailleProgressBar);
     RECT rect;
     GetWindowRect(mainWnd, &rect);

     sprintf(buffer, "%d %%", percent);
     SetBkMode(hdc, OPAQUE);
     TextOut(hdc, tailleProgressBar.right - rect.left - edge, tailleProgressBar.bottom - rect.top - caption - edge - ymenu, buffer, strlen(buffer));
     SendMessage(progressBarWnd, PBM_SETPOS, percent, 0);

     DeleteDC(hdc);

     if (number == 2)
     {
        if (percent >= 100)
           strcpy(buffer, "XCopy");
        else sprintf(buffer, "XCopy - %d %%", percent);
        SetWindowText(mainWnd, buffer);
     }

     return 1;
}



int is_item_selected(HWND listWnd, int item)
{
    int state = LOBYTE(ListView_GetItemState(listWnd, item, LVIS_SELECTED));
    if ((state & LVIS_SELECTED) == LVIS_SELECTED)
       return 1;
    else return 0;
}


int get_selected_items(HWND listWnd, int **ptOut)
{
    int count = ListView_GetItemCount(listWnd),
        selected = ListView_GetSelectedCount(listWnd);

    *ptOut = (int*) calloc(selected, sizeof(int));

    int i, j = 0;
    for (i = count - 1 ; i >= 0 ; i--)
    {
        if (is_item_selected(listWnd, i))
        {
           (*ptOut)[j] = i;
           j++;
        }
    }

    return selected;
}


void retrieve_system_time(char text[], SYSTEMTIME *st)
{
     //Format de la chaine : Day Month Year Hour Minute Second
     //Tout caractère différent d'un chiffre peut être inséré entre ces éléments

     int i;
     char *position;

     for (i = 0 ; text[i] != '\0' && (text[i] < '0' || text[i] > '9') ; i++);
     if (text[i] == '\0')
        return;
     st->wDay = strtol(text + i, &position, 10);

     for (i = (int)(position - text); text[i] != '\0' && (text[i] < '0' || text[i] > '9') ; i++);
     if (text[i] == '\0')
        return;
     st->wMonth = strtol(text + i, &position, 10);

     for (i = (int)(position - text); text[i] != '\0' && (text[i] < '0' || text[i] > '9') ; i++);
     if (text[i] == '\0')
        return;
     st->wYear = strtol(text + i, &position, 10);

     for (i = (int)(position - text); text[i] != '\0' && (text[i] < '0' || text[i] > '9') ; i++);
     if (text[i] == '\0')
        return;
     st->wHour = strtol(text + i, &position, 10);

     for (i = (int)(position - text); text[i] != '\0' && (text[i] < '0' || text[i] > '9') ; i++);
     if (text[i] == '\0')
        return;
     st->wMinute = strtol(text + i, &position, 10);

     for (i = (int)(position - text); text[i] != '\0' && (text[i] < '0' || text[i] > '9') ; i++);
     if (text[i] == '\0')
        return;
     st->wSecond = strtol(text + i, &position, 10);

     return;
}


int ready_to_work(int typeWork)
{
     char name[MAX_CHAINE],
          directory[MAX_CHAINE],
          destination[MAX_CHAINE],
          buffer[MAX_CHAINE];

     GetDlgItemText(optnDlgWnd, IDE_DIROUT, destination, MAX_CHAINE);

     HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);
     int i, j = 1, k = 0;

     nombreFichiersACopier = ListView_GetItemCount(listWnd);
     if (typeWork == TW_COPY || typeWork == TW_ENCRYPT || typeWork == TW_MOVE || typeWork == TW_DECRYPT)
        strcpy(tableauFichiersACopier[0], destination);
     else if (typeWork != TW_RENAME)
          j = 0;
     else j = 2;
     tailleTotaleFichiersACopier = 0;
     FILE *file = NULL;

     int count = nombreFichiersACopier, taille = 0;
     if (onlySelected)
        nombreFichiersACopier = ListView_GetSelectedCount(listWnd);

     if (typeWork == TW_COPY || typeWork == TW_MOVE || typeWork == TW_DELETE || typeWork == TW_RENAME)
     {
         for (i = 0 ; i < count ; i++)
         {
             if ((!onlySelected || is_item_selected(listWnd, i)) && is_item_directory(i))
             {

                  ListView_GetItemText(listWnd, i, IDCOL_FILENAME, name, MAX_CHAINE);
                  ListView_GetItemText(listWnd, i, IDCOL_DIR, directory, MAX_CHAINE);
                  sprintf(buffer, "%s\\%s", directory, name);

                  if (tableauDossiersACopier[k] == NULL)
                     tableauDossiersACopier[k] = malloc(MAX_CHAINE);
                  strcpy(tableauDossiersACopier[k], buffer);

                  k++;
             }
         }
     }

     if (tableauDossiersACopier[k] != NULL)
        free(tableauDossiersACopier[k]);
     tableauDossiersACopier[k] = NULL;


     for (i = 0 ; i < count ; i++)
     {
         if ((!onlySelected || is_item_selected(listWnd, i)) && !is_item_directory(i))
         {
             ListView_GetItemText(listWnd, i, IDCOL_FILENAME, name, MAX_CHAINE);
             ListView_GetItemText(listWnd, i, IDCOL_DIR, directory, MAX_CHAINE);
             sprintf(buffer, "%s\\%s", directory, name);
             strcpy(directory, buffer);

             if (typeWork == TW_COPY || typeWork == TW_ENCRYPT || typeWork == TW_MOVE || typeWork == TW_DECRYPT)
                file = fopen(directory, "rb");
             if (file != NULL)
             {
                 fseek(file, 0, SEEK_END);
                 taille = ftell(file) / (1.0 * pow(2,10));
                 tailleTotaleFichiersACopier += taille;
                 fclose(file);
             }
             else if (typeWork == TW_COPY || typeWork == TW_ENCRYPT || typeWork == TW_MOVE || typeWork == TW_DECRYPT)
                  add_bilan_text("Début de traitement", "Impossible d'ouvrir le fichier", "Inconnue", directory, NULL, ABT_GETERRNO);

             if (typeWork == TW_COPY || typeWork == TW_MOVE || typeWork == TW_ENCRYPT || typeWork == TW_DECRYPT)
                correct_adress(buffer, tableauFichiersACopier[0]);
             else sprintf(buffer, "%s\\%s", destination, name);

             if (typeWork == TW_ENCRYPT && encryptionExtension[0] != '\0')
                  sprintf(buffer, "%s.%s", buffer, encryptionExtension);
             else if (typeWork == TW_DECRYPT)
             {
                  if (deleteOldExtension)
                     delete_second_extension(buffer);
                  else if (encryptionExtension[0] != '\0')
                       sprintf(buffer, "%s.%s", buffer, encryptionExtension);
             }

             if (typeWork == TW_PROTECT || typeWork == TW_DISPROTECT || typeWork == TW_RENAME)
                buffer[0] = '\0';

             if (j < MAX_ITEMS && (typeWork != TW_MOVE || destination[0] != directory[0]) && is_file_enabled_to_copy(directory, buffer))
             {
                 tableauFichiersACopier[j] = malloc(MAX_CHAINE);
                 strcpy(tableauFichiersACopier[j], directory);
                 j++;
             }
             else
             {
                 nombreFichiersACopier--;
                 tailleTotaleFichiersACopier -= taille;
             }
         }
     }

     tableauFichiersACopier[j] = NULL;

     if (IsDlgButtonChecked(optnDlgWnd, IDC_SIZEMAX))
     {
         GetDlgItemText(optnDlgWnd, IDE_SIZEMAX, buffer, MAX_CHAINE);
         tailleMaxCopie = strtol(buffer, NULL, 10);
     }
     else tailleMaxCopie = -1;

     if (IsDlgButtonChecked(optnDlgWnd, IDC_TIMEMAX))
     {
         GetDlgItemText(optnDlgWnd, IDE_TIMEMAX, buffer, MAX_CHAINE);
         tempsMaxCopie = strtol(buffer, NULL, 10) * 1000;
     }
     else tempsMaxCopie = -1;

     enable_buttons(FALSE);

     heureDebutCopie = GetTickCount();
     return j;
}


void enable_buttons(BOOL state)
{
     EnableWindow(GetDlgItem(optnDlgWnd, IDP_COPY), state);
     EnableWindow(GetDlgItem(optnDlgWnd, IDP_MOVE), state);
     EnableWindow(GetDlgItem(optnDlgWnd, IDP_RENAME), state);
     EnableWindow(GetDlgItem(optnDlgWnd, IDP_ENCRYPT), state);
     EnableWindow(GetDlgItem(optnDlgWnd, IDP_DECRYPT), state);
     EnableWindow(GetDlgItem(optnDlgWnd, IDP_DELETE), state);
     EnableWindow(GetDlgItem(optnDlgWnd, IDP_PROTECT), state);

     MENUITEMINFO mui;
     mui.cbSize = sizeof(MENUITEMINFO);
     mui.fMask = MIIM_STATE;

     if (state == FALSE)
        mui.fState = MFS_DISABLED;
     else mui.fState = MFS_ENABLED;

     SetMenuItemInfo(menuAction, M_ACTION_COPY, FALSE, &mui);
     SetMenuItemInfo(menuAction, M_ACTION_MOVE, FALSE, &mui);
     SetMenuItemInfo(menuAction, M_ACTION_RENAME, FALSE, &mui);
     SetMenuItemInfo(menuAction, M_ACTION_ENCRYPT, FALSE, &mui);
     SetMenuItemInfo(menuAction, M_ACTION_DECRYPT, FALSE, &mui);
     SetMenuItemInfo(menuAction, M_ACTION_DELETE, FALSE, &mui);
     SetMenuItemInfo(menuAction, M_ACTION_PROTECT, FALSE, &mui);

     return;
}


void delete_second_extension(char name[])
{
    char buffer[MAX_CHAINE];
    strcpy(buffer, name);

    char *position = NULL,
         *slash = strrchr(name, '\\');
    if (slash != NULL)
       strcpy(buffer, slash + 1);

    position = strrchr(buffer, '.');
    if (position != NULL)
    {
        *position = '\0';
        position = strchr(buffer, '.');
        if (position != NULL)
        {
           if (slash == NULL)
              strcpy(name, buffer);
           else
           {
               *(slash + 1) = '\0';
               strcat(name, buffer);
           }
        }
    }

    return;
}


int add_bilan_text(char section[], char error[], char why[], char file[], SYSTEMTIME *hour, int flags)
{
    int errorCode = GetLastError(),
        langID = MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH),
        alloc = 0, alloc2 = 0, alloc3 = 0;

    switch(flags)
    {
        case ABT_GETLASTERROR:
             alloc2 = 1;
             FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, langID, (LPSTR)&why, 1, NULL);
             break;
        case ABT_ISLASTERROR:
             alloc2 = 1;
             FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, (int) why, langID, (LPSTR)&why, 1, NULL);
             break;
        case ABT_ISERRNO:
             why = strerror((int) why);
             break;
        case ABT_GETERRNO:
             why = _strerror(NULL);
             break;
    }

    if (hour == NULL)
    {
       alloc = 1;
       hour = malloc(sizeof(SYSTEMTIME));
       GetSystemTime(hour);
    }

    char buffer[MAX_CHAINE];
    if (numeroFichier >= 0 && numeroFichier < MAX_ITEMS && tableauFichiersACopier[numeroFichier] != NULL && strcmp(tableauFichiersACopier[numeroFichier], file) == 0)
    {
        if (file == tableauFichiersACopier[numeroFichier])
        {
           file = malloc(MAX_CHAINE);
           alloc3 = 1;
           strcpy(file, tableauFichiersACopier[numeroFichier]);
        }

        strcpy(buffer, tableauFichiersACopier[numeroFichier]);
        buffer[5] = '\0';
        if (strcmp(buffer, "[ERR]") != 0)
        {
           sprintf(buffer, "[ERR] %s", tableauFichiersACopier[numeroFichier]);
           strcpy(tableauFichiersACopier[numeroFichier], buffer);
        }
    }

    strcpy(buffer, section);
    HWND listWnd = GetDlgItem(bilanWnd, IDLV_LIST);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.iItem = 100000;
    lvi.iSubItem = 0;
    lvi.pszText = buffer;

    lvi.iItem = ListView_InsertItem(listWnd, &lvi);

    strcpy(buffer, error);
    lvi.iSubItem = IDCOL_ERROR;
    ListView_SetItem(listWnd, &lvi);

    strcpy(buffer, why);
    if (alloc2 && strchr(buffer, '\r') != NULL)
       *strchr(buffer, '\r') = '\0';
    lvi.iSubItem = IDCOL_WHY;
    ListView_SetItem(listWnd, &lvi);

    strcpy(buffer, file);
    lvi.iSubItem = IDCOL_FILEERROR;
    ListView_SetItem(listWnd, &lvi);

    sprintf(buffer, "%02d:%02d:%02d", hour->wHour, hour->wMinute, hour->wSecond);
    lvi.iSubItem = IDCOL_HOUR;
    ListView_SetItem(listWnd, &lvi);

    if (alloc)
       free(hour);
    if (alloc2)
       LocalFree(why);
    if (alloc3)
       free(file);

    attrib_lParams(listWnd);
    return 1;
}


void attrib_lParams(HWND listWnd)
{
     if (listWnd == NULL)
        return;

     LVITEM lvi;
     lvi.mask = LVIF_PARAM;
     lvi.iSubItem = 0;

     int number = ListView_GetItemCount(listWnd);
     int i;
     for (i = 0 ; i < number ; i++)
     {
         lvi.iItem = i;
         lvi.lParam = i;
         ListView_SetItem(listWnd, &lvi);
     }

     return;
}


int find_item(char fileAdress[])
{
    HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);

    char *position = strrchr(fileAdress, '\\');
    if (position == NULL)
       return -1;

    char name[MAX_CHAINE];
    strcpy(name, position + 1);

    char adress[MAX_CHAINE];
    strcpy(adress, fileAdress);
    adress[position - fileAdress] = '\0';

    char buffer[MAX_CHAINE];
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_STRING;
    lvfi.psz = name;
    int result = -1, continuer = 1;

    do
    {
        result = ListView_FindItem(listWnd, result, &lvfi);
        if (result >= 0)
        {
           ListView_GetItemText(listWnd, result, IDCOL_DIR, buffer, MAX_CHAINE);
           if (strcmp(buffer, adress) == 0)
              continuer = 0;
        }
        else break;
    } while (continuer);

    return result;
}


int delete_item(char adressFile[])
{
    int index = find_item(adressFile);
    if (index >= 0)
    {
       ListView_DeleteItem(GetDlgItem(dragAndDropWnd, IDLV_LIST), index);
       return 1;
    }
    else return 0;
}


void free_table()
{
     int i;
     for (i = 1 ; i < MAX_ITEMS ; i++)
     {
         if (tableauFichiersACopier[i] != NULL)
            free(tableauFichiersACopier[i]);
         tableauFichiersACopier[i] = NULL;
     }

     return;
}


void retrieve_error_text_for_SHFileOperation(int errorCode, char text[])
{
     strcpy(text, "Erreur d'origine inconnue");

     switch (errorCode)
     {
            case 0x71:
                 strcpy(text, "Source et cible identiques");
                 break;
            case 0x72:
                 strcpy(text, "Sources multiples, impossible de traiter");
                 break;
            case 0x73:
                 strcpy(text, "Répertoires différents, impossible de renommer le fichier");
                 break;
            case 0x74:
                 strcpy(text, "Impossible de bouger ou renommer un répertoire système");
                 break;
            case 0x75:
                 strcpy(text, "Opération annulée");
                 break;
            case 0x76:
                 strcpy(text, "La cible est (dans) un sous-dossier de la source");
                 break;
            case 0x78:
                 strcpy(text, "Accès refusé");
                 break;
            case 0x79:
                 strcpy(text, "Nom de la cible ou de la source trop long");
                 break;
            case 0x7A:
                 strcpy(text, "Cibles multiples, impossible de déplacer");
                 break;
            case 0x7C:
                 strcpy(text, "Source ou cible invalide");
                 break;
            case 0x7D:
                 strcpy(text, "La source et la cible ont le même répertoire parent");
                 break;
            case 0x7E:
                 strcpy(text, "La cible est un fichier existant déjà");
                 break;
            case 0x80:
                 strcpy(text, "La cible est un répertoire existant déjà");
                 break;
            case 0x81:
                 strcpy(text, "Le nom du fichier est trop long");
                 break;
            case 0x82:
                 strcpy(text, "La cible est un CD-ROM en lecture seule, probablement non formaté");
                 break;
            case 0x83:
                 strcpy(text, "La cible est un DVD en lecture seule, probablement non formaté");
                 break;
            case 0x84:
                 strcpy(text, "La cible est un CD inscriptible, probablement non formaté");
                 break;
            case 0x85:
                 strcpy(text, "Le fichier est trop volumineux");
                 break;
            case 0x86:
                 strcpy(text, "La source est un CD-ROM en lecture seule, probablement non formaté");
                 break;
            case 0x87:
                 strcpy(text, "La source est un DVD en lecture seule, probablement non formaté");
                 break;
            case 0x88:
                 strcpy(text, "La source est un CD inscriptible, probablement non formaté");
                 break;
            case 0xB7:
                 strcpy(text, "Une opération sur un nom de fichier demande trop de mémoire et a échoué");
                 break;
            case 0x402:
                 strcpy(text, "Erreur d'origine inconnue - généralement due à une source ou une cible incorrecte");
                 break;
            case 0x10000:
                 strcpy(text, "Erreur d'origine inconnue, concernant le nom de la cible");
                 break;
            case 0x10074:
                 strcpy(text, "La cible est un dossier système et ne peut être renommée");
                 break;
            default:
                 FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH), text, MAX_CHAINE, NULL);
                 break;
     }

     return;
}



int PL_add_file(char fileName[], int attributes)
{
    char buffer[MAX_CHAINE];
    PL_remove_file(fileName);

    FILE *file = NULL;
    if ((file = fopen(PL_listFileName, "r+")) == NULL)
    {
        sprintf(buffer, "Impossible de protéger le fichier %s.", fileName);
        MessageBox(NULL, buffer, "Erreur !", MB_OK | MB_ICONERROR);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    fprintf(file, "%s\n%d\n", fileName, attributes);
    fclose(file);
    return 1;
}


int PL_remove_file(char fileName[])
{
    char buffer[MAX_CHAINE];

    FILE *file = NULL;
    if ((file = fopen(PL_listFileName, "r+")) == NULL)
    {
        sprintf(buffer, "Impossible de déprotéger le fichier %s.", fileName);
        MessageBox(NULL, buffer, "Erreur !", MB_OK | MB_ICONERROR);
        return 0;
    }

    int i;
    for (i = 1 ; fgets(buffer, MAX_CHAINE, file) != NULL ; i++)
    {
          *strrchr(buffer, '\n') = '\0';
          if (strcmp(buffer, fileName) == 0)
             break;
    }

    fclose(file);
    if (strcmp(buffer, fileName) != 0)
       return 0;

    if (supprimer_ligne(PL_listFileName, MAX_CHAINE, i) && supprimer_ligne(PL_listFileName, MAX_CHAINE, i) && PL_launch_protection(0))
       return 1;
    else return 0;
}


int PL_launch_protection(int forcer)
{
    HANDLE handle = NULL;
    if (!forcer)
       handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "xcopy-protection");

    char *buffer = malloc(MAX_CHAINE);
    if (buffer == NULL)
       return 0;
    sprintf(buffer, "/%s", PL_listFileName);

    if (handle == NULL)
    {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = FALSE;

        char secu[MAX_CHAINE] = "D:(D;CIOI;SD;;;WD)";
        PSECURITY_DESCRIPTOR psd = NULL;
        int size = 0;

        if (!ConvertStringSecurityDescriptorToSecurityDescriptor(secu, 1, &psd, (PULONG)&size))
           MessageBox(mainWnd, "erreur", "inf", MB_OK);
        sa.lpSecurityDescriptor = psd;

        STARTUPINFO si;
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);

        PROCESS_INFORMATION pi;

        char commandLine[MAX_CHAINE];
        sprintf(commandLine, "%s /%s", PL_executable, PL_listFileName);
        CreateProcess(NULL,
                      commandLine,
                      &sa,
                      &sa,
                      FALSE,
                      0,
                      NULL,
                      NULL,
                      &si,
                      &pi);

        LocalFree(psd);
        PL_processHandle = pi.hProcess;
    }

    int i;
    for (i = 0 ; i < 5 && handle == NULL ; i++)
    {
        Sleep(1000);
        handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "xcopy-protection");
    }

    free(buffer);
    if (handle != NULL)
    {
        buffer = (char*) MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, MAX_CHAINE);
        if (buffer == NULL)
            MessageBox(NULL, "Impossible de transmettre l'ordre de mise en route.", "Erreur !", MB_OK | MB_ICONERROR);
        else
        {
            strcpy(buffer, "update");
            UnmapViewOfFile(buffer);
        }
        CloseHandle(handle);

        return 1;
    }

    MessageBox(NULL, "Impossible de lancer la protection.", "Erreur !", MB_OK | MB_ICONERROR);
    return 0;
}


int PL_verify_protection()
{
    if (PL_processHandle == NULL || !GetProcessId(PL_processHandle))
       return 0;

    HANDLE handle = OpenFileMapping(FILE_MAP_READ, FALSE, "xcopy-protection");
    if (handle == NULL)
       return 0;
    CloseHandle(handle);

    return 1;
}


int PL_stop_protection()
{
    HANDLE handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "xcopy-protection");

    char *buffer = NULL;
    if (handle != NULL)
    {
        buffer = (char*) MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, MAX_CHAINE);
        if (buffer == NULL)
            MessageBox(NULL, "Impossible de transmettre l'ordre d'arrêt.", "Erreur !", MB_OK | MB_ICONERROR);
        else
        {
            effacer_fichier(PL_listFileName);
            KillTimer(mainWnd, TIMERID_PROTECTION);
            protection = 0;
            strcpy(buffer, "close");
            UnmapViewOfFile(buffer);
        }
        CloseHandle(handle);

        PL_processHandle = NULL;
        return 1;
    }

    MessageBox(NULL, "Impossible de stopper la protection.", "Erreur !", MB_OK | MB_ICONERROR);
    return 0;
}


int create_toolTips()
{
    char buffer[MAX_CHAINE] = "\0";

    if (tableauToolTips[0] != NULL)
       DestroyWindow(tableauToolTips[0]);

    TOOLINFO ti;
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = optnDlgWnd;
    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_COPY);
    ti.hinst = mainInstance;
    ti.lpszText = "Copier les fichiers";
    ti.lParam = 0;

    HWND hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[0] = hwndTip;

    if (tableauToolTips[1] != NULL)
       DestroyWindow(tableauToolTips[1]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_MOVE);
    ti.lpszText = "Déplacer les fichiers";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   0,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[1] = hwndTip;

    if (tableauToolTips[2] != NULL)
       DestroyWindow(tableauToolTips[2]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_DELETE);
    ti.lpszText = "Envoyer les fichiers à la corbeille";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   0,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[2] = hwndTip;

    if (tableauToolTips[3] != NULL)
       DestroyWindow(tableauToolTips[3]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_RENAME);
    ti.lpszText = "Renommer les fichiers";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   0,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[3] = hwndTip;

    if (tableauToolTips[4] != NULL)
       DestroyWindow(tableauToolTips[4]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_ENCRYPT);
    ti.lpszText = "Crypter les fichiers";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   0,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[4] = hwndTip;

    if (tableauToolTips[5] != NULL)
       DestroyWindow(tableauToolTips[5]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_DECRYPT);
    ti.lpszText = "Décrypter les fichiers";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   0,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[5] = hwndTip;

    if (tableauToolTips[6] != NULL)
       DestroyWindow(tableauToolTips[6]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_PROTECT);
    ti.lpszText = "Protéger les fichiers";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[6] = hwndTip;

    if (tableauToolTips[7] != NULL)
       DestroyWindow(tableauToolTips[7]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_DISPROTECT);
    ti.lpszText = "Déprotéger les fichiers";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[7] = hwndTip;

    if (tableauToolTips[8] != NULL)
       DestroyWindow(tableauToolTips[8]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_BILAN);
    ti.lpszText = "Afficher le bilan des erreurs";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[8] = hwndTip;

    if (tableauToolTips[9] != NULL)
       DestroyWindow(tableauToolTips[9]);

    ti.uId = (int)GetDlgItem(optnDlgWnd, IDP_STOP);
    ti.lpszText = "Arrêter l'opération en cours";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   optnDlgWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[9] = hwndTip;

    if (tableauToolTips[10] != NULL)
       DestroyWindow(tableauToolTips[10]);

    ti.hwnd = dragAndDropWnd;
    ti.uId = (int)GetDlgItem(dragAndDropWnd, IDLV_LIST);
    ti.lpszText = "Ajoutez les fichiers ici";
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   dragAndDropWnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[10] = hwndTip;

    if (tableauToolTips[11] != NULL)
       DestroyWindow(tableauToolTips[11]);

    ti.hwnd = optnDlgWnd;
    ti.uId = (int)GetDlgItem(ti.hwnd, IDE_DIROUT);
    GetDlgItemText(ti.hwnd, IDE_DIROUT, buffer, MAX_CHAINE);
    ti.lpszText = buffer;
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND | TTF_CENTERTIP;
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   ti.hwnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[11] = hwndTip;

    if (tableauToolTips[12] != NULL)
       DestroyWindow(tableauToolTips[12]);

    ti.uId = (int)GetDlgItem(ti.hwnd, IDE_SEARCHDIR);
    GetDlgItemText(ti.hwnd, IDE_SEARCHDIR, buffer, MAX_CHAINE);
    ti.lpszText = buffer;
    hwndTip = CreateWindowEx(
                   0,
                   TOOLTIPS_CLASS,
                   NULL,
                   WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   CW_USEDEFAULT,
                   ti.hwnd,
                   NULL,
                   mainInstance,
                   NULL);

    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    tableauToolTips[12] = hwndTip;

    return 1;
}


void centrer_fenetre(HWND hwnd1, HWND hwnd2)
{
     RECT screen;
     screen.right = GetSystemMetrics(SM_CXFULLSCREEN);
     screen.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
     screen.top = 0;
     screen.left = 0;

     if (hwnd2 != NULL)
     {
          RECT rect;
          GetWindowRect(hwnd2, &rect);
          GetClientRect(hwnd2, &screen);
          screen.top = rect.top;
          screen.left = rect.left;
     }

     RECT wndRect;
     GetWindowRect(hwnd1, &wndRect);

     SetWindowPos(hwnd1, NULL, (screen.right - (wndRect.right - wndRect.left)) / 2 + screen.left, (screen.bottom - (wndRect.bottom - wndRect.top)) / 2 + screen.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

     return;
}


int is_item_directory (int index)
{
    HWND listWnd = GetDlgItem(dragAndDropWnd, IDLV_LIST);

    char name[MAX_CHAINE] = "\0",
         directory[MAX_CHAINE] = "\0",
         buffer[MAX_CHAINE] = "\0";

    ListView_GetItemText(listWnd, index, IDCOL_FILENAME, name, MAX_CHAINE);
    ListView_GetItemText(listWnd, index, IDCOL_DIR, directory, MAX_CHAINE);
    sprintf(buffer, "%s\\%s", directory, name);

    if (test_exist(buffer) == 2)
       return 1;
    else return 0;
}


int correct_adress(char dirIn[], char dirOut2[])
{
    char directory[MAX_CHAINE] = "\0",
         parentDir[MAX_CHAINE] = "\0",
         dirOut[MAX_CHAINE];

    strcpy(dirOut, dirOut2);
    strcpy(directory, dirIn);
    if (directory[strlen(directory) - 1] == '\\')
       directory[strlen(directory) - 1] = '\0';

    int found = 0;
    char *position = NULL;

    while (!found && (position = strrchr(directory, '\\')) != NULL)
    {
        *position = '\0';
        if (is_dir_included(directory))
           found = 1;
    }

    char buffer[MAX_CHAINE] = "\0";

    if (found)
    {
        strcpy(parentDir, directory);
        while ((position = strrchr(parentDir, '\\')) != NULL)
              *position = '\0';

        while (strcmp(parentDir, directory) != 0)
        {
              *strchr(parentDir, '\0') = '\\';
              if (is_dir_included(parentDir))
                 strcat(dirOut, strrchr(parentDir, '\\'));
        }

        sprintf(buffer, "%s\\%s", dirOut, strrchr(dirIn, '\\') + 1);
        strcpy(dirIn, buffer);
        return 1;
    }

    sprintf(buffer, "%s\\%s", dirOut, strrchr(dirIn, '\\') + 1);
    strcpy(dirIn, buffer);
    return 0;
}


int is_dir_included(char dir[])
{
    int j, i, found = 0;

    char adress[MAX_CHAINE] = "\0",
         directory[MAX_CHAINE] = "\0";

    for (i = 0 ; i < MAX_CHAINE && (i == 0 || dir[i - 1] != '\0') ; i++)
        adress[i] = toupper(dir[i]);

    for (j = 0 ; !found && j < MAX_ITEMS && tableauDossiersACopier[j] != NULL ; j++)
    {
        for (i = 0 ; i < MAX_CHAINE && (i == 0 || tableauDossiersACopier[j][i - 1] != '\0') ; i++)
            directory[i] = toupper(tableauDossiersACopier[j][i]);
        if (strcmp(directory, adress) == 0)
           found = 1;
    }

    return found;
}


int adjust_file_name(char path[])
{
    if (path == NULL)
       return 0;

    int type = test_exist(path), count = 1;
    if (type == 0)
       return 0;

    char buffer[MAX_CHAINE] = "\0",
         *position = strrchr(path, '.'),
         name[MAX_CHAINE] = "\0",
         extension[MAX_CHAINE] = "\0";

    strcpy(name, path);
    if (strchr(name, '.') != NULL)
       *strrchr(name, '.') = '\0';

    if (position != NULL)
       strcpy(extension, position);

    do
    {
         sprintf(buffer, "%s(%d)%s", name, count, extension);
         count++;
    } while (count < 100 && test_exist(buffer) == type);

    if (test_exist(buffer) != type)
    {
       strcpy(path, buffer);
       return 1;
    }
    else return 0;
}
