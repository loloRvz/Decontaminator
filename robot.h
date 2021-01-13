#ifndef ROBOT_H
#define ROBOT_H

#include <stdio.h>

#include "particule.h"

void rob_traitement_liste(FILE *fichier, int *nb_ligne, bool *p_errState);
int rob_dessine(void);
void rob_effacer_liste(void);
void rob_write_save_file (FILE * fichier);

void rob_error_collision(bool *p_errState);
void rob_check_collision_part(bool *p_errState);

void rob_update(void);
void rob_change_goal(int prev, int new_goal);
void rob_select(double x, double y);
void rob_selected_vtran(bool change);
void rob_selected_vrot(bool change);
double rob_get_vtran();
double rob_get_vrot();
void rob_deselect();

#endif
