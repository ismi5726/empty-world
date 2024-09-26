#pragma once
#ifndef _QUATHEADER_H_
#define _QUATHEADER_H_
// this is a more mathematical definition of a quaternion than the usual xyzw, but I like it
typedef struct quat {
    double i;
    double j;
    double k;
    double r;
} quat;

quat fromEuler(double, double, double);
quat multiply(quat, quat);
double *toMat4(quat);

#endif