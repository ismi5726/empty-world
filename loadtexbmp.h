#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef USEGLEW
#include <GL/glew.h>
#endif


#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#endif

void Fatal(const char* format, ...);
void ErrCheck(const char *where);
unsigned int LoadTexBMP(const char *file, int mode);