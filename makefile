# makefile copied almost entirely in whole from the ex14 makefile
EXE=project

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
#CFLG=-g -Wall
CFLG=-g -Wall -Wextra -Wpedantic $(shell pkg-config --cflags sdl2) -DSDL2
LIBS=-lGLU -lGL -lm $(shell pkg-config --libs sdl2)
endif
#  OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.o *.a
endif

# Dependencies
project-1.o: project-1.c arr.h ball.h cell.h funcs.h loadtexbmp.h march.h perlin.h quat.h shaders.h text.h tree.h vec3.h
arr.o: arr.c arr.h
ball.o: ball.c ball.h vec3.h
cell.o: cell.c arr.h cell.h funcs.h march.h queue.h tree.h vec3.h loadtexbmp.h
funcs.o: funcs.c funcs.h perlin.h vec3.h
loadtexbmp.o: loadtexbmp.c loadtexbmp.h
march.o: march.c march.h arr.h vec3.h
objects.o: objects.c objects.h arr.h quat.h vec3.h
perlin.o: perlin.c perlin.h
quat.o: quat.c quat.h
queue.o: queue.c queue.h
shaders.o: shaders.c shaders.h
text.o: text.c text.h
tree.o: tree.c tree.h arr.h objects.h vec3.h perlin.h
vec3.o: vec3.c vec3.h

#  Create archive
dep.a:arr.o ball.o cell.o funcs.o loadtexbmp.o march.o objects.o perlin.o quat.o queue.o shaders.o text.o tree.o vec3.o
		ar -rcs $@ $^

# Compile rules
.c.o:
		gcc -c $(CFLG)  $<
.cpp.o:
		g++ -c $(CFLG)  $<

#  Link
project:project-1.o   dep.a
		gcc $(CFLG) -o $@ $^  $(LIBS)

#  Clean
clean:
		$(CLEAN)