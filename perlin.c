// adapted from https://gist.github.com/nowl/828013 and https://github.com/christopher-little/PerlinNoiseCpp/blob/master/Perlin.cpp to be actual perlin noise and in c instead of c++
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

static int p[256] = {0};
static float Gx[256] = {0};
static float Gy[256] = {0};

float noise2(int x, int y, float xf, float yf)
{
    int idx = p[(x + p[y & 255]) & 255];
    return Gx[idx] * xf + Gy[idx] * yf;
}

float lin_inter(float x, float y, float s)
{
    return x + s * (y-x);
}

float smooth_inter(float x, float y, float s)
{
    return lin_inter(x, y, s * s * (3-2*s));
}

float noise2d(float x, float y)
{
    int x_int = floorf(x);
    int y_int = floorf(y);
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    float s = noise2(x_int, y_int, x_frac, y_frac);
    float t = noise2(x_int+1, y_int, x_frac - 1.0f, y_frac);
    float u = noise2(x_int, y_int+1, x_frac, y_frac - 1.0f);
    float v = noise2(x_int+1, y_int+1, x_frac - 1.0f, y_frac - 1.0f);
    float wx = ((6 * x_frac - 15) * x_frac + 10) * x_frac * x_frac * x_frac;
    float wy = ((6 * y_frac - 15) * y_frac + 10) * y_frac * y_frac * y_frac;
    float xa = s + wx * (t - s);
    float xb = u + wx * (v - u);
    float value = xa + wy * (xb - xa);
    return value;
}

float perlin2d(float x, float y, float freq, int depth)
{
    float xa = x*freq;
    float ya = y*freq;
    float amp = 1.0;
    float fin = 0;
    float div = 0.0;

    int i;
    for(i=0; i<depth; i++)
    {
        div += amp;
        fin += noise2d(xa, ya) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }

    return fin/div;
}

void setupPerlin() {
    for (int i = 0; i < 256; i++) {
        p[i] = i;

        Gx[i] = ((float)(rand()) / (RAND_MAX/2)) - 1.0f;
		Gy[i] = ((float)(rand()) / (RAND_MAX/2)) - 1.0f;
    }

    int j = 0;
    int tmp = 0;
    for (int i = 0; i < 256; i++) {
        j = rand() & 255;

        tmp = p[i];
        p[i] = p[j];
        p[j] = tmp;
    }
}