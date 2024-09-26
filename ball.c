#include "ball.h"
#include "vec3.h"
#include <math.h>

float influenceAt(vec3f pos, ball *balls, int num) {
    float influence = 0;
    for (int i = 0; i < num; i++) {
        ball c = balls[i];
        influence += c.influence / sqrtf((pos.x - c.pos.x) * (pos.x - c.pos.x) + (pos.y - c.pos.y) * (pos.y - c.pos.y) + (pos.z - c.pos.z) * (pos.z - c.pos.z));
    }
    return influence / num;
}

void maxArea(ball *balls, int num, vec3f *maxP, vec3f *minP) {
    vec3f max = {-100, -100, -100};
    vec3f min = {100, 100, 100};
    vec3f fudge = {1, 1, 1};
    for (int i = 0; i < num; i++) {
        ball c = balls[i];
        //float inf = c.influence * c.influence;
        float inf = c.influence < 0 ? -c.influence : c.influence;
        vec3f influenceVec = {inf, inf, inf};
        vec3f posMinus = sub(c.pos, influenceVec);
        vec3f posPlus = add(c.pos, influenceVec);
        if (posMinus.x < min.x) min.x = posMinus.x;
        if (posMinus.y < min.y) min.y = posMinus.y;
        if (posMinus.z < min.z) min.z = posMinus.z;
        if (posPlus.x > max.x) max.x = posPlus.x;
        if (posPlus.y > max.y) max.y = posPlus.y;
        if (posPlus.z > max.z) max.z = posPlus.z;
    }
    *maxP = add(max, fudge);
    *minP = sub(min, fudge);
}