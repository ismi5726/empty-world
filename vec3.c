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

vec3f interpolate(vec3f a, vec3f b, float _a, float _b) {
    if (_a == _b) {
        _b = 3;
        _a = 1;
    }
    return add(a, scalef(sub(b, a), -_a / (_b - _a)));
}