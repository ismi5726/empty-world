#pragma once
#ifndef _VEC3HEADER_H_
#define _VEC3HEADER_H_

typedef struct vec3f {
    float x;
    float y;
    float z;
} vec3f;

vec3f scalef(vec3f, float);
vec3f scalev3f(vec3f, vec3f);
vec3f add(vec3f, vec3f);
vec3f sub(vec3f, vec3f);
vec3f vAbs(vec3f);
vec3f inv(vec3f);
vec3f interpolate(vec3f, vec3f, float, float);
vec3f cross(vec3f, vec3f);
vec3f vec3fInArray(float*, int);
float vec3fLength(vec3f);
vec3f norm(vec3f);
float dot(vec3f, vec3f);

#endif