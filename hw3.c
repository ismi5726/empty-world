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
double phi = -PI / 2;
double theta = 0;

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
double threshhold = 1;

int mode = 0;
int perspective = 0;
// for blocking the movement event from warpPointer
int warp = 0;

int oldT = 0;

char state[][20] = {"Ortho", "Perspective", "Walking (outside)", "Walking (inside)"};

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

void setProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (perspective) {
        double bottom = tan(60 * 3.14159/360.0) * 0.01;
        glFrustum(-asp * bottom, asp * bottom, -bottom, bottom, 0.01, dim + clipPlane / 2);
        if (perspective == 1) {
            glTranslated(0, 0, -dim * 1.7);
        } else {
            //glRotated(-theta, cos(phi), 0, sin(phi));
            glRotated(phi, -1, 0, 0);
            glRotated(theta, 0, -1, 0);
            glTranslatef(position.x, -position.y, position.z);
        }
    } else {
        glOrtho(-hAsp*dim,+hAsp*dim, -wAsp*dim,+wAsp*dim, -clipPlane,+clipPlane);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glLoadIdentity();
    // draw a nice grassy field so it looks like we're walking
    if (perspective > 1) {
        glColor3f(0.05, 0.6, 0.1);
        glBegin(GL_TRIANGLES);
        glVertex3f(-100, -2, -100);
        glVertex3f(100, -2, -100);
        glVertex3f(-100, -2, 100);
        glVertex3f(100, -2, -100);
        glVertex3f(100, -2, 100);
        glVertex3f(-100, -2, 100);
        glEnd();
    }

    if (perspective < 3) {
        // we need to translate before rotating so that rotating doesn't also rotate the direction of translation
        // because of weird matrix multiplication stuff
        glTranslated(offsetX, offsetY, 0);
        // load current rotation
        double *rot = toMat4(rotation);
        glMultMatrixd(rot);
        free(rot);
    }
    
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
    if (perspective < 2) {
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
    } else {
        if (down[GLUT_LEFT_BUTTON]) {
            phi -= (double)adY / 4;
            theta -= (double)adX / 4;
            setProjection();
            shouldRedisplay = 1;
        }
        if (down[GLUT_MIDDLE_BUTTON] && perspective == 2) {
            // right mouse button - pan the attractor
            // this might seem like a bunch of magic numbers, but it's perfect to make moving smooth at any window size
            double dist = hypot(position.x, position.z) / 1.7;
            offsetX += 2 * dX * dist * hAsp / width;
            offsetY -= 2 * dY * dist * wAsp / height;
            shouldRedisplay = 1;
        }
        if (down[GLUT_RIGHT_BUTTON] && perspective == 2) {
            // left mouse button - rotate the attractor around its origin
            quat temp_rot = fromEuler(adY, adX, 0);
            rotation = multiply(temp_rot, rotation);
            shouldRedisplay = 1;
        }
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
            if (perspective == 3) break;
            mode = !mode;
            glutPostRedisplay();
            break;
        case 'p':
            perspective++;
            perspective %= 4;
            switch (perspective) {
                case 2:
                    position = pos1;
                    break;
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
            phi = -PI / 2;
            theta = 0;
            offsetX = 0;
            offsetY = 0;
            rotation = identity;
            mode = 0;
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
            if (influenceAt(pos, balls, nBalls) > threshhold) {
                acceleration.y -= 0.1;
            }
            shouldRedisplay = 1;
        }
        acceleration = scalef(acceleration, 0.2);
        acceleration.y *= 0.5;
        velocity = add(velocity, scalef(acceleration, 20));
        velocity = scalef(velocity, 0.95);
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
            while (influenceAt(pos, balls, nBalls) < threshhold && n++ < 2) {
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
            if (influenceAt(pos, balls, nBalls) < threshhold) {
                pos.y -= grav / 20.0;
                velocity.y = 0;
                onFloor = 1;
            }
            while ((inf = influenceAt(pos, balls, nBalls)) < threshhold && n++ < 15) {
                pos.y += threshhold - inf + 0.01;
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
                    triangles = march(cell, triangles, threshhold);
                }
            }
        }
    }
    free(influence);
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