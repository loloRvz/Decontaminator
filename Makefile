# Compilateur a utiliser :
##########################

CC     = gcc
PP     = g++

# Options de compilation : (-g := pour deverminer)
#################################################

CFLAGS = -g

# Repertoire include :
#########################

#INCLUDE_DIR   = /usr/local/include/

# Librairies a utiliser :
#########################

LIBS   = -lglut -lglui -lGL -lGLU -lm -Llib -L/usr/X11R6/lib -lX11 -lXext -lXmu -lXi -lstdc++

# Liste de fichiers source (.c) a compiler :
############################################

CFILES = robot.c error.c particule.c utilitaire.c main.cpp simulation.c graphic.c

# Liste de modules objets (generee a partir de CFILES) :
########################################################

OFILES = robot.o error.o particule.o utilitaire.o main.o simulation.o graphic.o

# Nom du fichier executable :
#############################

CIBLE  = projet.x

# edition de liens (rassembler tous les objets -> executable)
#############################################################
	
$(CIBLE): $(OFILES)
	$(PP) $(OFILES) ${LIBS} -o $(CIBLE)

# compilation separee (production du module objet seulement)

main.o: main.cpp
	$(PP) -c main.cpp $(CFLAGS)
	
robot.o: robot.c
	$(CC) -c robot.c $(CFLAGS)	

particule.o: particule.c
	$(CC) -c particule.c $(CFLAGS)
	
utilitaire.o: utilitaire.c
	$(CC) -c utilitaire.c $(CFLAGS)	
	
error.o: error.c
	$(CC) -c error.c $(CFLAGS)
	
simulation.o: simulation.c
	$(CC) -c simulation.c $(CFLAGS)
	
graphic.o: graphic.c
	$(CC) -c graphic.c $(CFLAGS)

# Definitions de cibles particulieres :
#
# "make depend" : genere la liste des dependances
# "make clean"  : efface les fichiers .o et .x
#################################################

depend:
	@echo " *** MISE A JOUR DES DEPENDANCES ***"
	@(sed '/^# DO NOT DELETE THIS LINE/q' Makefile && \
	  $(CC) -MM $(CFLAGS) $(CFILES) | \
	  egrep -v "/usr/include" \
	 ) >Makefile.new
	@mv Makefile.new Makefile

clean:
	@echo " *** EFFACE MODULES OBJET ET EXECUTABLE ***"
	@/bin/rm -f *.o projet.x

#
# -- Regles de dependances generees par "make depend"
#####################################################
# DO NOT DELETE THIS LINE
robot.o: robot.c robot.h particule.h utilitaire.h tolerance.h \
 constantes.h graphic.h error.h
error.o: error.c error.h constantes.h tolerance.h
particule.o: particule.c particule.h utilitaire.h tolerance.h \
 constantes.h graphic.h error.h
utilitaire.o: utilitaire.c utilitaire.h tolerance.h constantes.h \
 graphic.h
main.o: main.cpp simulation.h robot.h particule.h utilitaire.h \
 tolerance.h constantes.h graphic.h error.h
simulation.o: simulation.c simulation.h robot.h particule.h utilitaire.h \
 tolerance.h constantes.h graphic.h error.h
graphic.o: graphic.c graphic.h
