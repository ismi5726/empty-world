//#include "ball.h"
#include "vec3.h"
#include "funcs.h"
#include "perlin.h"
#include <math.h>

float sineHillFunc(vec3f pos) {
    return pos.y - 4 * (sinf(pos.x / 10) + sinf(pos.z / 10));
}

float perlinFunc(vec3f pos) {
    return pos.y - 64 * perlin2d(pos.x + 1000, pos.z - 1000, 1.0f / 2000, 2) * perlin2d(pos.x, pos.z, 1.0f / 20, 4) - 2000 * perlin2d(pos.x - 250, pos.z + 3000, 1.0f / 1500, 3) * perlin2d(pos.x, pos.z, 1.0f / 2000, 1) + 20;
    //return pos.y - 64 * perlin2d(pos.x, pos.z, 1.0f / 2000, 2) * perlin2d(pos.x, pos.z, 1.0f / 20, 4) - 200 * perlin2d(pos.x, pos.z, 1.0f / 1500, 3);
    //return pos.y - 16 * perlin2d(pos.x, pos.z, 1.0f / 8, 4);
}

const funcDef sineHill = {sineHillFunc, 0};
const funcDef perlin = {perlinFunc, 0};