//#include "ball.h"
#include "vec3.h"
#include "funcs.h"
#include "perlin.h"
#include <math.h>

float influenceAt(float x, float y, float z) {
    //
    return 0;
}

float sineHillFunc(vec3f pos) {
    return pos.y - 4 * (sinf(pos.x / 10) + sinf(pos.z / 10));
}

float perlinFunc(vec3f pos) {
    return pos.y - 64 * perlin2d(pos.x, pos.z, 1.0f / 256, 4);
}

const funcDef sineHill = {sineHillFunc, 0};
const funcDef perlin = {perlinFunc, 0};