#pragma once
#include "vec3.h"

typedef struct funcDef {
    float (*influenceAt)(vec3f);
    float threshhold;
} funcDef;

extern const funcDef sineHill;