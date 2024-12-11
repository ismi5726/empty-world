#include <string.h>
#include <stdio.h>
#include "helvetica.h"
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif

// code taken from freetype-gl's embedded font demo
// https://github.com/rougier/freetype-gl/blob/master/demos/embedded-font.c
// and mildly adapted for my use case

GLuint texid;
void textInit(void) {
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font.tex_width, font.tex_height,
                 0, GL_ALPHA, GL_UNSIGNED_BYTE, font.tex_data);
    glBindTexture(GL_TEXTURE_2D, texid);
}

void print2D(int pen_x, int pen_y, char *text, int size) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, texid);
    size_t i;
    size_t len = strlen(text);
    float scale = size / (float) font.size;
    for (i = 0; i < len; i++) {
        texture_glyph_t *glyph = font.glyphs[0][(int)text[i]];
        if (!glyph) {
            printf("font is missing glyph %i (%c)\n", text[i], text[i]);
            continue;
        }
        int x = pen_x + glyph->offset_x * scale;
        int y = pen_y + glyph->offset_y * scale;
        int w = glyph->width * scale;
        int h = glyph->height * scale;
        glBegin(GL_TRIANGLES);
        {
            glTexCoord2f(glyph->s0, glyph->t0); glVertex2i(x,   y  );
            glTexCoord2f(glyph->s0, glyph->t1); glVertex2i(x,   y-h);
            glTexCoord2f(glyph->s1, glyph->t1); glVertex2i(x+w, y-h);
            glTexCoord2f(glyph->s0, glyph->t0); glVertex2i(x,   y  );
            glTexCoord2f(glyph->s1, glyph->t1); glVertex2i(x+w, y-h);
            glTexCoord2f(glyph->s1, glyph->t0); glVertex2i(x+w, y  );
        }
        glEnd();
        pen_x += glyph->advance_x * scale;
        pen_y += glyph->advance_y * scale;
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
}