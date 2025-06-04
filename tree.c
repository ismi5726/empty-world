#include "perlin.h"
#include "arr.h"
#include "tree.h"
#include "objects.h"
#include <stdlib.h>
#include <math.h>

tree *createTree() {
    tree *t = (tree *) calloc(1, sizeof(tree));
    t->tree = createObject();
    return t;
}

void deleteTree(tree *t) {
    deleteObject(t->tree);
    free(t);
}

#ifndef PI
#define PI 3.14159265358979323846
#endif
vec3f randInCircle(vec3f pos, vec3f dir, float scale, float *angle) {
    (void)pos;
    vec3f perpendicular;
    // taken from https://stackoverflow.com/a/47960392
    if (dir.z != 0) {
        perpendicular.x = 1;
        perpendicular.y = 1;
        perpendicular.z = -(dir.x + dir.y) / dir.z;
    } else if (dir.y != 0) {
        perpendicular.x = 1;
        perpendicular.z = 1;
        perpendicular.y = -(dir.x + dir.z) / dir.y;
    } else if (dir.x != 0) {
        perpendicular.y = 1;
        perpendicular.z = 1;
        perpendicular.x = -(dir.y + dir.z) / dir.x;
    } else {
        // dir is 0 vector, perpendicular isn't well-defined;
        return dir;
    }
    perpendicular = norm(perpendicular);
    vec3f third = cross(dir, perpendicular);
    float r = fmodf((float)rand(), PI) + PI / 2 + *angle;
    *angle = r;
    //float r = perlin2d(pos.x - pos.y, pos.z + pos.y, 2000, 1);
    return add(scalef(perpendicular, scale * sinf(r)), scalef(third, scale * cosf(r)));
}

tree *genTree(vec3f pos, int level) {
    tree *t = createTree();
    t->pos = pos;
    vec3f vertical = {0, 1, 0};
    float angle = fmodf((float)rand(), PI * 2);
    branch trunk = {pos, norm(add(randInCircle(pos, vertical, 0.1, &angle), vertical)), 10.0, 0.5, 0.0};
    branchOut(t, trunk, 40, level, 1, 0.2, 0.33);
    genObject(t->tree);
    return t;
}

// childSpacing refers to how much farther from the end of the branch each successive child is, from 0 (all at the tip) to 1 (???)
// as such, count * childSpacing must be <1
// childRadius follows the equation s^2 = c^2 + e^2, which means something weird and complicated for the relationship between necessary branch radius and childRadius
// angle refers to the angle the child branch is at, but the main branch is also slightly deflected
void branchOut(tree *t, branch start, int count, int depth, float angle, float bareLength, float lengthFactor) {
    float childRadius = sqrtf(powf(start.radius, 2) * 1.5 / (float)count);
    float childSpacing = (1 - bareLength) / (float)count;
    float startDist = start.length * (1.0 - (count - 1) * childSpacing);
    float dist = startDist;
    vec3f startPos = add(start.start, scalef(start.dir, startDist * 0.9));
    float startLength = start.length;
    float startRadius = start.radius;
    float childAngle = fmodf((float)rand(), PI * 2);

    for (int i = 0; i < count - 1; i++) {
        vec3f childDir = add(scalef(start.dir, cosf(angle)), scalef(randInCircle(startPos, start.dir, 1.0, &childAngle), sinf(angle)));
        branch child = {startPos, childDir, startLength * lengthFactor, childRadius, 0.0};
        if (depth > 0) {
            branchOut(t, child, count > 16 ? count / 8 : 2, depth - 1, angle * 0.9, childSpacing, lengthFactor);
        } else {
            createBranch(t, child, childRadius, 1);
        }

        float newRadius = sqrtf(powf(start.radius, 2) - powf(childRadius, 2));
        if (newRadius < childRadius) newRadius = childRadius;
        branch oldSegment = {start.start, start.dir, startDist, start.radius, newRadius};
        createBranch(t, oldSegment, startRadius, 0);

        start.start = startPos;
        start.length = startLength - dist;
        start.radius = newRadius;
        childRadius = sqrtf(powf(start.radius, 2) * 1.5 / (float)count);
        float nextAngle = fmodf((float)rand(), PI * 2);
        start.dir = norm(add(start.dir, randInCircle(add(start.dir, start.start), start.dir, 0.1 * start.radius, &nextAngle)));

        startPos = add(startPos, scalef(start.dir, startLength * childSpacing * 0.9));
        startDist = startLength * childSpacing;
        dist += childSpacing * startLength;
    }

    
    start.dir = norm(add(start.dir, randInCircle(add(start.dir, start.start), start.dir, 0.05 * start.radius, &childAngle)));

    if (depth > 0) {
        start.length = startLength * lengthFactor;
        branchOut(t, start, count > 16 ? count / 8 : 2, depth - 1, angle * 0.9, childSpacing, lengthFactor);
    } else {
        createBranch(t, start, startRadius, depth == 0);
    }
}

void createBranch(tree *t, branch b, float radius, int isLeaf) {
    (void)radius;
    vec3f dir = b.dir;
    vec3f perpendicular;
    // taken from https://stackoverflow.com/a/47960392
    if (dir.z != 0) {
        perpendicular.x = 1;
        perpendicular.y = 1;
        perpendicular.z = -(dir.x + dir.y) / dir.z;
    } else if (dir.y != 0) {
        perpendicular.x = 1;
        perpendicular.z = 1;
        perpendicular.y = -(dir.x + dir.z) / dir.y;
    } else if (dir.x != 0) {
        perpendicular.y = 1;
        perpendicular.z = 1;
        perpendicular.x = -(dir.y + dir.z) / dir.x;
    } else {
        // dir is 0 vector, perpendicular isn't well-defined;
        return;
    }
    perpendicular = norm(perpendicular);
    vec3f third = cross(dir, perpendicular);

    double step = PI / 3;
    vec3f current;
    vec3f next = third;
    vec3f a, c, d, e;
    vec3f end = add(b.start, scalef(b.dir, b.length));
    for (double i = 0; i <= 2 * PI; i += step) {
        current = next;
        next = add(scalef(perpendicular, sinf(i + step)), scalef(third, cosf(i + step)));
        a = add(b.start, scalef(current, b.radius));
        c = add(b.start, scalef(next, b.radius));
        d = add(end, scalef(current, b.endRadius));
        e = add(end, scalef(next, b.endRadius));
        t->tree->vertices = pushMultiple(t->tree->vertices, 18,
            a.x, a.y, a.z,
            c.x, c.y, c.z,
            d.x, d.y, d.z,
            d.x, d.y, d.z,
            c.x, c.y, c.z,
            e.x, e.y, e.z
        );
        t->tree->normals = pushMultiple(t->tree->normals, 18,
            current.x, current.y, current.z,
            next.x, next.y, next.z,
            current.x, current.y, current.z,
            current.x, current.y, current.z,
            next.x, next.y, next.z,
            next.x, next.y, next.z
        );
        if (isLeaf == 0) {
            t->tree->colors = pushMultiple(t->tree->colors, 18,
                0.45, 0.36, 0.26,
                0.45, 0.36, 0.26,
                0.45, 0.36, 0.26,
                0.45, 0.36, 0.26,
                0.45, 0.36, 0.26,
                0.45, 0.36, 0.26
            );
        } else {
            t->tree->colors = pushMultiple(t->tree->colors, 18,
                0.36, 0.66, 0.01,
                0.36, 0.66, 0.01,
                0.36, 0.66, 0.01,
                0.36, 0.66, 0.01,
                0.36, 0.66, 0.01,
                0.36, 0.66, 0.01
            );
        }
    }
}