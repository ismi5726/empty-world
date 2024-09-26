#include "quat.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// all of the following code is adapted from https://glmatrix.net/docs/quat.js.html
// as these are just utility functions
#ifndef PI
#define PI 3.14159265358979323846
#endif

// convert an euler angle
// https://glmatrix.net/docs/quat.js.html#line459
quat fromEuler(double x, double y, double z) {
    quat out;
    double halfToRad = (0.5 * PI) / 180.0;
    double rx = x * halfToRad;
    double ry = y * halfToRad;
    double rz = z * halfToRad;

    double sx = sin(rx);
    double cx = cos(rx);
    double sy = sin(ry);
    double cy = cos(ry);
    double sz = sin(rz);
    double cz = cos(rz);

    out.i = sx * cy * cz - cx * sy * sz;
    out.j = cx * sy * cz + sx * cy * sz;
    out.k = cx * cy * sz - sx * sy * cz;
    out.r = cx * cy * cz + sx * sy * sz;

    return out;
}

// multiply 2 quaternions
// https://glmatrix.net/docs/quat.js.html#line110
quat multiply(quat a, quat b) {
    quat out;
    out.i = a.i * b.r + a.r * b.i + a.j * b.k - a.k * b.j;
    out.j = a.j * b.r + a.r * b.j + a.k * b.i - a.i * b.k;
    out.k = a.k * b.r + a.r * b.k + a.i * b.j - a.j * b.i;
    out.r = a.r * b.r - a.i * b.i - a.j * b.j - a.k * b.k;

    return out;
}

// except this one, which is https://glmatrix.net/docs/mat4.js.html#line1420
double *toMat4(quat x) {
    double i = x.i;
    double j = x.j;
    double k = x.k;
    double r = x.r;
    double i2 = i + i;
    double j2 = j + j;
    double k2 = k + k;

    double ii = i * i2;
    double ji = j * i2;
    double jj = j * j2;
    double ki = k * i2;
    double kj = k * j2;
    double kk = k * k2;
    double ri = r * i2;
    double rj = r * j2;
    double rk = r * k2;

    double *out = (double *) malloc(16 * sizeof(double));

    out[0] = 1 - jj - kk;
    out[1] = ji + rk;
    out[2] = ki - rj;
    out[3] = 0;

    out[4] = ji - rk;
    out[5] = 1 - ii - kk;
    out[6] = kj + ri;
    out[7] = 0;

    out[8] = ki + rj;
    out[9] = kj - ri;
    out[10] = 1 - ii - jj;
    out[11] = 0;

    out[12] = 0;
    out[13] = 0;
    out[14] = 0;
    out[15] = 1;

    return out;
}