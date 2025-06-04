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
#include "text.h"
//#include "ccVector.h"
#include "tree.h"

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
unsigned int skyboxID;
unsigned int waterID;
unsigned int treeID;

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
float fps = 60;
int lightOffset = 0;
int lightStopped = 0;

// storing all of the necessary matrices for when we need to change them out
float *modelView;
float *projection;
float *screenSpace;

unsigned int refractionBuffer;
unsigned int reflectionBuffer;
unsigned int refractionTexture;
unsigned int reflectionTexture;
unsigned int refractionDepth;
unsigned int reflectionDepth;

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
char skyboxFiles[][32] = {
    "skybox/Daylight Box_Right.bmp",
    "skybox/Daylight Box_Left.bmp",
    "skybox/Daylight Box_Bottom.bmp",
    "skybox/Daylight Box_Top.bmp",
    "skybox/Daylight Box_Front.bmp",
    "skybox/Daylight Box_Back.bmp"
};

int groundTexture;
int snowTexture;
int crateTexture;
int sunTexture;
int skyboxTexture;
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
        double bottom = tan(60 * 3.14159/360.0) * 0.1;
        glFrustum(-asp * bottom, asp * bottom, -bottom, bottom, 0.1, renderDistance * 16 * 1.1);
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
    glPushMatrix();
    glOrtho(0, w, 0, h, -1, 1);
    glGetFloatv(GL_MODELVIEW_MATRIX, screenSpace);
    glPopMatrix();
    setProjection();
}

void mouse(int button, int up, int x, int y) {
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
        case 27:
            if (hasFocus) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                hasFocus = 0;
                break;
            }
            // fall through
        case 'q':
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
            if (onFloor && position.y > -5) {
                velocity.y -= 5;
                onFloor = 0;
            } else if (position.y < -3) {
                if (position.y < -5) {
                    acceleration.y -= 0.5;
                    velocity.y -= 0.4;
                } else {
                    acceleration.y -= (position.y + 5) / 4;
                    velocity.y -= 0.4;
                }
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
            case SDL_KEYDOWN:
                key(event.key.keysym.sym, 0, 0);
                break;
            case SDL_KEYUP:
                keyUp(event.key.keysym.sym, 0, 0);
        }
    }
}

void drawGrid() {
    cell *row = corner, *current;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, snowTexture);
    glUniform1i(glGetUniformLocation(ID, "groundSampler"), 0);
    glUniform1i(glGetUniformLocation(ID, "snowSampler"), 1);
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

void drawSkybox() {
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxID);
    float rot2[16];
    glPushMatrix();
    glLoadIdentity();
    double bottom = tan(60 * 3.14159/360.0) * 0.01;
    glFrustum(-asp * bottom, asp * bottom, -bottom, bottom, 0.01, 4);
    glRotated(phi, -1, 0, 0);
    glRotated(theta, 0, -1, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, rot2);
    glPopMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxID, "projection"), 1, GL_FALSE, rot2);
    glUniformMatrix4fv(glGetUniformLocation(skyboxID, "modelview"), 1, GL_FALSE, modelView);
    ErrCheck("skybox-start");
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glUniform1i(glGetUniformLocation(skyboxID, "skybox"), 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, skyboxVertices);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDisableClientState(GL_VERTEX_ARRAY);
    glUseProgram(0);
    ErrCheck("skybox-end");
}

void drawTree();

void drawWater() {
    int aboveWater = position.y > -5 ? 1 : -1;
    glEnable(GL_CLIP_DISTANCE0);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glViewport(0, 0, 1024, 1024);
    glUseProgram(ID);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glUniformMatrix4fv(glGetUniformLocation(ID, "modelview"), 1, GL_FALSE, modelView);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // flip the camera for the reflection
    double bottom = tan(60 * 3.14159/360.0) * 0.01;
    glFrustum(-asp * bottom, asp * bottom, -bottom, bottom, 0.01, renderDistance * 16 * 1.1);
    glRotated(-phi, -1, 0, 0);
    glRotated(theta, 0, -1, 0);
    glTranslatef(position.x, position.y + 6, position.z);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glUniformMatrix4fv(glGetUniformLocation(ID, "projection"), 1, GL_FALSE, projection);
    glMatrixMode(GL_MODELVIEW);
    glUniform4f(glGetUniformLocation(ID, "plane"), 0, aboveWater, 0, aboveWater == 1 ? 1 : 0.5);
    vec3f lightPos = position;
    lightPos.y += 4;
    glUniform3fv(glGetUniformLocation(ID, "lightPos"), 1, (float *)&lightPos);
    drawGrid();
    glDisable(GL_CULL_FACE);
    phi = -phi;
    drawSkybox();
    phi = -phi;
    drawTree();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glBindFramebuffer(GL_FRAMEBUFFER, refractionBuffer);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glViewport(0, 0, 1024, 1024);
    glUseProgram(ID);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glUniformMatrix4fv(glGetUniformLocation(ID, "modelview"), 1, GL_FALSE, modelView);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glUniformMatrix4fv(glGetUniformLocation(ID, "projection"), 1, GL_FALSE, projection);
    glUniform4f(glGetUniformLocation(ID, "plane"), 0, -aboveWater, 0, aboveWater == 1 ? 0.5 : 1.5);
    glUniform3fv(glGetUniformLocation(ID, "lightPos"), 1, (float *)&lightPos);
    drawGrid();
    glDisable(GL_CULL_FACE);
    drawSkybox();
    drawTree();
    glDisable(GL_CLIP_DISTANCE0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    reshape(width, height);
    glUseProgram(waterID);
    float rot2[16];
    glPushMatrix();
    glLoadIdentity();
    glFrustum(-asp * bottom, asp * bottom, -bottom, bottom, 0.01, 4);
    glRotated(phi, -1, 0, 0);
    glRotated(theta, 0, -1, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, rot2);
    glPopMatrix();
    glUniformMatrix4fv(glGetUniformLocation(waterID, "projection"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(waterID, "view"), 1, GL_FALSE, modelView);
    glUniform3fv(glGetUniformLocation(waterID, "camera"), 1, (float *)&position);
    glUniform3fv(glGetUniformLocation(waterID, "light"), 1, (float *)&position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, refractionTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glUniform1i(glGetUniformLocation(waterID, "reflectionTexture"), 0);
    glUniform1i(glGetUniformLocation(waterID, "refractionTexture"), 1);
    ErrCheck("water-start");
    glUniform1f(glGetUniformLocation(waterID, "time"), SDL_GetTicks());
    int i = renderDistance * 16 * 1.5;
    float waterVertices[] = {
        -position.x-i, -1, -position.z-i,
        -position.x-i, -1, -position.z+i,
        -position.x+i, -1, -position.z-i,
        -position.x+i, -1, -position.z-i,
        -position.x-i, -1, -position.z+i,
        -position.x+i, -1, -position.z+i
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, waterVertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableClientState(GL_VERTEX_ARRAY);
    glUseProgram(0);
    ErrCheck("water-end");
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void drawTree() {
    cell *row = corner, *current;
    glUseProgram(treeID);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glUniformMatrix4fv(glGetUniformLocation(treeID, "modelview"), 1, GL_FALSE, modelView);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glUniformMatrix4fv(glGetUniformLocation(treeID, "projection"), 1, GL_FALSE, projection);
    glUniform4f(glGetUniformLocation(treeID, "plane"), 0, 0, 0, 0);
    vec3f lightPos = position;
    lightPos.y += 4;
    glUniform3fv(glGetUniformLocation(treeID, "lightPos"), 1, (float *)&lightPos);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    for (; row; row = row->nextZ) {
        current = row;
        for (; current; current = current->nextX) {
            if (current->t == NULL) continue;
            glBindBuffer(GL_ARRAY_BUFFER, current->t->tree->buffers[0]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glBindBuffer(GL_ARRAY_BUFFER, current->t->tree->buffers[1]);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);

            glBindBuffer(GL_ARRAY_BUFFER, current->t->tree->buffers[2]);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

            glDrawArrays(GL_TRIANGLES, 0, current->t->tree->vertices.size / 3);
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
    glUseProgram(ID);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glUniformMatrix4fv(glGetUniformLocation(ID, "modelview"), 1, GL_FALSE, modelView);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    glUniformMatrix4fv(glGetUniformLocation(ID, "projection"), 1, GL_FALSE, projection);
    glUniform4f(glGetUniformLocation(ID, "plane"), 0, 0, 0, 0);
    ErrCheck("display1");
    vec3f lightPos = position;
    lightPos.y += 4;
    glUniform3fv(glGetUniformLocation(ID, "lightPos"), 1, (float *)&lightPos);
    drawGrid();
    drawWater();

    glUseProgram(0);

    drawTree();
    
    drawSkybox();

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(screenSpace);

    glWindowPos2i(5, 5);
    glColor4f(1, 1, 1, 1);
    print2D(5, 5, state[perspective], 20);
    char posPrint[50];
    vec3f posVec = {-position.x, position.y + 4, -position.z};
    sprintf(posPrint, "x: %i, y: %i, z: %i", (int)posVec.x, (int)posVec.y, (int)posVec.z);
    print2D(5, 30, posPrint, 20);
    sprintf(posPrint, "FPS: %f", fps);
    print2D(5, 55, posPrint, 20);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    ErrCheck("display");

    SDL_GL_SwapWindow(window);
}

void updateGrid(vec3f change) {
    if (change.x == 0 && change.z == 0) return;
    cell *current = corner;
    cell *next;
    cell *nextRow = current->nextZ;
    cell *prev;
    int dist;
    int lod;
    vec3f pos = scalef(position, -1);
    for (; current;) {
        for (; current; current = next) {
            next = current->nextX;
            dist = roughCellDistance2(current, pos);
            if (current->lod < lods && dist <= lodCutoffs[current->lod]) {
                current->lod++;
                if (!current->needsRender || dist < 4) {
                    current->needsRender = 1;
                    cellQueue = enqueue(cellQueue, current, roughCellDistance(current, pos));
                }
            } else if (dist > (lodCutoffs[current->lod]) * 2) {
                current->lod--;
                if (current->lod >= 0) {
                    if (!current->needsRender) {
                        current->needsRender = 1;
                        cellQueue = enqueue(cellQueue, current, roughCellDistance(current, pos));
                    }
                } else {
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
    vec3f realPos = pos;
    // this part is kind of a mess of variable names so i'm documenting it here
    // pos gets changed from the negated player position to the position of the cell that we're creating
    // current isn't the current cell, it's the cell that has already been created in the previous row / column that we're using to get to the cell to be created
    // next is the cell we're creating, and prev is the cell we most recently created (prev is just used for setting up the doubly linked listiness of the grid)
    #define isqrt(x) (int)sqrtf((float)x)
    if (change.x < 0) {
        current = corner;
        if (roughCellDistance3(current, realPos, 1) < lodCutoffs[0]) {
            for (int i = 0; i > change.x; i--) {
                prev = NULL;
                nextRow = NULL;
                for (; current; current = next) {
                    pos = current->pos;
                    pos.x -= cellSize.x;
                    next = createCell(pos, cellSize);
                    if (!nextRow) nextRow = next;
                    if (prev) {
                        prev->nextZ = next;
                        next->prevZ = prev;
                    }
                    current->prevX = next;
                    next->nextX = current;
                    dist = roughCellDistance(next, realPos);
                    for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
                    next->lod = lod;
                    calculateTris(next, terrain);
                    vec3f treeOffset = {rand() % (int)next->size.x, 0, rand() % (int)next->size.z};
                    vec3f treePos = add(next->pos, treeOffset);
                    treePos.y = 0;
                    float inf = terrain.influenceAt(treePos);
                    if (inf > 0 && (int)(inf * 37) & 1) {
                        treePos.y += inf - 0.5;
                        next->t = genTree(treePos, 0);
                    }
                    cellQueue = enqueue(cellQueue, next, isqrt(dist));
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
        if (roughCellDistance3(current, realPos, 1) < lodCutoffs[0]) {
            // different from above
            for (int i = 0; i < change.x; i++) {
                prev = NULL;
                nextRow = NULL;
                for (; current; current = next) {
                    pos = current->pos;
                    // different from above
                    pos.x += cellSize.x;
                    next = createCell(pos, cellSize);
                    if (!nextRow) nextRow = next;
                    if (prev) {
                        prev->nextZ = next;
                        next->prevZ = prev;
                    }
                    // different from above
                    current->nextX = next;
                    next->prevX = current;
                    dist = roughCellDistance(next, realPos);
                    for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
                    next->lod = lod;
                    calculateTris(next, terrain);
                    vec3f treeOffset = {rand() % (int)next->size.x, 0, rand() % (int)next->size.z};
                    vec3f treePos = add(next->pos, treeOffset);
                    treePos.y = 0;
                    float inf = terrain.influenceAt(treePos);
                    if (inf > 0 && (int)(inf * 37) & 1) {
                        treePos.y += inf - 0.5;
                        next->t = genTree(treePos, 0);
                    }
                    cellQueue = enqueue(cellQueue, next, isqrt(dist));
                    prev = next;
                    next = current->nextZ;
                }
                current = nextRow;
            }
        }
    }
    if (change.z < 0) {
        current = corner;
        if (roughCellDistance3(current, realPos, 0) < lodCutoffs[0]) {
            for (int i = 0; i > change.z; i--) {
                prev = NULL;
                nextRow = NULL;
                for (; current; current = next) {
                    pos = current->pos;
                    pos.z -= cellSize.z;
                    next = createCell(pos, cellSize);
                    if (!nextRow) nextRow = next;
                    if (prev) {
                        prev->nextX = next;
                        next->prevX = prev;
                    }
                    current->prevZ = next;
                    next->nextZ = current;
                    dist = roughCellDistance(next, realPos);
                    for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
                    next->lod = lod;
                    calculateTris(next, terrain);
                    vec3f treeOffset = {rand() % (int)next->size.x, 0, rand() % (int)next->size.z};
                    vec3f treePos = add(next->pos, treeOffset);
                    treePos.y = 0;
                    float inf = terrain.influenceAt(treePos);
                    if (inf > 0 && (int)(inf * 37) & 1) {
                        treePos.y += inf - 0.5;
                        next->t = genTree(treePos, 0);
                    }
                    cellQueue = enqueue(cellQueue, next, isqrt(dist));
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
        if (roughCellDistance3(current, realPos, 0) < lodCutoffs[0]) {
            // different from above
            for (int i = 0; i < change.z; i++) {
                prev = NULL;
                nextRow = NULL;
                for (; current; current = next) {
                    pos = current->pos;
                    // different from above
                    pos.z += cellSize.z;
                    next = createCell(pos, cellSize);
                    if (!nextRow) nextRow = next;
                    if (prev) {
                        prev->nextX = next;
                        next->prevX = prev;
                    }
                    // different from above
                    current->nextZ = next;
                    next->prevZ = current;
                    dist = roughCellDistance(next, realPos);
                    for (lod = 0; lod < 3 && lodCutoffs[lod] >= dist; lod++);
                    next->lod = lod;
                    calculateTris(next, terrain);
                    vec3f treeOffset = {rand() % (int)next->size.x, 0, rand() % (int)next->size.z};
                    vec3f treePos = add(next->pos, treeOffset);
                    treePos.y = 0;
                    float inf = terrain.influenceAt(treePos);
                    if (inf > 0 && (int)(inf * 37) & 1) {
                        treePos.y += inf - 0.5;
                        next->t = genTree(treePos, 0);
                    }
                    cellQueue = enqueue(cellQueue, next, isqrt(dist));
                    prev = next;
                    next = current->nextX;
                }
                current = nextRow;
            }
        }
    }
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
                if (position.y > 5) {
                    acceleration.y += 0.05;
                } else {
                    acceleration.y += 0.1;
                }
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
        fps = (59 * fps + (1000 / ((1000.0 / 60.0) + time - oldT))) / 60;
        oldT += 1000 / 60.0;
        if (lightStopped) {
            lightOffset += 1000 / 60.0;
        }
        position = scalef(position, -1);
        updateGrid(sub(cellPos(oldPos), cellPos(position)));
    }
    cell *toRender = peek(cellQueue);
    if (toRender) {
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
            if (toRender->t) {
                vec3f treePos = toRender->t->pos;
                deleteTree(toRender->t);
                toRender->t = genTree(treePos, toRender->lod + 1);
            }
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
            calculateTris(current, terrain);
            vec3f treeOffset = {rand() % (int)current->size.x, 0, rand() % (int)current->size.z};
            vec3f treePos = add(current->pos, treeOffset);
            treePos.y = 0;
            float inf = terrain.influenceAt(treePos);
            if (inf > 0 && (int)(inf * 37) & 1) {
                treePos.y += inf - 0.5;
                current->t = genTree(treePos, 0);
            }
            cellQueue = enqueue(cellQueue, current, isqrt(dist));
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

    SDL_GL_CreateContext(window);
    modelView = calloc(16, sizeof(float));
    projection = calloc(16, sizeof(float));
    screenSpace = calloc(16, sizeof(float));
    reshape(width, height);

    //srand(time(NULL));
    srand(0);
    oldT = SDL_GetTicks();
    dimInit = dim;
    ID = createProgram("vert.vert", "frag.frag", 1);
    skyboxID = createProgram("skybox.vert", "skybox.frag", 1);
    waterID = createProgram("water.vert", "water.frag", 1);
    treeID = createProgram("tree.vert", "tree.frag", 1);
    for (int i = 0; i < nDodes; i++) {
        dodes[i].rX = rand() % 360;
        dodes[i].rY = rand() % 360;
        dodes[i].rZ = rand() % 360;
        dodes[i].scale = rand() % 20 + 10;
        dodes[i].tX = (rand() % 400) - 200;
        dodes[i].tZ = (rand() % 400) - 200;
    }
    groundTexture = LoadTexBMP("ground.bmp", GL_NEAREST);
    snowTexture = LoadTexBMP("snow.bmp", GL_LINEAR);
    char *skyboxFilePointers[6];
    for (int i = 0; i < 6; i++) {
        skyboxFilePointers[i] = skyboxFiles[i];
    }
    skyboxTexture = LoadCubemapBMP(skyboxFilePointers, GL_LINEAR);
    textures[0] = crateTexture;
    textures[1] = sunTexture;
    textures[2] = groundTexture;
    // for water
    glGenFramebuffers(1, &refractionBuffer);
    glGenTextures(1, &refractionTexture);
    glGenRenderbuffers(1, &refractionDepth);
    glBindFramebuffer(GL_FRAMEBUFFER, refractionBuffer);
    glBindTexture(GL_TEXTURE_2D, refractionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractionTexture, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, refractionDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, refractionDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("refraction buffer failed for some reason!\n");
    }
    glGenFramebuffers(1, &reflectionBuffer);
    glGenTextures(1, &reflectionTexture);
    glGenRenderbuffers(1, &reflectionDepth);

    glBindFramebuffer(GL_FRAMEBUFFER, reflectionBuffer);

    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTexture, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, reflectionDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, reflectionDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("reflection buffer failed for some reason!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ErrCheck("setup");
    terrain = perlin;
    setupPerlin();
    createGrid(0, 0);
    pos2.y += terrain.influenceAt(pos2);
    textInit();
    calculateTris(corner, terrain);

    while (1) {
        processEvents();
        if (shouldDisplay) display();
        idle();
        SDL_Delay(1);
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}