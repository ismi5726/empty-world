#pragma once

typedef struct growArr {
    int capacity;
    int size;
    float *data;
} growArr;

growArr createArr();
growArr _expand(growArr);
growArr push(growArr, float);
growArr pushMultiple(growArr, int, ...);