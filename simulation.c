#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "simulation.h"

void lecture_fichier(char *filename, char *mode)
{
	bool errState = false;
	int nb_ligne = 0;
	
	FILE * fichier = fopen(filename, "r");
	
	if (fichier)
	{
		rob_traitement_liste(fichier, &nb_ligne, &errState);
		traitement_liste_part(fichier, nb_ligne, &errState);
		rob_error_collision(&errState);
		part_error_collision(&errState);
	}
	else
	{
		errState = true;
		error_file_missing(filename);
	}
	
	if(!errState && !strcmp(mode, "Error"))
	{
		error_no_error_in_this_file();
	}
	else if (errState)
	{
		rob_effacer_liste();
		effacer_liste_part();
	}
	
	if(!strcmp(mode,"Error"))
	{
		exit(errState);
	}
	
	if(fichier)
	{
		fclose(fichier);		
	}
}
