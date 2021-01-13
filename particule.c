#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GL/glut.h>

#include "particule.h"

#define PART_DATA_NUM  4
#define FIRST_PART 1
#define PERCENT_FACTOR 100

typedef struct particule PART;
struct particule
{
	int num;
	C2D cercle;
	PART * suivant;
	double energie;
	bool deja_ciblee;
};

// Global varibale for the module

static PART * tete_liste_part = NULL;
static int nb_part;
static double energie_tot = 0;

// Function declaration

PART * liste_ajouter_part (PART ** p_tete);
void check_part_value(PART part, bool *p_errState);

void lecture_info_part (char tab[MAX_LINE+1], int *etat, int nb_ligne, 
														 int * p_nb_part_lu, 
														 bool *p_errState);

void traitement_liste_part(FILE * fichier, int nb_ligne, bool *p_errState)
{
	char tab[MAX_LINE+1], ligne[MAX_LINE+1];
	int etat = DEBUT, nb_part_lu = 0;
	char *fin_liste = "FIN_LISTE";
	energie_tot = 0;
	
	while (etat != SORTIE && fgets(ligne, MAX_LINE+1, fichier))
	{
		nb_ligne++;
		memset(tab, 0, MAX_LINE+1);
		vider_commentaire(ligne, tab);
		
		switch (etat)
		{
			case DEBUT : 
				lecture_nb(tab, &nb_part, &etat, nb_ligne, p_errState);		
				break;
						 
			case LECTURE : 
				if(comp_chaine(tab, fin_liste) && !(*p_errState))
				{
					error_fin_liste_particules(nb_ligne);
					*p_errState = true;
				}
				lecture_info_part(tab, &etat, nb_ligne, &nb_part_lu, p_errState);
				break; 
						   
			case FIN : 
				if (comp_chaine(tab, fin_liste))
				{
					etat = SORTIE;
					break;
				}
				if(!is_empty(tab) && !(*p_errState))
				{
					error_missing_fin_liste_particules(nb_ligne);
					*p_errState = true;
				}
				break;
		}
	}
}

void lecture_info_part (char tab[MAX_LINE+1], int *etat, int nb_ligne, 
														 int * p_nb_part_lu, 
														 bool *p_errState)
{
	PART struct_part;
	static PART *p_nouveau;
	
	while (sscanf(tab, "%lf %lf %lf %lf",&struct_part.energie, 
										 &struct_part.cercle.rayon, 
										 &struct_part.cercle.centre.x, 
										 &struct_part.cercle.centre.y)
										 == PART_DATA_NUM)
	{
		for(int i=0; i<PART_DATA_NUM; i++)
		{
			strtod(tab,&tab);
		}
		(*p_nb_part_lu)++;
		
		if(*p_nb_part_lu == FIRST_PART)
		{
			p_nouveau = liste_ajouter_part (&tete_liste_part);
		}
		else
		{
			p_nouveau = liste_ajouter_part (&p_nouveau->suivant);
		}
		p_nouveau->energie = struct_part.energie;
		p_nouveau->cercle= struct_part.cercle;
		p_nouveau->num = *p_nb_part_lu;
		p_nouveau->deja_ciblee = false;
		
		check_part_value(struct_part, p_errState);
		
		if (*p_nb_part_lu == nb_part)
		{
			*etat = FIN;							 
		}
		energie_tot += p_nouveau->energie;
	}	
	if(!is_empty(tab) && !(*p_errState))
	{
		error_useless_char(nb_ligne);
		*p_errState = true;
	}
}

void part_dessine (void)
{
	PART * courant = tete_liste_part;
	PART * effacer;
	
	double coord_x, coord_y, rayon;
	
	if(courant)
	{
		while(courant) 
		{
			coord_x = courant->cercle.centre.x;
			coord_y = courant->cercle.centre.y;
			rayon = courant->cercle.rayon;
			
			glBegin(GL_POLYGON); 
			glColor3f(0.1, 0.5, 0.5);
			graphic_circle(coord_x, coord_y, rayon);
			
			effacer = courant->suivant;
			courant = effacer;
		}
	}
}

void afficher_liste_part (void)
{
	PART * courant = tete_liste_part;
	printf("%d\n", nb_part);
	if (courant)
	{
		while(courant)
		{
			printf("%lf %lf %lf %lf\n\n", courant->energie, 
										  courant->cercle.rayon, 
										  courant->cercle.centre.x, 
										  courant->cercle.centre.y);
			
			courant = courant->suivant;
		}
	}
}

PART * liste_ajouter_part (PART ** p_tete)
{
	PART * element = (PART *)malloc(sizeof(PART));
	if (element == NULL)
	{
		printf("Erreur dans %s\n", __func__);
		exit(EXIT_FAILURE);
	}
	element->suivant = * p_tete;
	* p_tete = element;
	return element;
}

void effacer_liste_part(void)
{
   PART * courant = tete_liste_part;
   PART * suivant;
 
   while (courant) 
   {
       suivant = courant->suivant;
       free(courant);
       courant = suivant;
   }
   
	tete_liste_part = NULL;
}

void part_delete(int num)
{
	if(tete_liste_part)
	{
		PART *current = tete_liste_part, *prev;
		if(current->num == num)
		{
			tete_liste_part = current->suivant;
			free(current);
		}
		else
		{
			prev = current;
			current = current-> suivant;
			while(current)
			{
				if(current->num == num)
				{
					prev->suivant = current->suivant;
					free(current); break;
				}
				prev = current;
				current = current->suivant;
			}
		}
	}
}

void check_part_value(PART part, bool *p_errState)
{
	if((util_point_dehors(part.cercle.centre, DMAX) ||
	   part.cercle.rayon < R_PARTICULE_MIN || 
	   part.cercle.rayon > R_PARTICULE_MAX ||
	   part.energie < 0 ||
	   part.energie > E_PARTICULE_MAX) &&
	   !(*p_errState))
	{
		error_invalid_particule_value(part.energie, part.cercle.rayon,
													part.cercle.centre.x, 
													part.cercle.centre.y);
		*p_errState = true;
	}
}

void lecture_nb(char tab[MAX_LINE+1], int *p_nb, int *etat, int nb_ligne, 
															bool *p_errState)
{	
	int nombre = sscanf(tab,"%d", p_nb);
	if (nombre == 1)
	{
		strtod(tab, &tab);
		if(!is_empty(tab) && !(*p_errState))
		{
			error_useless_char(nb_ligne);
			*p_errState = true;
		}
		*etat = LECTURE; 
	}
	else if(nombre != EOF && !(*p_errState))
	{
		error_useless_char(nb_ligne);
		*p_errState = true;
	}
}

bool comp_chaine (char tab[MAX_LINE+1], char *fin_liste)
{
	char t1[MAX_LINE+1],t2[MAX_LINE+1];
	
	if(sscanf(tab,"%s %s", t1, t2) == 1)
	{
		return !strcmp(t1,fin_liste);
	}
	else
	{
		return false;
	}
}

bool is_empty(char *tab)
{
	while(*tab != '\0')
	{
		if(!isspace(*tab))
		{
			return false;
		}
		tab++;
	}
	return true;
}

void write_save_file_part (FILE * fichier)
{
	PART * courant_part = tete_liste_part;
	
	fprintf(fichier, "%d\n", nb_part);
	while(courant_part)
	{
		fprintf(fichier, "%lf %lf %lf %lf\n", courant_part->energie, 
											  courant_part->cercle.rayon, 
											  courant_part->cercle.centre.x, 
											  courant_part->cercle.centre.y);
		courant_part = courant_part->suivant;
	}
	fprintf(fichier, "FIN_LISTE\n");
}


void part_error_collision(bool *p_errState)
{
	PART *current = tete_liste_part, *compare;
	double t;
	
	while(current && !(*p_errState))
	{
		compare = current->suivant;
		while(compare && !(*p_errState))
		{
			if(util_collision_cercle(current->cercle, compare->cercle, &t))
			{
				error_collision(PARTICULE_PARTICULE, current->num, compare->num);
				*p_errState = true;
			}
			compare = compare->suivant;
		}
		current = current->suivant;
	}
}

C2D *part_collision_robot(C2D rob, double *p_dist, int *num)
{
	PART *current = tete_liste_part;
	while(current)
	{
		if(util_collision_cercle(rob,current->cercle,p_dist))
		{
			*num = current->num;
			return &current->cercle;
		}
		current = current->suivant;
	}
	return NULL;
}


int search_particule_cible(S2D robot_position)
{
	PART * courant = tete_liste_part;
	PART * new_cible = NULL; 
	double max_score = 0, score;
	int part_cible_num = 0;
	bool free_part = false;
	
	while(courant)
	{	
		score = courant->cercle.rayon / 
				util_distance(robot_position, courant->cercle.centre);
		if(!courant->deja_ciblee && score > max_score)
		{
			new_cible = courant;
			part_cible_num = courant->num;
			max_score = score;
			free_part = true;
		}
		courant = courant->suivant;
	}
	if(!free_part && tete_liste_part)
	{
		new_cible = tete_liste_part;
		part_cible_num = tete_liste_part->num;
	}
	if(new_cible)
	{
		new_cible->deja_ciblee = true;
	}
	return part_cible_num;
}
			
bool part_get_pos(int num, S2D *pos)
{
	PART *courant = tete_liste_part;
	while(courant)
	{
		if(courant->num == num)
		{
			*pos = courant->cercle.centre;
			return true;
		}
		courant = courant->suivant;
	}
	return false;
}	

void part_libere_cible(int num)
{
	PART *current = tete_liste_part;
	while(current)
	{
		if(current->num == num)
		{
			current->deja_ciblee = false;
		}
		current = current->suivant;
	}
}	
		

void part_update()
{
	PART *current = tete_liste_part;
	while(current)
	{
		float new_rad = current->cercle.rayon * R_PARTICULE_FACTOR;
		
		if(new_rad >= R_PARTICULE_MIN && 
		   (double)rand()/(double)RAND_MAX <= DECOMPOSITION_RATE)
		{
			part_decomp_add(current->cercle.centre.x + new_rad, 
							current->cercle.centre.y + new_rad,
							new_rad,
							current->energie * E_PARTICULE_FACTOR);
			part_decomp_add(current->cercle.centre.x - new_rad, 
							current->cercle.centre.y + new_rad,
							new_rad,
							current->energie * E_PARTICULE_FACTOR);
			part_decomp_add(current->cercle.centre.x + new_rad, 
							current->cercle.centre.y - new_rad,
							new_rad,
							current->energie * E_PARTICULE_FACTOR);
			part_decomp_add(current->cercle.centre.x - new_rad, 
							current->cercle.centre.y - new_rad,
							new_rad,
							current->energie * E_PARTICULE_FACTOR);
			part_delete(current->num);
		}
		current = current->suivant;
	}
}

void part_decomp_add(double x, double y, double r, double e)
{
	PART *new = (PART *)malloc(sizeof(PART));
	new->cercle = (C2D){(S2D){x,y},r};
	new->energie = e;
	
	if(!tete_liste_part)
	{
		new->num = FIRST_PART;
		tete_liste_part = new;
	}
	else
	{
		PART *current=tete_liste_part->suivant, *prev = tete_liste_part;
		while(current)
		{
			prev = current;
			current = current->suivant;
		}
		new->num = prev->num + 1;
		new->suivant = prev->suivant;
		prev->suivant = new;
	}
}

bool part_left()
{
	return tete_liste_part;
}
	
double calcul_decontamination()
{
	PART * courant = tete_liste_part;
	double energie_courant = energie_tot;
	while(courant)
	{
		energie_courant -= courant->energie;
		courant = courant->suivant;
	}
	
	return (double) PERCENT_FACTOR*energie_courant/energie_tot;
}
