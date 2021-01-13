#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#include "graphic.h"

#define COTES_CERCLE 50.

void graphic_circle(float cx, float cy, float r) 
{ 
	float theta;

	for(int i = 0; i < COTES_CERCLE; i++) 
	{ 
		theta = (2 * M_PI * (float)i) / COTES_CERCLE; 

		float x = r * cosf(theta);
		float y = r * sinf(theta);
		
		glVertex2f(x + cx, y + cy);
	} 	
	glEnd(); 
}

void graphic_line(float x1, float y1, float x2,float y2)
{ 
    glBegin (GL_LINES);
	glColor3f(1, 0, 0);
	
    glVertex2f (x1, y1);
    glVertex2f (x2, y2);

    glEnd ();
}

void graphic_square(double x1, double y1, double half_side)
{
    glBegin(GL_LINE_LOOP);
    glColor3f(0, 0, 0);
    
    glVertex2f(x1 + half_side, y1 + half_side);
    glVertex2f(x1 + half_side, y1 - half_side);
    glVertex2f(x1 - half_side, y1 - half_side);
    glVertex2f(x1 - half_side, y1 + half_side);

    glEnd();
}

