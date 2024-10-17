#pragma once

#include "vec3.h"
#include "arr.h"
#include "cell.h"
typedef struct marchData {
    float vals[8];
    vec3f pos;
    float size;
} marchData;

void march(marchData, cell*, funcDef);