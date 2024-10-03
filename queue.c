#include "queue.h"
#include <stdlib.h>

queue *enqueue(queue *q, void *data, int priority) {
    queue *item;
    item = (queue *) calloc(1, sizeof(queue));
    item->data = data;
    item->priority = priority;

    queue *prev = NULL;
    queue *current = q;
    for (; current && current->priority > priority; current = current->next) prev = current;

    if (prev) {
        item->next = prev->next;
        prev->next = item;
        return q;
    } else {
        item->next = q;
        return item;
    }
}

void *peek(queue *q) {
    return q->data;
}

queue *pop(queue *q) {
    if (!q) return NULL;
    queue *next = q->next;
    free(q);
    return next;
}