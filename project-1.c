#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES
#ifdef SDL2
#include <SDL2/SDL.h>
//#include <SDL2/SDL_mixer>
#elif defined(__APPLE__)
#include <GLUT/glut.h>
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif

#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glu.h>
#include <GL/gl.h>
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
#include "loadtexbmp.h"
#include "perlin.h"

SDL_Window *window = NULL;
int rate;
int width, height;


// total rotation
quat rotation = {0, 0, 0, 1};
quat identity = {0, 0, 0, 1};

int mouseX = 0;
int mouseY = 0;
// mouse button states
int down[3] = {0, 0, 0};
char keyDown = 0;
//int isInside = GLUT_LEFT;
char hasFocus = 0;
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

double dim = 300;
// used during resets
double dimInit;
double clipPlane = 500;
// these are useful to have for other calculations
double asp = 1;
// horizontal aspect ratio - 1 if taller than wide, otherwise asp
double hAsp = 1;
// vertical aspect ratio - 1 if wider than tall, otherwise 1/asp (if asp is non-zero)
double wAsp = 1;

unsigned int ID;

funcDef terrain;

vec3f cellSize = {16, 16, 16};
cell *corner;
int lodCutoffs[3] = {10, 5, 2};
int lods = 3;
int renderDistance = 25;
queue *cellQueue;

int mode = 0;
int perspective = 3;
// for blocking the movement event from warpPointer
int warp = 0;

int oldT = 0;
int lightOffset = 0;
int lightStopped = 0;

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

int groundTexture;
int crateTexture;
int sunTexture;
int textures[3];
int textureIndex = 0;

char shouldDisplay = 1;

void createGrid(int, int);
void clearGrid();

void quitProgram(int code) {
    SDL_Quit();
    exit(code);
}

void glutPostRedisplay() {
    shouldDisplay = 1;
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
            glTranslatef(position.x, -position.y - 4, position.z);
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

void mouse(int button, int up, int x, int y) {
    printf("%i %i\n", button, up);
    // if it's one of the 3 main buttons, just set it to down
    // no buffer-overflow weirdness here !
    if (button < 3 && button >= 0) {
        down[button] = !up;

        SDL_SetRelativeMouseMode(SDL_TRUE);
        hasFocus = 1;
    }
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
    /*if (!isInside) {
        mouseX = x;
        mouseY = y;
        return;
    }*/
    //double dX = x - mouseX;
    //double dY = y - mouseY;
    double dX = x;
    double dY = y;
    double adjSensitivity = sensitivity / (double) width;
    double adX = dX * adjSensitivity;
    double adY = dY * adjSensitivity;
    int shouldRedisplay = 0;
    if (hasFocus) {
        // left mouse button - rotate the scene
        phi -= (double)adY / 4;
        if (phi > 90) phi = 90;
        else if (phi < -90) phi = -90;
        theta -= (double)adX / 4;
        setProjection();
        shouldRedisplay = 1;
    }
    //mouseX = x;
    //mouseY = y;
    SDL_WarpMouseInWindow(window, width / 2, height / 2);
    if (shouldRedisplay) {
        glutPostRedisplay();
    }
}

void key(unsigned char key, int _x, int _y) {
    (void)(_x);
    (void)(_y);
    switch (key) {
        // exit on 'Q' or escape (was listed as a nice thing to do)
        case 'q':
        case 27:
            // space here to avoid accidentally doing anything with terminal codes
            printf("%s\n", "Thanks for playing :) ");
            exit(0);
            break;
        case 'p':
            perspective++;
            perspective %= 4;
            switch (perspective) {
                case 2:
                    position = pos1;
                    perspective++;
                    // fall through
                case 3:
                    pos1 = position;
                    position = pos2;
                    break;
                case 0:
                    pos2 = position;
                    break;
            }
            velocity = vZero;
            acceleration = vZero;
            setProjection();
            break;
        case 'm':
            mode++;
            mode %= 3;
            break;
        case 't':
            textureIndex++;
            textureIndex %= 3;
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
                velocity.y -= 5;
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
        case 'o':
            lightStopped = 1 - lightStopped;
            break;
        case 'e':
            SDL_SetRelativeMouseMode(SDL_FALSE);
            hasFocus = 0;
            break;
    }
}

void keyUp(unsigned char key, int _x, int _y) {
    (void)(_x);
    (void)(_y);
    if (key == keyDown) {
        keyDown = 0;
    }
}

void processEvents() {
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quitProgram(0);
                break;
            case SDL_WINDOWEVENT:
                //printf("%i, %i, %i\n", event.window.data1, event.window.data2, event.window.event);
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                        reshape(event.window.data1, event.window.data2);
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                if (hasFocus) motion(event.motion.xrel, event.motion.yrel);
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouse(event.button.button - 1, 0, event.button.x, event.button.y);
                break;
            case SDL_MOUSEBUTTONUP:
                mouse(event.button.button - 1, 1, event.button.x, event.button.y);
                break;
            case SDL_MOUSEWHEEL:
                mouse((event.wheel.y < 0) + 3, 0, event.wheel.mouseX, event.wheel.mouseY);
                break;
            case SDL_KEYDOWN:
                key(event.key.keysym.sym, 0, 0);
                break;
            case SDL_KEYUP:
                keyUp(event.key.keysym.sym, 0, 0);
        }
    }
}

void dodecahedron() {
    double phi = (1 + sqrt(5)) / 2;
    float coords[] = {
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
    float faceTexture[] = {
        0, 1, 0,
        1, 0, 0,
        1, 1, 0,
        0, 1, 0,
        1, 0, 0,
        0, 0.75, 0,
        1, 0, 0,
        0.75, 0, 0,
        0, 0.75, 0,
    };
    float faceTexture2[] = {
        0.5, 1, 0,
        1, 1, 0,
        1, 0, 0,
        0, 0, 0,
        0, 1, 0,
        0.5, 0.5, 0,
    };
    float faceTexture3[] = {
        0, 1 / phi, 0,
        1, 1 / phi, 0,
        0.5, 1, 0,
        0, 1 / phi, 0,
        1, 1 / phi, 0,
        1 - phi / 2, 0, 0,
        1, 1 / phi, 0,
        phi / 2, 0, 0,
        1 - phi / 2, 0, 0,
    };
    char faces[] = {
        0, 1, 16, 0, 1, 8, 1, 9, 8,
        1, 3, 13, 1, 3, 16, 3, 17, 16, 
        3, 2, 17, 3, 2, 11, 2, 10, 11,
        0, 2, 12, 0, 2, 16, 2, 17, 16,
        4, 5, 18, 4, 5, 8, 5, 9, 8, 
        5, 7, 15, 5, 7, 18, 7, 19, 18,
        7, 6, 19, 7, 6, 11, 6, 10, 11, 
        4, 6, 14, 4, 6, 18, 6, 19, 18, 
        0, 4, 8, 0, 4, 12, 4, 14, 12,
        1, 5, 9, 1, 5, 13, 5, 15, 13, 
        6, 2, 10, 6, 2, 14, 2, 12, 14,
        7, 3, 11, 7, 3, 15, 3, 13, 15 
    };
    // the faces from above, after having been sorted around their centers and de-duplicated
    // sorted using algorithm from https://stackoverflow.com/a/47956568
    char faces2[] = {
        16,0,8,9,1,
        13,3,17,16,1,
        17,3,11,10,2,
        12,0,16,17,2,
        18,5,9,8,4,
        15,5,18,19,7,
        19,6,10,11,7,
        14,6,19,18,4,
        0,12,14,4,8,
        9,5,15,13,1,
        10,6,14,12,2,
        11,3,13,15,7,
    };
    // colors adapted from ex6.c
    float colors[] = {
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 0.0
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    vec3f v[180] = {0};
    vec3f n[180] = {0};
    vec3f t[180] = {0};
    vec3f c[180] = {0};
    if (mode == 1) {
        const int f = 108 * 15 / 9;
        int k = 0;
        for (int i = 0; i < 60; i += 5) {
            vec3f v1 = vec3fInArray(coords, faces2[i] * 3);
            vec3f v2 = vec3fInArray(coords, faces2[i + 1] * 3);
            vec3f v3 = vec3fInArray(coords, faces2[i + 2] * 3);
            vec3f v4 = vec3fInArray(coords, faces2[i + 3] * 3);
            vec3f v5 = vec3fInArray(coords, faces2[i + 4] * 3);
            vec3f normal = cross(sub(v2, v1), sub(v3, v1));
            vec3f center = scalef(add(add(add(v1, v2), add(v3, v4)), v5), 1.0 / 5.0);
            vec3f vecs[5] = {v1, v2, v3, v4, v5};
            for (int j = 0; j < 5; j++) {
                v[k] = vecs[j];
                c[k] = vec3fInArray(colors, i * 27 / 5);
                t[k] = vec3fInArray(faceTexture2, ((j) % 5) * 3);
                n[k++] = normal;
                c[k] = vec3fInArray(colors, i * 27 / 5);
                t[k] = vec3fInArray(faceTexture2, ((j + 1) % 5) * 3);
                v[k] = vecs[(j + 1) % 5];
                n[k++] = normal;
                c[k] = vec3fInArray(colors, i * 27 / 5);
                t[k] = vec3fInArray(faceTexture2, 5 * 3);
                v[k] = center;
                n[k++] = normal;
            }
        }

        glVertexPointer(3, GL_FLOAT, 0, v);
        glColorPointer(3, GL_FLOAT, 0, c);
        glNormalPointer(GL_FLOAT, 0, n);
        glTexCoordPointer(3, GL_FLOAT, 0, t);
        glShadeModel(GL_SMOOTH);
        glDrawArrays(GL_TRIANGLES, 0, f);
    } else {
        const int f = 108;
        float *ft = mode ? faceTexture3 : faceTexture;
        for (int i = 0; i < f; i+=3) {
            vec3f v1 = vec3fInArray(coords, faces[i] * 3);
            vec3f v2 = vec3fInArray(coords, faces[i + 1] * 3);
            vec3f v3 = vec3fInArray(coords, faces[i + 2] * 3);
            vec3f normal = cross(sub(v2, v1), sub(v3, v1));
            v[i] = v1;
            n[i] = normal;
            v[i + 1] = v2;
            n[i + 1] = normal;
            v[i + 2] = v3;
            n[i + 2] = normal;
            int j = i % 9;
            vec3f texCoords;
            texCoords = vec3fInArray(ft, j++ * 3);
            t[i] = texCoords;
            texCoords = vec3fInArray(ft, j++ * 3);
            t[i + 1] = texCoords;
            texCoords = vec3fInArray(ft, j++ * 3);
            t[i + 2] = texCoords;
        }

        glVertexPointer(3, GL_FLOAT, 0, v);
        glColorPointer(3, GL_FLOAT, 0, colors);
        glNormalPointer(GL_FLOAT, 0, n);
        glTexCoordPointer(3, GL_FLOAT, 0, t);
        glShadeModel(GL_SMOOTH);
        glDrawArrays(GL_TRIANGLES, 0, f);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void drawGrid() {
    cell *row = corner, *current;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glUniform1i(glGetUniformLocation(ID, "smplr"), 0);
    for (; row; row = row->nextZ) {
        current = row;
        for (; current; current = current->nextX) {
            glBindBuffer(GL_ARRAY_BUFFER, current->buffers[0]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glBindBuffer(GL_ARRAY_BUFFER, current->buffers[1]);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);

            glBindBuffer(GL_ARRAY_BUFFER, current->buffers[2]);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

            glDrawArrays(GL_TRIANGLES, 0, current->tris.size / 3);
        }
    }
    ErrCheck("drawGrid");
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void display() {
    shouldDisplay = 0;
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

    ErrCheck("display2");
    float *matrix = calloc(16, sizeof(float));
    glUseProgram(ID);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    glUniformMatrix4fv(glGetUniformLocation(ID, "modelview"), 1, GL_FALSE, matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    glUniformMatrix4fv(glGetUniformLocation(ID, "projection"), 1, GL_FALSE, matrix);
    ErrCheck("display1");
    free(matrix);
    // don't cull backfaces when looking around from afar (looks awkward)
    if (perspective > 1) {
        //glEnable(GL_CULL_FACE);
        vec3f lightPos = position;
        lightPos.y += 4;
        glUniform3fv(glGetUniformLocation(ID, "lightPos"), 1, (float *)&lightPos);
        drawGrid();
        glDisable(GL_CULL_FACE);
    }

    glUseProgram(0);
    
    // much of this part of the code comes from ex13.c
    float a = 0.2;
    float amb[4] = {a, a, a, 1};
    float d = 1;
    float dif[4] = {d, d, d, 1};
    float s = 0.5;
    float spc[4] = {s, s, s, 1};
    float white[4] = {1.0, 1.0, 1.0, 1.0};
    float t = oldT - lightOffset;
    vec3f posVec = {0};
    if (perspective > 1) {
        vec3f temp = {-position.x, position.y + 4, -position.z};
        posVec = temp;
    } else {
        vec3f temp = {cosf(t / 2000) * 300, 6, sinf(t / 2000) * 300};
        posVec = temp;
        glPushMatrix();
        glTranslatef(temp.x, temp.y, temp.z);
        glRotatef(cosf(t / 2000), 1, 0, 0);
        glRotatef(sinf(t / 3000), 0, 1, 1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, sunTexture);
        dodecahedron();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }
    float pos[4] = {posVec.x, posVec.y, posVec.z, 1};
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spc);

    if (perspective > 1) {
        // makes this roughly equal to the terrain, except the dodes are shinier
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.022);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0019);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    for (int i = 0; i < nDodes; i++) {
        glPushMatrix();
        glTranslatef(dodes[i].tX, 0, dodes[i].tZ);
        glScalef(dodes[i].scale, dodes[i].scale, dodes[i].scale);
        if (perspective < 2) {
            float tempPos[4] = {pos[0] - dodes[i].tX, pos[1], pos[2] - dodes[i].tZ, pos[3]};
            glLightfv(GL_LIGHT0, GL_POSITION, tempPos);
        }
        glRotatef(dodes[i].rX, 1, 0, 0);
        glRotatef(dodes[i].rY, 0, 1, 0);
        glRotatef(dodes[i].rZ, 0, 0, 1);
        dodecahedron();
        glPopMatrix();
    }
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glShadeModel(GL_FLAT);
    dodecahedron();
    glDisable(GL_TEXTURE_2D);

    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_NORMALIZE);
    //dodecahedron();

    glLoadIdentity();

    glWindowPos2i(5, 5);
    //Print(state[perspective]);
    ErrCheck("display");

    //glFlush();
    SDL_GL_SwapWindow(window);
}

void updateGrid(vec3f change) {
    if (change.x == 0 && change.z == 0) return;
    vec3f currentPos = cellPos(scalef(position, -1));
    cell *current = corner;
    cell *next;
    cell *nextRow = current->nextZ;
    cell *row;
    cell *prev;
    int dist;
    int lod;
    vec3f pos = scalef(position, -1);
    //printf("currently in %f %f\n", pos.x, pos.z);
    for (; current;) {
        for (; current; current = next) {
            next = current->nextX;
            //delCell(current);
            dist = roughCellDistance2(current, pos);
            if (current->lod < lods && dist <= lodCutoffs[current->lod]) {
                current->lod++;
                //printf("%f %f+; ", current->pos.x, current->pos.z);
                if (!current->needsRender || dist < 4) {
                    current->needsRender = 1;
                    cellQueue = enqueue(cellQueue, current, roughCellDistance(current, pos));
                }
            } else if (dist > (lodCutoffs[current->lod]) * 1.5) {
                current->lod--;
                if (current->lod >= 0) {
                    //printf("%f %f-; ", current->pos.x, current->pos.z);
                    if (!current->needsRender) {
                        current->needsRender = 1;
                        cellQueue = enqueue(cellQueue, current, roughCellDistance(current, pos));
                    }
                } else {
                    //printf("del %f %f; ", current->pos.x, current->pos.z);
                    if (current == corner) {
                        if (current->nextX) corner = current->nextX;
                        else corner = nextRow;
                    }
                    cellQueue = dequeue(cellQueue, current);
                    delCell(current);
                }
            }
        }
        prev = current;
        current = nextRow;
        if (current) {
            nextRow = current->nextZ;
        }
    }
    //printf("\n");
    //if (change.x) {
        //int sign = (change.x < 0) ? -1 : 1;
        //if (chang)
    //}
    if (change.x < 0) {
        current = corner;
        if (roughCellDistance2(current, position) < lodCutoffs[0]) {
            for (int i = 0; i > change.x; i--) {
                row = NULL;
                prev = NULL;
                nextRow = NULL;
                for (; current; current = next) {
                    pos = current->pos;
                    pos.x -= cellSize.x;
                    next = createCell(pos, cellSize);
                    if (!nextRow) nextRow = next;
                    if (prev) {
                        prev->nextZ = current;
                        current->prevZ = prev;
                    }
                    current->prevX = next;
                    next->nextX = current;
                    dist = roughCellDistance(current, position);
                    for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
                    current->lod = lod;
                    cellQueue = enqueue(cellQueue, current, dist);
                    prev = next;
                    next = current->nextZ;
                }
                current = nextRow;
            }
            corner = current;
        }
    } else if (change.x > 0) {
        current = corner;
        for (; current; current = current->nextX) prev = current;
        current = prev;
        if (roughCellDistance2(current, position) < lodCutoffs[0]) {
            // different from above
            for (int i = 0; i < change.x; i++) {
                row = NULL;
                prev = NULL;
                nextRow = NULL;
                for (; current; current = next) {
                    pos = current->pos;
                    // different from above
                    pos.x += cellSize.x;
                    next = createCell(pos, cellSize);
                    if (!nextRow) nextRow = next;
                    if (prev) {
                        prev->nextZ = current;
                        current->prevZ = prev;
                    }
                    // different from above
                    current->nextX = next;
                    next->prevX = current;
                    dist = roughCellDistance(current, position);
                    for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
                    current->lod = lod;
                    cellQueue = enqueue(cellQueue, current, dist);
                    prev = next;
                    next = current->nextZ;
                }
                current = nextRow;
            }
        }
    }
    if (change.z < 0) {
        current = corner;
        if (roughCellDistance2(current, position) < lodCutoffs[0]) {
            for (int i = 0; i > change.z; i--) {
                row = NULL;
                prev = NULL;
                nextRow = NULL;
                for (; current; current = next) {
                    pos = current->pos;
                    pos.z -= cellSize.z;
                    next = createCell(pos, cellSize);
                    if (!nextRow) nextRow = next;
                    if (prev) {
                        prev->nextX = current;
                        current->prevX = prev;
                    }
                    current->prevZ = next;
                    next->nextZ = current;
                    dist = roughCellDistance(current, position);
                    for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
                    current->lod = lod;
                    cellQueue = enqueue(cellQueue, current, dist);
                    prev = next;
                    next = current->nextX;
                }
                current = nextRow;
            }
            corner = current;
        }
    } else if (change.z > 0) {
        current = corner;
        for (; current; current = current->nextZ) prev = current;
        current = prev;
        if (roughCellDistance2(current, position) < lodCutoffs[0]) {
            // different from above
            for (int i = 0; i < change.z; i++) {
                row = NULL;
                prev = NULL;
                nextRow = NULL;
                for (; current; current = next) {
                    pos = current->pos;
                    // different from above
                    pos.z += cellSize.z;
                    next = createCell(pos, cellSize);
                    if (!nextRow) nextRow = next;
                    if (prev) {
                        prev->nextX = current;
                        current->prevX = prev;
                    }
                    // different from above
                    current->nextZ = next;
                    next->prevZ = current;
                    dist = roughCellDistance(current, position);
                    for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
                    current->lod = lod;
                    cellQueue = enqueue(cellQueue, current, dist);
                    prev = next;
                    next = current->nextZ;
                }
                current = nextRow;
            }
        }
    }//*/
}

void idle() {
    int time = SDL_GetTicks();
    int shouldRedisplay = 1;
    // note: changed this from a while loop to an if statement
    // so if the physics take too long to calculate (they shouldn't, but just in case)
    // it won't start running super behind
    // however, this means that the physics are max 60 fps
    if (oldT < time) {
        switch (keyDown) {
            case 'w':
            case 's':
                acceleration.x -= sin(theta * PI / 180) / 50 * (keyDown == 'w' ? 1 : -1);
                acceleration.z -= cos(theta * PI / 180) / 50 * (keyDown == 'w' ? 1 : -1);
                break;
            case 'a':
            case 'd':
                acceleration.x -= cos(theta * PI / 180) / 50 * (keyDown == 'a' ? 1 : -1);
                acceleration.z += sin(theta * PI / 180) / 50 * (keyDown == 'a' ? 1 : -1);
                break;
            default:
                shouldRedisplay = 0;
                break;
        }
        //position.x = -position.x;
        //position.z = -position.z;
        vec3f oldPos = position;
        position = scalef(position, -1);
        if (perspective == 3) {
            vec3f pos = {position.x, position.y - 2, position.z};
            if (terrain.influenceAt(pos) < terrain.threshhold) {
                acceleration.y += 0.1;
            }
            shouldRedisplay = 1;
        }
        acceleration = scalef(acceleration, 0.2);
        acceleration.y *= 0.5;
        velocity = add(velocity, scalef(acceleration, 20));
        velocity = scalef(velocity, onFloor ? 0.95 : 0.98);
        if (perspective == 3) {
            vec3f pos = add(position, scalef(velocity, 1 / 20.0));
            float grav = velocity.y;
            pos.y = position.y;
            int n = 0;
            int rVelocity = 0;
            pos.y -= 4;
            while (terrain.influenceAt(pos) > terrain.threshhold && n++ < 2) {
                velocity = scalef(velocity, 0.5);
                pos = sub(pos, scalef(velocity, 1 / 20.0));
            }
            pos.y += 4;
            if (n == 3) {
                rVelocity = 1;
                pos = position;
                velocity.y = grav;
            } else {
            }
            pos.y += grav / 20.0 - 2;
            n = 0;
            double inf;
            if (terrain.influenceAt(pos) > terrain.threshhold) {
                pos.y -= grav / 20.0;
                velocity.y = 0;
                onFloor = 1;
            }
            while ((inf = terrain.influenceAt(pos)) > terrain.threshhold && n++ < 15) {
                pos.y += terrain.threshhold - inf - 0.01;
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
        if (lightStopped) {
            lightOffset += 1000 / 60.0;
        }
        position = scalef(position, -1);
        updateGrid(sub(cellPos(oldPos), cellPos(position)));
    }
    cell *toRender = peek(cellQueue);
    if (toRender) {
        //if (cellQueue->priority < 4) printf("currently %f, %f\n", toRender->pos.x, toRender->pos.z);
        cellQueue = pop(cellQueue);
        // avoid re-rendering cells we don't need
        if (toRender->needsRender == 1) {
            if (toRender->tris.size > 0) {
                delArr(toRender->tris);
                delArr(toRender->normals);
                delArr(toRender->textureCoords);
                toRender->tris = createArr();
                toRender->normals = createArr();
                toRender->textureCoords = createArr();
            }
            calculateTris(toRender, terrain);
            shouldRedisplay = 1;
        }
    }
    if (shouldRedisplay) {
        setProjection();
        glutPostRedisplay();
    }
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

int main() {
    SDL_DisplayMode mode;
    // get this to hopefully work well under wayland
    if (SDL_Init( SDL_INIT_EVERYTHING ) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_GetCurrentDisplayMode(0, &mode) < 0) {
        printf("oops\n");
        return 1;
    }

    rate = mode.refresh_rate;
    width = mode.w / 2;
    height = mode.h / 2;

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("Isaac Mitchell", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window) {
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    reshape(width, height);

    srand(time(NULL));
    oldT = SDL_GetTicks();
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
    groundTexture = LoadTexBMP("ground.bmp", GL_NEAREST);
    crateTexture = LoadTexBMP("crate.bmp", GL_LINEAR);
    sunTexture = LoadTexBMP("sun.bmp", GL_LINEAR);
    textures[0] = crateTexture;
    textures[1] = sunTexture;
    textures[2] = groundTexture;
    createGrid(0, 0);
    //terrain = sineHill;
    terrain = perlin;
    setupPerlin();
    calculateTris(corner, terrain);

    //system("pause");

    while (1) {
        processEvents();
        if (shouldDisplay) display();
        idle();
        SDL_Delay(1000 / rate);
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}