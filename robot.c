#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <GL/glut.h>

#include "robot.h"

#define ROB_DATA_NUM 3
#define FIRST_ROBOT 1

typedef struct select SELECT;
struct select
{
	double vrot;
	double vtran;
};

typedef struct
{
	int num;
	S2D position;
	double ecart_angle;
}CIBLE;

typedef struct robot ROBOT;
struct robot
{
	int num;
	S2D position;
	double angle;
	bool selected;
	ROBOT * suivant;
	SELECT vitesse;
	CIBLE cible;
};

// Global variable for the module

static ROBOT * tete_liste_robot = NULL;
static int nb_robot;
static ROBOT * selected_robot = NULL;

// Function definition

ROBOT * liste_ajouter_robot (ROBOT ** p_tete);
ROBOT *robot_collision_robot(S2D rob, int num, double *p_dist);
void rob_translation(ROBOT *current, bool *rob_collision);
void rob_reposition(ROBOT *current, C2D obstacle, S2D	*newPos, double lb);

void lecture_info_robot (char tab[MAX_LINE+1], int *etat, int nb_ligne, 
														  int *p_nb_robot_lu, 
														  bool *p_errState);

void lecture_info_robot (char tab[MAX_LINE+1], int *etat, int nb_ligne, 
														  int *p_nb_robot_lu, 
														  bool *p_errState)
{
	//buffer struct for sscanf
	ROBOT struct_robot;
	static ROBOT  *p_nouveau;
	
	//read 3 data values and save in buffer struct
	while (sscanf(tab," %lf %lf %lf", &struct_robot.position.x,
									  &struct_robot.position.y,              
									  &struct_robot.angle) == ROB_DATA_NUM)
	{
		//advance "pointer" to the end of the third value
		for(int i=0; i<ROB_DATA_NUM; i++)
		{
			strtod(tab, &tab);
		}
		(*p_nb_robot_lu)++;
		
		if(*p_nb_robot_lu == FIRST_ROBOT)
		{
			p_nouveau = liste_ajouter_robot (&tete_liste_robot);
		}
		else
		{
			p_nouveau = liste_ajouter_robot (&p_nouveau->suivant);
		}
		p_nouveau->position = struct_robot.position;
		p_nouveau->angle = struct_robot.angle;
		p_nouveau->num = *p_nb_robot_lu;
		p_nouveau->cible.num = 0;
		p_nouveau->selected = false;
		
		if (util_alpha_dehors(struct_robot.angle) && !(*p_errState))
		{
			error_invalid_robot_angle(struct_robot.angle);
			*p_errState = true;
		}
		
		if (*p_nb_robot_lu == nb_robot)
		{
			*etat = FIN;
			break;
		}
	}
	/*If the tab is empty --> data values are not in groups of 3 
	 * --> improper formatting*/
	if(!is_empty(tab) && !(*p_errState))
	{
		error_useless_char(nb_ligne);
		*p_errState = true;
	}
}

void rob_traitement_liste(FILE *fichier, int *nb_ligne, bool *p_errState)
{
	char tab[MAX_LINE+1], ligne[MAX_LINE+1];
	int nb_robot_lu = 0;
	int etat = DEBUT;
	char *fin_liste = "FIN_LISTE";
	while (etat != SORTIE && fgets(ligne, MAX_LINE+1, fichier))
	{
		(*nb_ligne)++;
		memset(tab, 0, MAX_LINE+1);
		vider_commentaire(ligne, tab);
		
		switch (etat)
		{
			case DEBUT:	
				lecture_nb (tab, &nb_robot, &etat, *nb_ligne, p_errState);
				break;
						 
			case LECTURE: 
				//fin_liste should only appear when in etat == FIN
				if(comp_chaine(tab, fin_liste) && !(*p_errState))
				{
					error_fin_liste_robots(*nb_ligne);
					*p_errState = true;
				}
				//read and save all values of one line
				lecture_info_robot(tab, &etat, *nb_ligne, &nb_robot_lu, p_errState);
				break; 
				
			case FIN : 
				if (comp_chaine(tab, fin_liste))
				{
					etat = SORTIE;
					break;
				}
				if(!is_empty(tab) && !(*p_errState))
				{
					error_missing_fin_liste_robots(*nb_ligne);
					*p_errState = true;
				}
				break;	  
		}
	};
}

int rob_dessine(void)
{
	ROBOT * courant = tete_liste_robot;
	ROBOT * effacer;
	int nb_robot = 0;
	
	double coord_x, coord_y, angle;
	
	if(courant)
	{
		while(courant) 
		{
			coord_x = courant->position.x;
			coord_y = courant->position.y;
			angle = courant->angle;
			
			glBegin(GL_LINE_LOOP); 
			glColor3f(0, 0, 0);
			if(courant->selected)
			{
				glColor3f(1, 0, 0);
			}
			graphic_circle(coord_x, coord_y, R_ROBOT);
			graphic_line(coord_x, coord_y, coord_x + R_ROBOT*cos(angle), 
										   coord_y + R_ROBOT*sin(angle)); 
			
			effacer = courant->suivant;
			courant = effacer;
			
			nb_robot++;
		}
	}	
	return nb_robot;
}

void rob_effacer_liste(void)
{
   ROBOT * courant = tete_liste_robot;
   ROBOT * suivant;
 
   while (courant != NULL) 
   {
       suivant = courant->suivant;
       free(courant);
       courant = suivant;
   }
   
   tete_liste_robot = NULL;
}

ROBOT * liste_ajouter_robot (ROBOT ** p_tete)
{
	ROBOT * element = (ROBOT *)malloc(sizeof(ROBOT));
	if (element == NULL)
	{
		printf("Erreur dans %s\n", __func__);
		exit(EXIT_FAILURE);
	}
	element->suivant = * p_tete;
	* p_tete = element;
	return element;
}

void rob_write_save_file(FILE * fichier)
{
	ROBOT * courant_robot = tete_liste_robot;
	
	fprintf(fichier, "%d\n", nb_robot);
	while(courant_robot)
	{
		fprintf(fichier, "%lf %lf %lf\n", courant_robot->position.x, 
										  courant_robot->position.y, 
										  courant_robot->angle);
		courant_robot = courant_robot->suivant;
	}
	fprintf(fichier, "FIN_LISTE\n\n");
}

void rob_error_collision(bool *p_errState)
{
	ROBOT *current = tete_liste_robot, *compare;
	double t;
	int num;
	
	while(current && !(*p_errState))
	{
		compare = current->suivant;
		while(compare && !(*p_errState))
		{
			if(util_collision_cercle((C2D){current->position,R_ROBOT},
									 (C2D){compare->position,R_ROBOT}, &t))
			{
				error_collision(ROBOT_ROBOT, current->num, compare->num);
				*p_errState = true;
			}
			compare = compare->suivant;
		}
		if(part_collision_robot((C2D){current->position,R_ROBOT}, &t, &num) && 
		   !(!p_errState))
		{
			error_collision(ROBOT_PARTICULE, current->num, num);
			*p_errState = true;
		}
		current = current->suivant;
	}
}

ROBOT *robot_collision_robot(S2D rob, int num, double *p_dist)
{
	ROBOT *current = tete_liste_robot;
	while(current)
	{
		if((current->num != num) && util_collision_cercle((C2D){rob,R_ROBOT},
														  (C2D){current->position,
															    R_ROBOT},
														  p_dist))
		{
			return current;
		}
		current = current->suivant;
	}
	return NULL;
}

void rob_update()
{
	float vrot;
	ROBOT *current = tete_liste_robot;
	while (current)
	{
		if(!current->cible.num || !part_get_pos(current->cible.num, 
												&current->cible.position))
		{
			current->cible.num = search_particule_cible(current->position);
			part_get_pos(current->cible.num, &current->cible.position);
		}
		
		//TRANSLATION
		bool rob_collision = false;
		if(current->cible.num)
		{
			rob_translation(current, &rob_collision);	
		}
		
		//ROTATION
		if (!current->selected && current->cible.num && 
			util_ecart_angle(current->position, current->angle, 
							 current->cible.position, &current->cible.ecart_angle))
		{
			vrot = current->cible.ecart_angle/DELTA_T;
			if(fabs(vrot) > VROT_MAX)
			{
				vrot *= VROT_MAX / fabs(vrot);
			}
			if(rob_collision)
			{
				vrot = VROT_MAX;
			}
			current->angle += vrot * DELTA_T;
		}
		
		if(current->selected)
		{
			current->angle += current->vitesse.vrot * DELTA_T;
		}
		
		current = current->suivant;
	}
}

void rob_translation(ROBOT *current, bool *rob_collision)
{
	double lb;
	int num;
	
	S2D newPos = {current->position.x + VTRAN_MAX*DELTA_T*cos(current->angle),
				  current->position.y + VTRAN_MAX*DELTA_T*sin(current->angle)};
				  
	if(current->selected)
	{
		newPos.x = current->position.x + 
				   current->vitesse.vtran*DELTA_T*cos(current->angle);
		newPos.y = current->position.y + 
				   current->vitesse.vtran*DELTA_T*sin(current->angle);
	}
			
	C2D *part = part_collision_robot((C2D){newPos,R_ROBOT},&lb,&num);
	while(part)
	{
		if(current->cible.num != num && !current->selected)
		{
			part_libere_cible(current->cible.num);
			current->cible.num = num;
			part_get_pos(current->cible.num, &current->cible.position);
		}
		rob_reposition(current, *part, &newPos, lb);
		if(util_alignement(current->position,current->angle, part->centre))
		{
			part_delete(num);
			current->cible.num = 0;
		}
		part = part_collision_robot((C2D){newPos,R_ROBOT},&lb,&num);
	}
	
	ROBOT *rob = robot_collision_robot(newPos, current->num, &lb);
	while(rob)
	{
		*rob_collision = true;
		rob_reposition(current, (C2D){rob->position,R_ROBOT}, &newPos, lb);
		rob = robot_collision_robot(newPos, current->num, &lb);
	}
	
	if(!*rob_collision && fabs(current->cible.ecart_angle) > M_PI/4)
	{
		newPos = current->position;
	}
	
	current->position = newPos;	
}

void rob_reposition(ROBOT *current, C2D obstacle, S2D	*newPos, double lb)
{
	double la_new;
	if(util_inner_triangle(util_distance(current->position,*newPos),
						   lb,
						   util_distance(current->position, obstacle.centre),
						   R_ROBOT+obstacle.rayon,
						   &la_new))
	{
		*newPos = (S2D){current->position.x + la_new * cos(current->angle),
					   current->position.y + la_new * sin(current->angle)};
	}
	else
	{
		*newPos = current->position;
	}
}

void rob_select(double x, double y)
{
	ROBOT* courant = tete_liste_robot;
	double dist;
	unsigned int compt = 0;
	S2D click_pos = {x, y};
	
	while(courant)
	{
		dist = util_distance(click_pos, courant->position);
		if (dist < R_ROBOT)
		{
			if(selected_robot)
			{
				selected_robot->selected = false;
				selected_robot->vitesse.vrot = 0;
				selected_robot->vitesse.vtran = 0;
				selected_robot = NULL;
			}
			selected_robot = courant;
			selected_robot->selected = true;
			selected_robot->vitesse.vrot = 0;
			selected_robot->vitesse.vtran = 0;
			compt++;
		}
		courant = courant->suivant;
	}
	
	if (selected_robot && !compt)
	{
		selected_robot->selected = false;
		selected_robot->vitesse.vrot = 0;
		selected_robot->vitesse.vtran = 0;
		selected_robot = NULL;
	}
	
	if(selected_robot)
	{
		printf("robot selectionne : %d\n", selected_robot->num);
	}
}

void rob_selected_vtran(bool change)
{
	if(selected_robot)
	{
		if(change && selected_robot->vitesse.vtran < VTRAN_MAX)
		{
			selected_robot->vitesse.vtran += DELTA_VTRAN;
		}
		else if (!change && selected_robot->vitesse.vtran> -VTRAN_MAX)
		{
			selected_robot->vitesse.vtran -= DELTA_VTRAN;
		}
	}
}

void rob_selected_vrot(bool change)
{
	if(selected_robot)
	{
		if(change && selected_robot->vitesse.vrot < VROT_MAX)
		{
			selected_robot->vitesse.vrot += DELTA_VROT;
		}
		else if(!change && selected_robot->vitesse.vrot > -VROT_MAX)
		{
			selected_robot->vitesse.vrot -= DELTA_VROT;
		}
	}
}

double rob_get_vtran()
{
	if(selected_robot)
	{
		return selected_robot->vitesse.vtran;
	}
	return 0;	
}

double rob_get_vrot()
{
	if(selected_robot)
	{
		return selected_robot->vitesse.vrot;
	}
	return 0;
}

void rob_deselect()
{
	if(selected_robot)
	{
		selected_robot->selected = false;
		selected_robot->vitesse.vrot = 0;
		selected_robot->vitesse.vtran = 0;
		selected_robot = NULL;
	}
}	
