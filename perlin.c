// adapted from https://gist.github.com/nowl/828013 and https://github.com/christopher-little/PerlinNoiseCpp/blob/master/Perlin.cpp to be actual perlin noise and in c instead of c++
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

static int SEED = 0;

/*static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};*/

//static int hash[] = {198,205,217,45,56,171,133,103,141,11,131,149,69,26,89,9,162,182,173,49,215,93,140,218,230,37,126,33,207,98,180,233,116,59,40,165,158,20,240,177,246,138,157,0,79,118,130,6,22,25,176,66,58,91,57,134,23,92,179,74,111,212,2,117,224,114,95,183,211,188,76,135,208,249,86,191,83,73,29,64,1,210,235,115,169,132,200,44,36,106,247,125,145,113,250,195,5,41,122,241,234,202,219,27,189,80,227,155,223,108,120,39,252,85,151,160,129,12,143,146,14,175,148,236,163,54,167,75,16,242,8,127,194,105,13,96,97,139,84,28,77,100,231,90,161,128,201,109,99,150,237,137,248,243,203,238,51,123,87,142,71,156,112,216,144,124,48,214,82,72,32,7,152,78,213,209,190,193,4,61,154,192,47,63,225,107,197,159,196,68,94,21,30,53,65,52,221,34,46,19,147,60,199,67,50,184,204,170,121,220,251,245,168,253,181,81,206,31,254,110,226,24,62,174,55,228,255,43,187,70,136,104,239,244,229,10,35,17,0,178,186,38,232,166,42,3,119,18,153,88,101,172,15,164,222,102};

static int p[256];
static float Gx[256];
static float Gy[256];
static float Gz[256];

float noise2(int x, int y, float xf, float yf)
{
    //int tmp = hash[MOD(y + SEED, 256)];
    //return hash[MOD(tmp + x, 256)];
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
    /*float low = smooth_inter(s, t, x_frac);
    float high = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);*/
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
		//Gz[i] = ((float)(rand()) / (RAND_MAX/2)) - 1.0f;
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