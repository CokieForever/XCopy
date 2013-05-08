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

#ifndef FONCTIONS_CRYPTAGE

#define FONCTIONS_CRYPTAGE

#include <stdlib.h>
#include <string.h>
#include "define.h"

extern unsigned char *CRYPT_tableauCode[256];
extern char CRYPT_signature[];
extern int CRYPT_previous;
#define CRYPT_CONTINUATE (-2)


int CRYPT_crypter(void *ptv, int size, char pwd[], int init);
int CRYPT_decrypter(void *ptv, int size, char pwd[], int init);
void CRYPT_init();
void CRYPT_copier_chaine(char chaine1[], char chaine2[], int tailleMaxChaine);
void CRYPT_reset_count();
int CRYPT_encoder_header(char pwd[], char out[], int tailleMax);
int CRYPT_tester_header(FILE *file, char pwd[]);


#endif
