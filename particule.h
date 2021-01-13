#ifndef PARTICULE_H
#define PARTICULE_H

#include <stdio.h>

#include "utilitaire.h"
#include "error.h"

enum etat_lecture {DEBUT, LECTURE, FIN, SORTIE};

// Traitement de la liste de particules

void traitement_liste_part(FILE * fichier, int nb_ligne, bool *p_errState);

// Dessin d'une particule

void part_dessine (void);

// Opérations sur la liste de particule

void afficher_liste_part (void);
void effacer_liste_part(void);
void write_save_file_part (FILE * fichier);
void part_delete(int num);

// Vérifications d'erreurs de particules dans les fichiers test

int missing_fin_liste_part(char tab[MAX_LINE+1]);
void check_fin_liste(char tab[MAX_LINE+1],int nb_ligne);
void part_error_collision(bool *p_errState);
C2D *part_collision_robot(C2D rob, double *p_dist, int *num);

// Opération utiles pour l'analyse de fichiers test

void lecture_nb(char tab[MAX_LINE+1], int *p_nb, int *etat, int nb_ligne, 
															bool *p_errState);
bool comp_chaine (char tab[MAX_LINE+1], char *fin_liste);
bool is_empty(char *tab);

// choix d'une particule cible

int search_particule_cible(S2D robot_position);
bool part_get_pos(int num, S2D *pos);
void part_libere_cible(int num);
void part_decomp_add(double x, double y, double r, double e);
void part_update();

bool part_left();
double calcul_decontamination();

#endif
