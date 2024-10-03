#include "vec3.h"

vec3f scalef(vec3f v, float n) {
    vec3f out = {v.x * n, v.y * n, v.z * n};
    return out;
}

vec3f scalev3f(vec3f v, vec3f n) {
    vec3f out = {v.x * n.x, v.y * n.y, v.z * n.z};
    return out;
}

vec3f add(vec3f a, vec3f b) {
    vec3f out = {a.x + b.x, a.y + b.y, a.z + b.z};
    return out;
}

vec3f sub(vec3f a, vec3f b) {
    vec3f out = {a.x - b.x, a.y - b.y, a.z - b.z};
    return out;
}

vec3f vAbs(vec3f v) {
    if (v.x < 0) v.x = -v.x;
    if (v.y < 0) v.y = -v.y;
    if (v.z < 0) v.z = -v.z;
    return v;
}

vec3f inv(vec3f v) {
    if (v.x != 0) v.x = 1/v.x;
    if (v.y != 0) v.y = 1/v.y;
    if (v.z != 0) v.z = 1/v.z;
    return v;
}

vec3f interpolate(vec3f a, vec3f b, float _a, float _b) {
    if (_a == _b) {
        _b = 3;
        _a = 1;
    }
    return add(a, scalef(sub(b, a), -_a / (_b - _a)));
}