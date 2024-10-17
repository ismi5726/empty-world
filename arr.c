#include "arr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

growArr createArr() {
    growArr out = {10, 0, 0};
    out.data = calloc(10, sizeof(float));
    return out;
}

growArr _expand(growArr input) {
    int newSize = input.capacity * 1.5;
    float *data = realloc(input.data, newSize * sizeof(float));
    if (!data) {
        printf("Allocation of size %i failed!\n", newSize);
        exit(1);
    }
    growArr out = {newSize, input.size, data};
    return out;
}

growArr push(growArr input, float element) {
    input.data[input.size++] = element;
    if (input.size >= input.capacity) {
        return _expand(input);
    } else {
        return input;
    }
}

growArr pushMultiple(growArr input, int num, ...) {
    while (input.size + num >= input.capacity) {
        input = _expand(input);
    }

    va_list arg;
    va_start(arg, num);

    for (int i = 0; i < num; i++) {
        input.data[input.size++] = (float)va_arg(arg, double);
    }

    va_end(arg);

    return input;
}

void delArr(growArr input) {
    free(input.data);
    // i think this should avoid double-frees
    input.data = 0;
}

growArr clear(growArr input) {
    delArr(input);
    return createArr();
}