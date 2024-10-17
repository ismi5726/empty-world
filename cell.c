#include "march.h"
#include "arr.h"
#include "cell.h"
#include "funcs.h"
#include "vec3.h"
//#include "loadtexbmp.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif

void calculateTris(cell *ptr, funcDef f) {
    //cell c = *ptr;
    float step = 4.0 / (1 << ptr->lod);
    vec3f areasize = scalef(ptr->size, 1 / step);
    areasize.x++;areasize.y++;areasize.z++;
    int arrSize = areasize.x * areasize.y * areasize.z;
    float *influence = calloc(arrSize, sizeof(float));
    int n = 0;
    vec3f pos;
    for (float z = ptr->pos.z; z <= ptr->pos.z + ptr->size.z; z += step) {
        pos.z = z;
        for (float y = ptr->pos.y; y <= ptr->pos.y + ptr->size.y; y += step) {
            pos.y = y;
            for (float x = ptr->pos.x; x <= ptr->pos.x + ptr->size.x; x += step) {
                pos.x = x;
                if (n >= arrSize) {
                    printf("warning: influence array not big enough; could not calculate tris!");
                    return;
                }
                influence[n++] = f.influenceAt(pos);
            }
            //if (n >= arrSize) break;
        }
        //if (n >= arrSize) break;
    }

    n = 0;
    for (float z = ptr->pos.z; z < ptr->pos.z + ptr->size.z; z += step) {
        pos.z = z;
        for (float y = ptr->pos.y; y < ptr->pos.y + ptr->size.y; y += step) {
            pos.y = y;
            for (float x = ptr->pos.x; x < ptr->pos.x + ptr->size.x; x += step) {
                pos.x = x;
                marchData cellData = {{influence[n], influence[n + 1],
                    influence[n + 1 + (int)areasize.x], influence[n + (int)areasize.x],
                    influence[n + (int)(areasize.x * areasize.y)], influence[n + 1 + (int)(areasize.x * areasize.y)],
                    influence[n + 1 + (int)areasize.x + (int)(areasize.x * areasize.y)], influence[n + (int)areasize.x + (int)(areasize.x * areasize.y)],
                }, pos, (float)step};
                n++;
                march(cellData, ptr, f);
            }
            n++;
        }
        n += areasize.x;
    }

    glBindBuffer(GL_ARRAY_BUFFER, ptr->buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, ptr->tris.size * sizeof(float), ptr->tris.data, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ptr->buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, ptr->normals.size * sizeof(float), ptr->normals.data, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ptr->buffers[2]);
    glBufferData(GL_ARRAY_BUFFER, ptr->textureCoords.size * sizeof(float), ptr->textureCoords.data, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //ErrCheck("calculateTris");

    ptr->needsRender = 0;
}

float cellDistance(cell *ptr, vec3f pos) {
    cell c = *ptr;
    vec3f cellClosestPos = pos;
    if (pos.x < c.pos.x) {
        cellClosestPos.x = c.pos.x;
    } else if (pos.x > c.pos.x + c.size.x) {
        cellClosestPos.x = c.pos.x + c.size.x;
    }
    if (pos.y < c.pos.y) {
        cellClosestPos.y = c.pos.y;
    } else if (pos.y > c.pos.y + c.size.y) {
        cellClosestPos.y = c.pos.y + c.size.y;
    }
    if (pos.z < c.pos.z) {
        cellClosestPos.z = c.pos.z;
    } else if (pos.z > c.pos.z + c.size.z) {
        cellClosestPos.z = c.pos.z + c.size.z;
    }
    vec3f diff = sub(cellClosestPos, pos);
    return sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

int roughCellDistance(cell *ptr, vec3f pos) {
    vec3f tot = scalev3f(vAbs(sub(ptr->pos, pos)), inv(ptr->size));
    return (int)(tot.x + tot.y + tot.z);
}

vec3f cellPos(vec3f pos) {
    pos = scalef(pos, 1.0 / 16);
    pos.x = (int)pos.x; pos.y = (int)pos.y; pos.z = (int)pos.z;
    return pos;
}

void delCell(cell *ptr) {
    cell c = *ptr;
    delArr(c.tris);
    if (c.nextX) {
        c.nextX->prevX = NULL;
    }
    if (c.prevX) {
        c.prevX->nextX = NULL;
    }
    if (c.nextZ) {
        c.nextZ->prevZ = NULL;
    }
    if (c.prevZ) {
        c.prevZ->nextZ = NULL;
    }
    glDeleteBuffers(2, ptr->buffers);
    free(ptr);
}

cell *createCell(vec3f pos, vec3f size) {
    //cell c = {{0, 0, 0}, {0, 0, 0}, 0, {0, 0, 0}, 0, 0, 0, 0};
    /*cell c = {};
    c.tris = createArr();
    c.pos = pos;
    c.size = size;*/
    cell *ptr;
    ptr = (cell *)calloc(1, sizeof(cell));

    //cell c = *ptr;
    ptr->tris = createArr();
    ptr->normals = createArr();
    ptr->textureCoords = createArr();
    ptr->pos = pos;
    ptr->size = size;
    ptr->needsRender = 1;
    glGenBuffers(3, ptr->buffers);
    return ptr;
}