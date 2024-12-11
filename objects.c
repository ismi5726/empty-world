#include "objects.h"
#include "arr.h"
#include <stdlib.h>
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif

object *createObject() {
    object *obj = (object *) calloc(1, sizeof(object));
    obj->vertices = createArr();
    obj->normals = createArr();
    obj->colors = createArr();
    glGenBuffers(3, obj->buffers);
    return obj;
}

void genObject(object *ptr) {
    glBindBuffer(GL_ARRAY_BUFFER, ptr->buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, ptr->vertices.size * sizeof(float), ptr->vertices.data, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ptr->buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, ptr->normals.size * sizeof(float), ptr->normals.data, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ptr->buffers[2]);
    glBufferData(GL_ARRAY_BUFFER, ptr->colors.size * sizeof(float), ptr->colors.data, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void deleteObject(object *obj) {
    delArr(obj->vertices);
    delArr(obj->colors);
    delArr(obj->normals);
    free(obj);
}

object *chairGen() {
    object *chair = createObject();
    
}