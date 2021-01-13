#include   <stdlib.h>
#include   <string.h>
#include <math.h>
#include   <GL/glu.h>
#include   <GL/glui.h>
#include   <GL/glut.h>

enum terminal{EXE, MODE, TEST_FILE};
enum control{CTRL_OPEN, CTRL_SAVE, CTRL_STEP, CTRL_START};
enum domain_limits{X_MIN = -20, X_MAX = 20, Y_MIN = -20, Y_MAX = 20};

#define NUM_PARAM 3
#define WINDOW_SIZE 600
#define WINDOW_POS 200
#define LINE_WIDTH 2
#define ESC_KEY 27
#define STRING_LENGHT 20

extern "C" 
{
	#include "simulation.h"
	#include "graphic.h"
} 

namespace 
{ 
	int main_window, width, height, start_variable = false;
	unsigned int nb_step = 0;
	double x_min, x_max, y_min, y_max, taux_decontamination = 0;
	
	GLUI_EditText *file_open = NULL, *file_save = NULL;
	GLUI_Button *start_button = NULL;
	GLUI_StaticText *turn = NULL, *rate = NULL, *vtran = NULL, *vrot = NULL;
	GLUI_Checkbox *record = NULL;
	GLUI_RadioButton *manual = NULL, *automatic = NULL;
	
	FILE * sim_record = NULL;
}

void idle_cb();
void display_cb();
void reshape_cb(int x, int y);
void key_cb(unsigned char key, int x, int y);
void mouse_cb(int button, int button_state, int x, int y);
void turn_update();
void rate_update();
void vitesses_update();
void record_update();
void arrow_cb(int key, int x, int y);

void create_user_interface();
void reinit_window();

int main(int argc, char **argv)
{	
	if(argc == NUM_PARAM && (!strcmp(argv[MODE],"Error")||!strcmp(argv[MODE],"Draw")))
	{
		lecture_fichier(argv[TEST_FILE], argv[MODE]);
	}
	else if(argc > NUM_PARAM || (argv[MODE]?strcmp(argv[MODE],"Draw"):false))
	{
		printf("Invalid use of this program!\n"
			   "Syntax: ./demo.x [Error|Draw, name_file]\n");
		exit(EXIT_FAILURE);
	}
	srand(time(NULL));
	glutInit(&argc, argv);	
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(WINDOW_POS,WINDOW_POS);
	glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);

	main_window = glutCreateWindow("Decontaminator");
	glutDisplayFunc(display_cb);
	glutReshapeFunc(reshape_cb);
	
	
	glLoadIdentity();
	glOrtho(-DMAX, DMAX, -DMAX, DMAX, -1, 1);
	
	create_user_interface();
	if (argc == NUM_PARAM)
	{
		file_open->set_text(argv[TEST_FILE]);
	}
	glutKeyboardFunc(key_cb);
	glutMouseFunc(mouse_cb);
	glutSpecialFunc(arrow_cb);
	GLUI_Master.set_glutIdleFunc(idle_cb); 
	glutMainLoop();
}


void idle_cb()
{
	if (glutGetWindow() != main_window)
	{
		glutSetWindow(main_window);
	}
	if(!part_left())
	{
		start_variable = false;
		start_button->set_name("Start");
	}
	
	if(start_variable)
	{
		rob_update();
		part_update();
		record_update();
		nb_step++; 
		turn_update();
		rate_update();		
		vitesses_update();
	}
	
	if(automatic->get_int_val())
	{
		rob_deselect();
	}
	
	glutPostRedisplay();
}

void display_cb()
{	
	glClearColor(1, 1, 1, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glOrtho(x_min, x_max, y_min, y_max, -1, 1);
	
	glLineWidth(LINE_WIDTH);
	graphic_square(0, 0, DMAX);
	
	rob_dessine();
	part_dessine();
	
	glutSwapBuffers();
}

void reshape_cb(int x, int y)
{
	glViewport(0, 0, x, y);
	width = x;
	height = y;
	
	double aspect_ratio = (double) width / height;
	if (aspect_ratio <= 1.)
	{
		x_min = X_MIN; 
		x_max = X_MAX;
		y_min = Y_MIN/aspect_ratio;
		y_max = Y_MAX/aspect_ratio;
	}
	else
	{
		x_min = X_MIN*aspect_ratio;
		x_max = X_MAX*aspect_ratio;
		y_min = Y_MIN ; 
		y_max = Y_MAX ;
	}
}

void control_cb(int id)
{
	switch(id)
	{
		case CTRL_OPEN : 
			start_variable = false;
			const char * text;
			text = file_open->get_text(); 
			rob_effacer_liste();
			effacer_liste_part();
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			lecture_fichier((char *)text, (char *)"Draw");
			nb_step = 0;
			turn->set_text("Turn: 0");
			rate->set_text("Rate: 0.000");
			vtran->set_text("Translation: 0.000");
			vrot->set_text("Rotation: 0.000");
			reinit_window(); 
			break;	
				     
		case CTRL_SAVE : 
			const char * file;
			file = file_save->get_text();   
			FILE * fichier;
			fichier = fopen(file, "a");
			rob_write_save_file(fichier);
			write_save_file_part(fichier);
			fclose(fichier);
			break;
				         
		case CTRL_STEP : 
			rob_update(); 
			part_update();
			glutPostRedisplay();
			if(part_left())
			{
				nb_step++;
				turn_update();
				rate_update();
			}
			record_update();
			vitesses_update();
			break;
			
		case CTRL_START : 
			if (part_left())
			{
				if(start_variable && record->get_int_val())
				{
					record->set_int_val(0);
				}
				start_variable = !start_variable;
				start_button->set_name(start_variable?"Stop":"Start");
			}
			break;
	}
}

void create_user_interface()
{
	GLUI *glui = GLUI_Master.create_glui("Interface utilisateur",0,
										 WINDOW_POS+WINDOW_SIZE,WINDOW_POS);
	
	GLUI_Panel * opening = glui->add_panel("Opening");
	file_open = glui->add_edittext_to_panel(opening, "File name:");
	glui->add_button_to_panel(opening, "Open", CTRL_OPEN, control_cb);
	
	GLUI_Panel * saving = glui->add_panel("Saving");
	file_save = glui->add_edittext_to_panel(saving, "File name:");
	glui->add_button_to_panel(saving, "Save", CTRL_SAVE, control_cb);
	file_save->set_text("save.txt");
	
	glui->add_column(true);
	
	GLUI_Panel *simulation = glui->add_panel("Simulation");
	start_button = glui->add_button_to_panel(simulation, "Start", CTRL_START, 
											 control_cb);
	glui->add_button_to_panel(simulation, "Step", CTRL_STEP, control_cb);
	
	GLUI_Panel * recording = glui->add_panel("Recording");
	record = glui->add_checkbox_to_panel(recording, "Record");
	rate = glui->add_statictext_to_panel(recording, "Rate: 0.000");
	turn = glui->add_statictext_to_panel(recording, "Turn: 0");
	
	glui->add_column(true);
		
	GLUI_Panel * control_mode = glui->add_panel("Control mode");
	GLUI_RadioGroup * controle = glui->add_radiogroup_to_panel(control_mode);
	automatic = glui->add_radiobutton_to_group(controle, "Automatic");
	manual = glui->add_radiobutton_to_group(controle, "Manual");
	
	GLUI_Panel * robot_control = glui->add_panel("Robot control");
	vtran = glui->add_statictext_to_panel(robot_control, "Translation: 0.000");
	vrot = glui->add_statictext_to_panel(robot_control, "Rotation: 0.000");
		
	glui->add_button("Exit", EXIT_SUCCESS, exit);
	
	glui->set_main_gfx_window(main_window);
}

void key_cb(unsigned char key, int x, int y)
{
	switch(key)
	{
		case ESC_KEY: 
			exit(EXIT_SUCCESS); break;
	}
}

void arrow_cb(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
			rob_selected_vtran(true); break;
		case GLUT_KEY_DOWN:
			rob_selected_vtran(false); break;
		case GLUT_KEY_LEFT:
			rob_selected_vrot(true); break;
		case GLUT_KEY_RIGHT:
			rob_selected_vrot(false); break;
	}
	vitesses_update();
}



void reinit_window()
{	
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);	
	glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);	
	glLoadIdentity();
	glOrtho(-DMAX, DMAX, -DMAX, DMAX, -1, 1);
}

void turn_update()
{
	char str_turn[STRING_LENGHT];
	snprintf(str_turn, STRING_LENGHT, "Turn: %d", nb_step);
	turn->set_text(str_turn);
}

void vitesses_update()
{
	char str[STRING_LENGHT];
	snprintf(str, STRING_LENGHT, "Translation: %.3lf", rob_get_vtran());
	vtran->set_text(str);
	
	snprintf(str, STRING_LENGHT, "Rotation: %.3lf", rob_get_vrot());
	vrot->set_text(str);
}
	
void rate_update()
{
	taux_decontamination = calcul_decontamination();
	char str[STRING_LENGHT];
	snprintf(str, STRING_LENGHT, "Rate: %.2lf", taux_decontamination);
	rate->set_text(str);
}

void mouse_cb(int button, int state, int x, int y)
{
	if(state == GLUT_DOWN && button == GLUT_LEFT_BUTTON && manual->get_int_val())
	{
		if(manual->get_int_val())
		{
			double click_x = ((double)x/width)*(x_max - x_min) + x_min;
			double click_y = ((double)(height - y)/height)*(y_max - y_min) + y_min;
			printf("%lf %lf\n", click_x, click_y);
			rob_select(click_x, click_y);
			vitesses_update();
		}
	}
}

void record_update()
{
	if(record->get_int_val())
	{
		if(!sim_record)
		{
			sim_record = fopen("out.dat", "w");
		}
		fprintf(sim_record, "%d\t%lf\n", nb_step, taux_decontamination);
	}
	else if(sim_record)
	{
		fclose(sim_record);
	}
}
	
	
