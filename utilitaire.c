#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "utilitaire.h"

double util_distance(S2D a, S2D b)
{
	double dx, dy;
	
	dx = b.x - a.x;
	dy = b.y - a.y;
	return sqrt(dx*dx + dy*dy);
}

double util_angle(S2D a, S2D b)
{
	double dy, dx;
	
	dx = b.x - a.x;
	dy = b.y - a.y;
	return atan2(dy,dx);
}

void util_range_angle(double *p_angle)
{
	double t = fmod(*p_angle,2*M_PI);
	
	if(t > M_PI)
	{
		t -= 2*M_PI;
	}
	if(t <= -M_PI)
	{
		t += 2*M_PI;
	}
	*p_angle = t;
}

bool util_point_dehors(S2D a, double max)
{
	return (fabs(a.x) > max || fabs(a.y) > max);
}

bool util_alpha_dehors(double alpha)
{
	return (fabs(alpha) > M_PI);
}

bool util_point_dans_cercle(S2D a, C2D c)
{
	return (util_distance(a, c.centre) < c.rayon - EPSIL_ZERO);
}

bool util_collision_cercle(C2D a, C2D b, double *p_dist)
{
	*p_dist = util_distance(a.centre, b.centre);
	return (*p_dist < a.rayon + b.rayon - EPSIL_ZERO);
}

S2D util_deplacement(S2D p, double alpha, double dist)
{
	p.x += dist*cos(alpha);
	p.y += dist*sin(alpha);
	return p;
}

bool util_ecart_angle(S2D a, double alpha, S2D b, double *p_ecart_angle)
{
	if(util_distance(a,b) > EPSIL_ZERO)
	{
		*p_ecart_angle = util_angle(a,b) - alpha;
		util_range_angle(p_ecart_angle);
		return true;
	}
	else
	{
		return false;
	}
}

bool util_alignement(S2D a , double alpha , S2D b)
{
	double ecart_angle;
	
	return (util_ecart_angle(a, alpha, b, &ecart_angle) && 
			fabs(ecart_angle) < EPSIL_ALIGNEMENT);
}

bool util_inner_triangle(double la, double lb, double lc, double lb_new, double *p_la_new)
{
	if (fmin(la,lc) > EPSIL_ZERO && lb >= 0 && lb_new >= lb && lb_new <= lc)
	{
		double B = -(lc*lc + la*la - lb*lb)/la;
		double C = lc*lc - lb_new*lb_new;
		double D = B*B - 4*C;
		double lambda_1, lambda_2;
		
		if (D > 0)
		{
			lambda_1 = (-B-sqrt(D))/2;
			lambda_2 = (-B+sqrt(D))/2;
			*p_la_new = fmin(lambda_1, lambda_2);
			return true;
		}
		else if (D == 0)
		{
			*p_la_new = -B/2;
			return true;
		}
	}
	return false;
}
		
// Fonctions supplémentaires:

void vider_commentaire (char ligne[MAX_LINE+1], char tab[MAX_LINE+1])
{	
	for(int i = 0; i < MAX_LINE+1 && ligne[i] != '#'; i++)
	{
		tab[i] = ligne[i];
	}
}		
		


