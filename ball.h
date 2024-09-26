#pragma once
#ifndef _BALLHEADER_H_
#define _BALLHEADER_H_
#include "vec3.h"

typedef struct ball {
    vec3f pos;
    float influence;
} ball;

float influenceAt(vec3f, ball*, int);
void maxArea(ball*, int, vec3f*, vec3f*);

#endif