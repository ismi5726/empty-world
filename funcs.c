//#include "ball.h"
#include "vec3.h"
#include "funcs.h"
#include <math.h>

float influenceAt(float x, float y, float z) {
    //
    return 0;
}

float sineHillFunc(vec3f pos) {
    return pos.y - 4 * (sinf(pos.x / 10) + sinf(pos.z / 10));
}

const funcDef sineHill = {sineHillFunc, 0};