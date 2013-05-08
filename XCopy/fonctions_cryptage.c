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

#include "fonctions_cryptage.h"

unsigned char *CRYPT_tableauCode[256] = {NULL};
char CRYPT_signature[] = "namcosys_QSystemesCorporation_@LIAS";
int CRYPT_previous = -1;

void CRYPT_reset_count()
{
     CRYPT_previous = -1;
     return;
}


int CRYPT_crypter(void *ptv, int size, char pwd[], int init)
{
    unsigned char caractere,
                  crypte,
                  crypteur;

    char *chaine = (char*)ptv;

    int i, j = -1;
    if (size < 0)
    {
             if (strchr(chaine, '\0') != NULL)
                size = strchr(chaine, '\0') - chaine;
             else return 0;
    }

    if (init > 0)
       j = init - 1;
    else if (init == CRYPT_CONTINUATE)
         j = CRYPT_previous;

    for (i = 0 ; i < size ; i++)
    {
        caractere = chaine[i];
        j++;
        if (j >= strlen(pwd))
           j = 0;
        crypteur = pwd[j];

        crypte = CRYPT_tableauCode[crypteur][caractere];

        chaine[i] = crypte;
    }

    CRYPT_previous = j;

    return 1;
}



int CRYPT_decrypter(void *ptv, int size, char pwd[], int init)
{
    unsigned char crypte,
                  decrypte,
                  decrypteur;

    char *chaine = (char*)ptv;

    if (size < 0)
    {
             if (strchr(chaine, '\0') != NULL)
                size = strchr(chaine, '\0') - chaine;
             else return 0;
    }

    int i, j = -1, k;
    if (init > 0)
       j = init - 1;
    else if (init == CRYPT_CONTINUATE)
         j = CRYPT_previous;

    for (i = 0 ; i < size ; i++)
    {
        crypte = chaine[i];
        j++;
        if (j >= strlen(pwd))
           j = 0;
        decrypteur = pwd[j];

        for (k = 0 ; k < 256 && CRYPT_tableauCode[decrypteur][k] != crypte ; k++);

        if (CRYPT_tableauCode[decrypteur][k] != crypte)
           chaine[i] = ' ';
        else
        {
            decrypte = k;
            chaine[i] = decrypte;
        }
    }

    CRYPT_previous = j;

    return 1;
}



void CRYPT_init()
{
    //préparation du tableau de cryptage
    int i;
    unsigned char algorithme[256] = {111, 164, 136, 130, 18, 147, 113, 232, 188, 68, 148, 30, 127, 13, 63, 72, 191, 86, 138, 47, 241, 79, 59, 161, 222, 230, 196, 186, 220, 89, 108, 2, 128, 55, 175, 228, 27, 226, 118, 114, 6, 158, 7, 170, 217, 244, 218, 248, 85, 167, 253, 198, 0, 143, 154, 38, 174, 1, 75, 110, 239, 254, 141, 91, 243, 153, 181, 37, 189, 87, 214, 69, 77, 215, 40, 184, 182, 133, 121, 78, 109, 93, 249, 151, 185, 8, 58, 56, 104, 22, 238, 172, 210, 173, 233, 229, 221, 207, 23, 82, 25, 76, 187, 42, 169, 54, 32, 157, 163, 159, 246, 92, 119, 46, 5, 64, 57, 236, 105, 194, 81, 171, 67, 35, 74, 116, 152, 90, 44, 88, 240, 26, 60, 224, 49, 155, 140, 227, 102, 251, 211, 179, 71, 160, 73, 99, 201, 180, 178, 234, 177, 213, 132, 255, 51, 223, 139, 120, 48, 204, 219, 245, 34, 19, 43, 237, 16, 96, 21, 17, 62, 61, 208, 168, 144, 235, 50, 117, 142, 225, 162, 45, 125, 106, 115, 10, 14, 122, 83, 137, 28, 24, 252, 231, 183, 212, 131, 29, 66, 39, 200, 80, 129, 205, 247, 176, 134, 3, 95, 112, 36, 135, 166, 242, 126, 41, 84, 15, 202, 206, 156, 12, 195, 97, 199, 65, 146, 150, 33, 149, 101, 103, 20, 192, 193, 145, 216, 197, 53, 203, 124, 98, 165, 190, 107, 209, 31, 100, 70, 52, 4, 250, 123, 9, 94, 11};
    for (i = 0 ; i < 256 ; i++)
    {
           CRYPT_tableauCode[i] = malloc(sizeof(char) * 256);
    }

    CRYPT_copier_chaine((char*)CRYPT_tableauCode[0], (char*)algorithme, 256);

    for (i = 1 ; i < 256 ; i++)
    {
           CRYPT_copier_chaine((char*)CRYPT_tableauCode[i], (char*)&(CRYPT_tableauCode[i - 1][1]), 255);
           CRYPT_tableauCode[i][255] = CRYPT_tableauCode[i - 1][0];
    }

    return;
}


void CRYPT_copier_chaine(char chaine1[], char chaine2[], int tailleMaxChaine)
{
     int i;

     for (i = 0 ; i < tailleMaxChaine ; i++)
     {
         chaine1[i] = chaine2[i];
     }
     return;
}


int CRYPT_encoder_header(char pwd[], char out[], int tailleMax)
{
    tailleMax -= 1;

    char buffer[100];
    strcpy(buffer, CRYPT_signature);
    int taille = strlen(buffer);
    CRYPT_reset_count();
    CRYPT_crypter(buffer, taille, pwd, 0);

    if (taille > tailleMax)
       buffer[tailleMax] = '\0';

    strcpy(out, buffer);

    if (taille > tailleMax)
       return tailleMax;
    else return taille;
}


int CRYPT_tester_header(FILE *file, char pwd[])
{
    char header[100];
    rewind(file);
    int bytesRead = fread(header, 1, strlen(CRYPT_signature), file);
    header[bytesRead] = '\0';

    CRYPT_reset_count();
    CRYPT_decrypter(header, bytesRead, pwd, 0);

    if (strcmp(header, CRYPT_signature) == 0)
       return 1;
    else return 0;
}


