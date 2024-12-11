#pragma once

#include "arr.h"
#include "vec3.h"
#include "quat.h"

typedef struct object {
    growArr vertices;
    growArr normals;
    growArr colors;
    vec3f size;
    unsigned int buffers[3];
} object;

typedef struct objectInstance {
    object *obj;
    vec3f pos;
    quat orientation;
} objectInstance;

object *createObject();
void genObject(object *ptr);
void deleteObject(object *obj);
object *chairGen();