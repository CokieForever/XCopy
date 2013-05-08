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

#include "fonctions_fichier.h"

void lire_caracteres(FILE *fichier, char chaine[], int nombreCaracteres)
{
     int i;
     for (i = 0 ; i < nombreCaracteres ; i++)
     {
         chaine[i] = fgetc(fichier);
     }
}


int lire_ligne(char nomFichier[], int numeroLigne, char chaine[], int tailleMaxChaine)
{
    FILE *fichier = NULL;
    if ((fichier = fopen(nomFichier, "r")) == NULL)
    {
        fprintf(stderr, "ERR lire_ligne %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichier);
        return 0;
    }

    int i;

    for (i = 1 ; i <= numeroLigne ; i++)
    {
        if (fgets(chaine, tailleMaxChaine, fichier) == NULL)
        {
           fprintf(stderr, "ERR lire_ligne %d : Le fichier %s contient moins de %d lignes.\n", __LINE__, nomFichier, numeroLigne);
           return -1;
        }
    }

    char *positionEntree = NULL;
    if ((positionEntree = strrchr(chaine, '\n')) != NULL)
       *positionEntree = '\0';

    while(fclose(fichier) == EOF);

    return 1;
}


int effacer_fichier(char nomFichier[])
{
    FILE *fichier = NULL;
    if ((fichier = fopen(nomFichier, "w")) == NULL)
    {
                 fprintf(stderr, "ERR effacer_fichier %d : Le fichier %s n'a pas pu être effacé.\n", __LINE__, nomFichier);
                 return 0;
    }

    while(fclose(fichier) == EOF);
    return 1;
}


int test_vide(char nomFichier[])
{
         FILE *fichier = NULL;
         int i;

         for (i = 0 ; (fichier = fopen(nomFichier, "r")) == NULL && i < 10 ; i++);

         if (fichier == NULL)
         {
                      fprintf(stderr, "ERR test_vide %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichier);
                      return -1;
         }

         if (fgetc(fichier) == EOF)
         {
            while(fclose(fichier) == EOF);
            return 0;
         }
         while(fclose(fichier) == EOF);
         return 1;
}


int compter_lignes(char nomFichier[])
{
    FILE *fichier = NULL;
    if ((fichier = fopen(nomFichier, "r")) == NULL)
    {
                 fprintf(stderr, "ERR compter_lignes %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichier);
                 return -1;
    }

    char chaine[TAILLE_MAX_NOM] = {0};
    int nombreLignes = 0;

    while (fgets(chaine, TAILLE_MAX_NOM, fichier) != NULL)
    {
          nombreLignes++;
    }

    while(fclose(fichier) == EOF);

    return nombreLignes;
}



int supprimer_ligne(char nomFichier[], int tailleMaxChaine, int numeroLigne)
{
    int nombreLignes = compter_lignes(nomFichier);
    if (numeroLigne > nombreLignes)
       numeroLigne = nombreLignes;
    if (numeroLigne < 0)
       numeroLigne = 0;

    char **tableauChaines = NULL;
    tableauChaines = malloc(sizeof(char*) * (1 + nombreLignes));

    int i;
    for(i = 1 ; i <= nombreLignes ; i++)
    {
        tableauChaines[i] = malloc(sizeof(char) * (1 + tailleMaxChaine));
    }

    FILE *fichier = NULL;
    if ((fichier = fopen(nomFichier, "r")) == NULL)
    {
        fprintf(errFile, "ERR supprimer_ligne %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichier);
        return 0;
    }

    for (i = 1 ; i <= nombreLignes ; i++)
    {
        fgets(tableauChaines[i], tailleMaxChaine, fichier);
    }

    while(fclose(fichier) == EOF);
    if ((fichier = fopen(nomFichier, "w+")) == NULL)
    {
        fprintf(errFile, "ERR supprimer_ligne %d : Impossible de rouvrir le fichier %s.\n", __LINE__, nomFichier);
        return 0;
    }

    for (i = 1 ; i <= nombreLignes ; i++)
    {
        if (i != numeroLigne)
           fprintf(fichier, "%s", tableauChaines[i]);
        free(tableauChaines[i]);
    }

    while(fclose(fichier) == EOF);
    free(tableauChaines);
    return 1;
}



int inserer_ligne(char nomFichier[], int tailleMaxChaine, int numeroLigne, char chaineAInserer[])
{
    int nombreLignes = compter_lignes(nomFichier) + 1;
    if (numeroLigne > nombreLignes)
       numeroLigne = nombreLignes;
    if (numeroLigne < 0)
       numeroLigne = 0;

    char **tableauChaines = NULL;
    tableauChaines = malloc(sizeof(char*) * (1 + nombreLignes));

    int i;
    for(i = 1 ; i <= nombreLignes ; i++)
    {
          tableauChaines[i] = malloc(sizeof(char) * (1 + tailleMaxChaine));
    }

    FILE *fichier = NULL;
    if ((fichier = fopen(nomFichier, "r+")) == NULL)
    {
                 fprintf(stderr, "ERR inserer_ligne %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichier);
                 return 0;
    }

    for (i = 1 ; i <= nombreLignes ; i++)
    {
        if (i != numeroLigne)
           fgets(tableauChaines[i], tailleMaxChaine, fichier);
    }

    sprintf(tableauChaines[numeroLigne], "%s\n", chaineAInserer);

    while(fclose(fichier) == EOF);
    if ((fichier = fopen(nomFichier, "w")) == NULL)
    {
                 fprintf(stderr, "ERR inserer_ligne %d : Impossible de rouvrir le fichier %s.\n", __LINE__, nomFichier);
                 return 0;
    }

    for (i = 1 ; i <= nombreLignes ; i++)
    {
        fprintf(fichier, "%s", tableauChaines[i]);
        free(tableauChaines[i]);
    }

    while(fclose(fichier) == EOF);
    free(tableauChaines);
    return 1;
}

