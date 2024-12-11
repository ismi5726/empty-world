#pragma once

#include "arr.h"
#include "objects.h"
#include "vec3.h"

typedef struct tree {
    object *tree;
    vec3f pos;
} tree;

typedef struct branch {
    vec3f start;
    vec3f dir;
    float length;
    float radius;
    float endRadius;
} branch;

tree *createTree();
void deleteTree(tree *t);
vec3f randInCircle(vec3f pos, vec3f dir, float scale, float *angle);
tree *genTree(vec3f pos, int level);
void branchOut(tree *t, branch start, int count, int depth, float angle, float bareLength, float lengthFactor);
void createBranch(tree *t, branch b, float radius, int isLeaf);