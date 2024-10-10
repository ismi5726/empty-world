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
//#include "ball.h"
#include "march.h"
#include "arr.h"
#include "vec3.h"
#include "shaders.h"
#include "cell.h"
#include "funcs.h"
#include "queue.h"


// total rotation
quat rotation = {0, 0, 0, 1};
quat identity = {0, 0, 0, 1};

int mouseX = 0;
int mouseY = 0;
// mouse button states
int down[3] = {0, 0, 0};
char keyDown = 0;
int isInside = GLUT_LEFT;
// mouse rotation sensitivity
double sensitivity = 400;

// offsets for panning
double offsetX = 0;
double offsetY = 0;

// for movement
#ifndef PI
#define PI 3.141592653
#endif
//double position[3] = {0, 0, 0};
//double velocity[3] = {0, 0, 0};
//double acceleration[3] = {0, 0, 0};
vec3f position = {0, 0, 0};
vec3f pos1 = {0, 0, -10};
vec3f pos2 = {0, 0, 0};
vec3f velocity = {0, 0, 0};
vec3f acceleration = {0, 0, 0};
vec3f vZero = {0, 0, 0};
int onFloor = 0;
double phi = -30;
double theta = 45;

double dim = 30;
// used during resets
double dimInit;
double clipPlane = 200;
// these are useful to have for other calculations
double asp = 1;
// horizontal aspect ratio - 1 if taller than wide, otherwise asp
double hAsp = 1;
// vertical aspect ratio - 1 if wider than tall, otherwise 1/asp (if asp is non-zero)
double wAsp = 1;

int width = 500;
int height = 500;

unsigned int ID;

/*growArr triangles;

int nBalls = 7;
ball *balls = NULL;
double threshhold = 1;*/

funcDef terrain;

vec3f cellSize = {16, 16, 16};
cell *corner;
int lodCutoffs[3] = {10, 5, 2};
int renderDistance = 25;
queue *cellQueue;

int mode = 0;
int perspective = 0;
// for blocking the movement event from warpPointer
int warp = 0;

int oldT = 0;

typedef struct dodeData {
    float tX;
    float tZ;
    float scale;
    float rX;
    float rY;
    float rZ;
} dodeData;

#define nDodes 5
dodeData dodes[nDodes];

char state[][20] = {"Ortho", "Perspective", "Walking (outside)", "Walking"};

void createGrid(int, int);
void clearGrid();

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
    glColor3f(1.0, 1.0, 1.0);
    glDrawElements(GL_TRIANGLES, 108, GL_UNSIGNED_BYTE, faces);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void setProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (perspective) {
        double bottom = tan(60 * 3.14159/360.0) * 0.01;
        glFrustum(-asp * bottom, asp * bottom, -bottom, bottom, 0.01, dim + clipPlane);
        if (perspective == 1) {
            glTranslated(0, 0, -dim * 1.7);
            glRotated(phi, -1, 0, 0);
            glRotated(theta, 0, -1, 0);
            glTranslatef(position.x, 0, position.z);
            //glTranslated(0, 0, -dim * 1.7);
        } else {
            //glRotated(-theta, cos(phi), 0, sin(phi));
            glRotated(phi, -1, 0, 0);
            glRotated(theta, 0, -1, 0);
            glTranslatef(position.x, -position.y, position.z);
        }
    } else {
        glOrtho(-hAsp*dim,+hAsp*dim, -wAsp*dim,+wAsp*dim, -clipPlane,+clipPlane);
        glRotated(phi, -1, 0, 0);
        glRotated(theta, 0, -1, 0);
        glTranslatef(position.x, 0, position.z);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glutPostRedisplay();
}

void drawGrid() {
    cell *row = corner, *current;

    glEnableClientState(GL_VERTEX_ARRAY);
    for (; row; row = row->nextZ) {
        current = row;
        for (; current; current = current->nextX) {
            glVertexPointer(3, GL_FLOAT, 0, current->tris.data);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glDrawArrays(GL_TRIANGLES, 0, current->tris.size / 3);
        }
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glLoadIdentity();

    if (perspective < 3) {
        // we need to translate before rotating so that rotating doesn't also rotate the direction of translation
        // because of weird matrix multiplication stuff
        glTranslated(offsetX, offsetY, 0);
        // load current rotation
        double *rot = toMat4(rotation);
        glMultMatrixd(rot);
        free(rot);
    }

    float *matrix = calloc(16, sizeof(float));
    glUseProgram(ID);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glUniformMatrix4fv(glGetUniformLocation(ID, "modelview"), 1, GL_FALSE, matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    glUniformMatrix4fv(glGetUniformLocation(ID, "projection"), 1, GL_FALSE, matrix);
    ErrCheck("display1");
    free(matrix);
    drawGrid();
    glUseProgram(0);
    
    for (int i = 0; i < nDodes; i++) {
        glPushMatrix();
        glTranslatef(dodes[i].tX, 0, dodes[i].tZ);
        glScalef(dodes[i].scale, dodes[i].scale, dodes[i].scale);
        glRotatef(dodes[i].rX, 1, 0, 0);
        glRotatef(dodes[i].rY, 0, 1, 0);
        glRotatef(dodes[i].rZ, 0, 0, 1);
        dodecahedron();
        glPopMatrix();
    }
    // draw a nice grassy field so it looks like we're walking
    if (perspective > 1) {
        glColor3f(0.05, 0.6, 0.1);
        glBegin(GL_TRIANGLES);
        glVertex3f(-100 - position.x, -2, -100 - position.z);
        glVertex3f(100 - position.x, -2, -100 - position.z);
        glVertex3f(-100 - position.x, -2, 100 - position.z);
        glVertex3f(100 - position.x, -2, -100 - position.z);
        glVertex3f(100 - position.x, -2, 100 - position.z);
        glVertex3f(-100 - position.x, -2, 100 - position.z);
        glEnd();
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
    Print(state[perspective]);

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
        setProjection();
    }
    mouseX = x;
    mouseY = y;
}

void motion(int x, int y) {
    if (warp) {
        warp = 0;
        return;
    }
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
        // left mouse button - rotate the scene
        phi -= (double)adY / 4;
        if (phi > 90) phi = 90;
        else if (phi < -90) phi = -90;
        theta -= (double)adX / 4;
        setProjection();
        shouldRedisplay = 1;
    }
    mouseX = x;
    mouseY = y;
    if (shouldRedisplay) {
        glutPostRedisplay();
    }
}

void entry(int state) {
    isInside = state;
}

void key(unsigned char key, int x, int y) {
    switch (key) {
        // exit on 'Q' or escape (was listed as a nice thing to do)
        case 'q':
        case 27:
            // space here to avoid accidentally doing anything with terminal codes
            printf("%s\n", "Thanks for playing :) ");
            exit(0);
            break;
        case 'm':
            perspective++;
            perspective %= 4;
            switch (perspective) {
                case 2:
                    position = pos1;
                    perspective++;
                case 3:
                    pos1 = position;
                    position = pos2;
                    mode = 0;
                    break;
                case 0:
                    pos2 = position;
                    break;
            }
            velocity = vZero;
            acceleration = vZero;
            setProjection();
            break;
        case 'w':
        case 'a':
        case 's':
        case 'd':
            if (perspective > 1) {
                keyDown = key;
            }
            break;
        // jump with space
        case 32:
            if (onFloor) {
                velocity.y += 5;
                onFloor = 0;
            } else {
            }
            break;
        case '0':
            position = vZero;
            if (perspective == 2) {
                position.z = -10;
            }
            phi = -30;
            theta = 45;
            offsetX = 0;
            offsetY = 0;
            rotation = identity;
            dim = dimInit;
            break;
    }
}

void keyUp(unsigned char key, int x, int y) {
    if (key == keyDown) {
        keyDown = 0;
    }
}

void idle() {
    int time = glutGet(GLUT_ELAPSED_TIME);
    int shouldRedisplay = 1;
    // note: changed this from a while loop to an if statement
    // so if the physics take too long to calculate (they shouldn't, but just in case)
    // it won't start running super behind
    // however, this means that the physics are max 60 fps
    if (oldT < time) {
        switch (keyDown) {
            case 'w':
            case 's':
                acceleration.x += sin(theta * PI / 180) / 50 * (keyDown == 'w' ? 1 : -1);
                acceleration.z += cos(theta * PI / 180) / 50 * (keyDown == 'w' ? 1 : -1);
                break;
            case 'a':
            case 'd':
                acceleration.x += cos(theta * PI / 180) / 50 * (keyDown == 'a' ? 1 : -1);
                acceleration.z -= sin(theta * PI / 180) / 50 * (keyDown == 'a' ? 1 : -1);
                break;
            default:
                shouldRedisplay = 0;
                break;
        }
        if (perspective == 2) {
            if (position.y > 0) {
                acceleration.y -= 0.1;
            } else {
                onFloor = 1;
            }
            shouldRedisplay = 1;
        } else if (perspective == 3) {
            vec3f pos = {position.x, position.y - 2, position.z};
            if (terrain.influenceAt(pos) > terrain.threshhold) {
                acceleration.y -= 0.1;
            }
            shouldRedisplay = 1;
        }
        acceleration = scalef(acceleration, 0.2);
        acceleration.y *= 0.5;
        velocity = add(velocity, scalef(acceleration, 20));
        velocity = scalef(velocity, onFloor ? 0.95 : 0.98);
        if (perspective == 2) {
            position = add(position, scalef(velocity, 1 / 20.0));
            if (position.y < 0) {
                position.y = 0;
                velocity.y = 0;
                acceleration.y = 0;
                onFloor = 1;
            }
        } else if (perspective == 3) {
            vec3f pos = add(position, scalef(velocity, 1 / 20.0));
            float grav = velocity.y;
            pos.y = position.y;
            int n = 0;
            int rVelocity = 0;
            while (terrain.influenceAt(pos) < terrain.threshhold && n++ < 2) {
                velocity = scalef(velocity, 0.5);
                pos = sub(pos, scalef(velocity, 1 / 20.0));
            }
            if (n == 3) {
                rVelocity = 1;
                pos = position;
                velocity.y = grav;
            } else {
            }
            pos.y += grav / 20.0 - 2;
            n = 0;
            double inf;
            if (terrain.influenceAt(pos) < terrain.threshhold) {
                pos.y -= grav / 20.0;
                velocity.y = 0;
                onFloor = 1;
            }
            while ((inf = terrain.influenceAt(pos)) < terrain.threshhold && n++ < 15) {
                pos.y += terrain.threshhold - inf + 0.01;
                velocity.y = 0;
            }
            if (n == 16) {
                position = sub(position, scalef(velocity, 1 / 20.0));
                rVelocity = 1;
                printf("can't solve\n");
            } else if (n > 0) {
                position = pos;
                position.y += 2;
                velocity.y = 0;
            } else {
                position = pos;
                position.y += 2;
            }
            if (rVelocity) {
                velocity = vZero;
            }
        }
        oldT += 1000 / 60.0;
    }
    cell *toRender = peek(cellQueue);
    if (toRender) {
        //printf("%f, %f\n", toRender->pos.x, toRender->pos.z);
        cellQueue = pop(cellQueue);
        // avoid re-rendering cells we don't need
        if (toRender->needsRender) {
            calculateTris(toRender, terrain);
            shouldRedisplay = 1;
        }
    }
    if (shouldRedisplay) {
        setProjection();
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
    //  Orthogonal projection box adjusted for the
    //  aspect ratio of the window
    asp = (height>0) ? (double)width/height : 1;
    hAsp = asp < 1 ? 1 : asp;
    // should avoid any divide-by-zero errors
    wAsp = asp < 1 ? (width > 0 ? 1 / asp : 1) : 1;
    setProjection();
}

void createGrid(int centerX, int centerZ) {
    vec3f cellPos = {centerX, -8, centerZ};
    vec3f centerPos = cellPos;
    vec3f cellwh = {cellSize.x, 0, cellSize.z};
    corner = NULL;
    vec3f rowPos = sub(cellPos, scalef(cellwh, renderDistance));
    vec3f pos = rowPos;
    cell *prevRow = NULL;
    cell *row = NULL;
    cell *prev;
    cell *current;
    int dist;
    int lod;
    for (int z = 0; z < 2 * renderDistance + 1; z++) {
        prev = NULL;
        row = NULL;
        for (int x = 0; x < 2 * renderDistance + 1; x++) {
            current = createCell(pos, cellSize);
            if (!corner) corner = current;
            if (!row) row = current;
            if (prev) {
                prev->nextX = current;
                current->prevX = prev;
            }
            if (prevRow) {
                prevRow->nextZ = current;
                current->prevZ = prevRow;
                prevRow = prevRow->nextX;
            }
            dist = roughCellDistance(current, centerPos);
            for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
            current->lod = lod;
            cellQueue = enqueue(cellQueue, current, dist);
            pos.x += cellSize.x;
            prev = current;
        }
        prevRow = row;
        rowPos.z += cellSize.z;
        pos = rowPos;
    }
}

void clearGrid() {
    cell *current = corner;
    cell *next;
    cell *nextRow = current->nextZ;
    for (; current;) {
        for (; current; current = next) {
            next = current->nextX;
            delCell(current);
        }
        current = nextRow;
        if (current) {
            nextRow = current->nextZ;
        }
    }
    corner = NULL;
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
    glutKeyboardUpFunc(keyUp);
    glutIdleFunc(idle);
    srand(time(NULL));
    oldT = glutGet(GLUT_ELAPSED_TIME);
    dimInit = dim;
    ID = createProgram("vert.vert", "frag.frag", 1);
    for (int i = 0; i < nDodes; i++) {
        dodes[i].rX = rand() % 360;
        dodes[i].rY = rand() % 360;
        dodes[i].rZ = rand() % 360;
        dodes[i].scale = rand() % 20 + 10;
        dodes[i].tX = (rand() % 400) - 200;
        dodes[i].tZ = (rand() % 400) - 200;
    }
    createGrid(0, 0);
    terrain = sineHill;
    calculateTris(corner, terrain);
    glutMainLoop();
    return 0;
}