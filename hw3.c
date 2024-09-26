#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif

#ifndef RES
#define RES 1
#endif

#define LEN 8192  // Maximum length of text string

#include "quat.h"
#include "ball.h"
#include "march.h"
#include "arr.h"
#include "vec3.h"
#include "shaders.h"


// total rotation
quat rotation = {0, 0, 0, 1};

int mouseX = 0;
int mouseY = 0;
// mouse button states
int down[3] = {0, 0, 0};
int isInside = GLUT_LEFT;
// mouse rotation sensitivity
double sensitivity = 400;

// offsets for panning
double offsetX = 0;
double offsetY = 0;

double dim = 30;
// used during resets
double dimInit;
double clipPlane = 50;
// these are useful to have for other calculations
double asp = 1;
// horizontal aspect ratio - 1 if taller than wide, otherwise asp
double hAsp = 1;
// vertical aspect ratio - 1 if wider than tall, otherwise 1/asp (if asp is non-zero)
double wAsp = 1;

int width = 500;
int height = 500;

unsigned int ID;

growArr triangles;

int nBalls = 7;
ball *balls = NULL;

int mode = 0;

void genTriangles();

/*
 *  Copied from example code (ex6.c)
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

/*
 * Copied from example code (ex8.c)
 * Check for OpenGL errors
 */
void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

/*
 *  Copied from example code (ex6.c)
 *  Print message to stderr and exit
 */
void Fatal(const char* format , ...)
{
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}

void dodecahedron() {
    double phi = (1 + sqrt(5)) / 2;
    double coords[] = {
        1, 1, 1,
        1, 1, -1,
        1, -1, 1,
        1, -1, -1,
        -1, 1, 1,
        -1, 1, -1,
        -1, -1, 1,
        -1, -1, -1,
        0, phi, 1/phi,
        0, phi, -1/phi,
        0, -phi, 1/phi,
        0, -phi, -1/phi,
        1/phi, 0, phi,
        1/phi, 0, -phi,
        -1/phi, 0, phi,
        -1/phi, 0, -phi,
        phi, 1/phi, 0,
        phi, -1/phi, 0,
        -phi, 1/phi, 0,
        -phi, -1/phi, 0
    };
    char faces[] = {
        0, 1, 16, 0, 1, 8, 1, 8, 9,
        1, 3, 13, 1, 3, 16, 3, 16, 17,
        3, 2, 17, 3, 2, 10, 3, 10, 11,
        2, 0, 12, 2, 0, 16, 2, 16, 17,
        4, 5, 18, 4, 5, 8, 5, 8, 9,
        5, 7, 15, 5, 7, 18, 7, 18, 19,
        7, 6, 19, 7, 6, 11, 6, 11, 10,
        6, 4, 14, 6, 4, 18, 6, 18, 19,
        0, 4, 8, 0, 4, 12, 4, 12, 14,
        1, 5, 9, 1, 5, 13, 5, 13, 15,
        2, 6, 10, 2, 6, 14, 2, 14, 12,
        3, 7, 11, 3, 7, 15, 3, 15, 13
    };
    // colors from ex6.c
    float colors[] = {
        0.0,0.0,1.0,
        0.0,1.0,0.0,
        0.0,1.0,1.0,
        1.0,0.0,0.0,
        1.0,0.0,1.0,
        1.0,1.0,0.0,
        0.0,0.0,1.0,
        0.0,1.0,0.0,
        0.0,1.0,1.0,
        1.0,0.0,0.0,
        1.0,0.0,1.0,
        1.0,1.0,0.0,
    };
 

    glVertexPointer(3, GL_DOUBLE, 0, coords);
    glEnableClientState(GL_VERTEX_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, colors);
    glEnableClientState(GL_COLOR_ARRAY);
    /*glIndexPointer(GL_INT, 0, faces);
    glEnableClientState(GL_INDEX_ARRAY);*/
    glColor3f(1.0, 1.0, 1.0);
    glDrawElements(GL_TRIANGLES, 108, GL_UNSIGNED_BYTE, faces);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glLoadIdentity();

    // we need to translate before rotating so that rotating doesn't also rotate the direction of translation
    // because of weird matrix multiplication stuff
    glTranslated(offsetX, offsetY, 0);

    // load current rotation
    double *rot = toMat4(rotation);
    glMultMatrixd(rot);
    free(rot);
    
    if (mode == 0) {
        float *matrix = calloc(16, sizeof(float));
        glUseProgram(ID);
        glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
        glUniformMatrix4fv(glGetUniformLocation(ID, "modelview"), 1, GL_FALSE, matrix);
        glGetFloatv(GL_PROJECTION_MATRIX, matrix);
        glUniformMatrix4fv(glGetUniformLocation(ID, "projection"), 1, GL_FALSE, matrix);
        ErrCheck("display1");
        free(matrix);

        glVertexPointer(3, GL_FLOAT, 0, triangles.data);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, triangles.size / 3);

        glDisableClientState(GL_VERTEX_ARRAY);
        glUseProgram(0);
    } else {
        glPushMatrix();
        glRotated(12, 1, 0, 3);
        glScaled(1.5, 1.5, 1.5);
        dodecahedron();
        glPopMatrix();
        glPushMatrix();
        glTranslated(6, 2, 1);
        glRotated(20, 0, -2, 1);
        dodecahedron();
        glPopMatrix();
        glPushMatrix();
        glScaled(3, 3, 3);
        glTranslated(-5, -5, 3);
        dodecahedron();
        glPopMatrix();
    }

    double gridSize = dim / 10;
    double offset = 2 * (dim / dimInit);
    glLineWidth(2);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(gridSize, 0.0, 0.0);
    glEnd();
    glRasterPos3f(gridSize + offset, 0.0, 0.0);
    Print("x");

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, gridSize, 0.0);
    glEnd();
    glRasterPos3f(0.0, gridSize + offset, 0.0);
    Print("y");

    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, gridSize);
    glEnd();
    glRasterPos3f(0.0, 0.0, gridSize + offset);
    Print("z");
    glLineWidth(1);
    glColor3f(1.0, 1.0, 1.0);

    glLoadIdentity();

    glWindowPos2i(5, 5);

    glFlush();
    glutSwapBuffers();
}

void mouse(int button, int up, int x, int y) {
    // if it's one of the 3 main buttons, just set it to down
    // no buffer-overflow weirdness here !
    if (button < 3 && button >= 0)
        down[button] = !up;
    // only process on the down event for the scroll wheel
    else if (!up) {
        dim *= (button == 4) ? 1.1 : (1 / 1.1);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-hAsp*dim,+hAsp*dim, -wAsp*dim,+wAsp*dim, -clipPlane,+clipPlane);
        glMatrixMode(GL_MODELVIEW);
        glutPostRedisplay();
    }
    mouseX = x;
    mouseY = y;
}

void motion(int x, int y) {
    // we still want to keep track of the mouse even when it's outside of the window to keep dragging smooth
    if (!isInside) {
        mouseX = x;
        mouseY = y;
        return;
    }
    double dX = x - mouseX;
    double dY = y - mouseY;
    double adjSensitivity = sensitivity / (double) width;
    double adX = dX * adjSensitivity;
    double adY = dY * adjSensitivity;
    int shouldRedisplay = 0;
    if (down[GLUT_LEFT_BUTTON]) {
        // left mouse button - rotate the attractor around its origin
        quat temp_rot = fromEuler(adY, adX, 0);
        rotation = multiply(temp_rot, rotation);
        shouldRedisplay = 1;
    }
    if (down[GLUT_RIGHT_BUTTON]) {
        // right mouse button - pan the attractor
        // this might seem like a bunch of magic numbers, but it's perfect to make moving smooth at any window size
        offsetX += 2 * dX * dim * hAsp / width;
        offsetY -= 2 * dY * dim * wAsp / height;
        shouldRedisplay = 1;
    }
    mouseX = x;
    mouseY = y;
    if (shouldRedisplay) glutPostRedisplay();
}

void entry(int state) {
    isInside = state;
}

void key(unsigned char key, int x, int y) {
    if (key == 'm') {
        mode = !mode;
        glutPostRedisplay();
    }
}

/*
 *  copied from example code (ex6.c)
 *  modified to use the smaller dimension as a size reference to avoid ever clipping
 *  GLUT calls this routine when the window is resized
 */
void reshape(int w,int h) {
    width = w;
    height = h;
    //  Set the viewport to the entire window
    glViewport(0,0, RES*width,RES*height);
    //  Tell OpenGL we want to manipulate the projection matrix
    glMatrixMode(GL_PROJECTION);
    //  Undo previous transformations
    glLoadIdentity();
    //  Orthogonal projection box adjusted for the
    //  aspect ratio of the window
    asp = (height>0) ? (double)width/height : 1;
    hAsp = asp < 1 ? 1 : asp;
    // should avoid any divide-by-zero errors
    wAsp = asp < 1 ? (width > 0 ? 1 / asp : 1) : 1;
    glOrtho(-hAsp*dim,+hAsp*dim, -wAsp*dim,+wAsp*dim, -clipPlane,+clipPlane);
    //glFrustum(-hAsp*dim,+hAsp*dim, -wAsp*dim,+wAsp*dim, 0.1,+clipPlane);
    //  Switch to manipulating the model matrix
    glMatrixMode(GL_MODELVIEW);
    //  Undo previous transformations
    glLoadIdentity();
}

void genTriangles() {
    vec3f min, max;
    maxArea(balls, 1, &max, &min);
    vec3f areasize = sub(max, min);
    double precision = .1;
    areasize = scalef(areasize, 1 / precision);
    // comment this line out if using a number that is well-represented by a float (e.g. 0.5, 1, etc.)
    // you'll know if this line needs to be commented out (everything will look broken)
    // conversely, if the program segfaults, uncomment it
    areasize.x++;areasize.y++;areasize.z++;
    float *influence = calloc((int)(areasize.x * areasize.y * areasize.z), sizeof(float));
    int n = 0;
    vec3f pos;
    float maxInfluence = 0;
    for (float z = min.z; z < max.z; z += precision) {
        pos.z = z;
        for (float y = min.y; y < max.y; y += precision) {
            pos.y = y;
            for (float x = min.x; x < max.x; x += precision) {
                pos.x = x;
                influence[n++] = influenceAt(pos, balls, nBalls);
                if (influence[n-1] > maxInfluence) {
                    maxInfluence = influence[n-1];
                    //printf("max: %f", maxInfluence);
                }
            }
        }
    }
    n = 0;
    for (float z = min.z; z < max.z; z += precision) {
        pos.z = z;
        for (float y = min.y; y < max.y; y += precision) {
            pos.y = y;
            for (float x = min.x; x < max.x; x += precision) {
                pos.x = x;
                if (n + 1 + (int) (areasize.x * areasize.y) < (int)(areasize.x * areasize.y * areasize.z)) {
                    marchData cell = {{influence[n], influence[n + 1],
                        influence[n + 1 + (int)areasize.x], influence[n + (int)areasize.x],
                        influence[n + (int)(areasize.x * areasize.y)], influence[n + 1 + (int)(areasize.x * areasize.y)],
                        influence[n + 1 + (int)areasize.x + (int)(areasize.x * areasize.y)], influence[n + (int)areasize.x + (int)(areasize.x * areasize.y)],
                    }, pos, (float)precision};
                    n++;
                    triangles = march(cell, triangles, 1);
                }
            }
        }
    }
    free(influence);
    //printf("%i\n", triangles.size);
}

// the main function is mostly copied from the example code (likely ex6.c)
// but I've added other functions as needed
int main(int argc, char* argv[]) {
    // initialize everything
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Isaac Mitchell");
#ifdef USEGLEW
    if (glewInit() != GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
    // event handlers for all of the different events I used
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutEntryFunc(entry);
    glutKeyboardFunc(key);
    srand(time(NULL));
    triangles = createArr();
    balls = calloc(nBalls, sizeof(ball));
    balls[0].influence = 5;
    balls[1].pos.x = 3.5;
    balls[1].influence = 2;
    balls[2].pos.y = 2.5;
    balls[2].influence = 4;
    balls[3].pos.z = 1;
    balls[3].influence = -1.5;
    balls[4].pos.x = 3;
    balls[4].pos.y = 2;
    balls[4].pos.z = 1;
    balls[4].influence = 5;
    balls[5].pos.x = -4;
    balls[5].influence = 5;
    balls[6].pos.x = -3;
    balls[6].pos.z = 1;
    balls[6].influence = 1;
    genTriangles();
    ID = createProgram("vert.vert", "frag.frag", 1);
    glutMainLoop();
    return 0;
}