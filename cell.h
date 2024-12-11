#pragma once
#include "arr.h"
#include "funcs.h"
#include "vec3.h"
#include "tree.h"

typedef struct cell {
    vec3f pos;
    vec3f size;
    int lod;
    int needsRender;
    growArr tris;
    growArr normals;
    growArr textureCoords;
    unsigned int buffers[3];
    struct cell *nextX;
    struct cell *nextZ;
    struct cell *prevX;
    struct cell *prevZ;
    tree *t;
} cell;

void calculateTris(cell *ptr, funcDef f);
float cellDistance(cell *ptr, vec3f pos);
int roughCellDistance(cell *ptr, vec3f pos);
int roughCellDistance2(cell *ptr, vec3f pos);
int roughCellDistance3(cell *ptr, vec3f pos, int isX);
void delCell(cell *ptr);
cell *createCell(vec3f pos, vec3f size);
vec3f cellPos(vec3f pos);