# makefile copied almost entirely in whole from the ex14 makefile
EXE=hw4

# Main target
all: $(EXE)

#  Msys/MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-O3 -Wall -DUSEGLEW
LIBS=-lfreeglut -lglew32 -lglu32 -lopengl32 -lm
CLEAN=rm -f *.exe *.o *.a
else
#  OSX
ifeq "$(shell uname)" "Darwin"
RES=$(shell uname -r|sed -E 's/(.).*/\1/'|tr 12 21)
CFLG=-O3 -Wall -Wno-deprecated-declarations -DRES=$(RES)
LIBS=-framework GLUT -framework OpenGL
#  Linux/Unix/Solaris
else
CFLG=-g -Wall
#CFLG=-O3 -Wall
LIBS=-lglut -lGLU -lGL -lm
endif
#  OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.o *.a
endif

# Dependencies
hw4.o: hw4.c arr.h ball.h cell.h funcs.h march.h quat.h vec3.h shaders.h
arr.o: arr.c arr.h
ball.o: ball.c ball.h vec3.h
cell.o: cell.c arr.h cell.h funcs.h march.h vec3.h
funcs.o: funcs.c funcs.h
march.o: march.c march.h arr.h vec3.h
quat.o: quat.c quat.h
queue.o: queue.c queue.h
shaders.o: shaders.c shaders.h
vec3.o: vec3.c vec3.h

#  Create archive
dep.a:arr.o ball.o cell.o funcs.o march.o quat.o queue.o shaders.o vec3.o
		ar -rcs $@ $^

# Compile rules
.c.o:
		gcc -c $(CFLG)  $<
.cpp.o:
		g++ -c $(CFLG)  $<

#  Link
hw4:hw4.o   dep.a
		gcc $(CFLG) -o $@ $^  $(LIBS)

#  Clean
clean:
		$(CLEAN)