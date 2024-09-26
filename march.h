#pragma once

#include "vec3.h"
#include "arr.h"
typedef struct marchData {
    float vals[8];
    vec3f pos;
    float size;
} marchData;

growArr march(marchData, growArr, float);