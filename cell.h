#pragma once
#include "arr.h"
#include "funcs.h"
#include "vec3.h"

typedef struct cell {
    vec3f pos;
    vec3f size;
    int lod;
    int needsRender;
    growArr tris;
    growArr normals;
    unsigned int buffers[2];
    struct cell *nextX;
    struct cell *nextZ;
    struct cell *prevX;
    struct cell *prevZ;
} cell;

void calculateTris(cell *ptr, funcDef f);
float cellDistance(cell *ptr, vec3f pos);
int roughCellDistance(cell *ptr, vec3f pos);
void delCell(cell *ptr);
cell *createCell(vec3f pos, vec3f size);