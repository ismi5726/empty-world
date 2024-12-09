#include "queue.h"
#include <stdlib.h>

queue *enqueue(queue *q, void *data, int priority) {
    queue *item;
    item = (queue *) calloc(1, sizeof(queue));
    item->data = data;
    item->priority = priority;

    queue *prev = NULL;
    queue *current = q;
    for (; current && current->priority <= priority; current = current->next) {
        prev = current;
        // avoid re-adding items
        if (current->data == data && current->priority == priority) return q;
    }

    if (prev) {
        item->next = prev->next;
        prev->next = item;
        return q;
    } else {
        item->next = q;
        return item;
    }
}

queue *dequeue(queue *q, void *data) {
    if (!q) return q;
    queue *temp, *current;
    while (q->data == data) {
        temp = q->next;
        free(q);
        q = temp;
    }
    temp = q;
    current = q->next;
    for (; current; current = current->next) {
        if (current->data == data) {
            temp->next = current->next;
            free(current);
            current = temp->next;
            if (!current) break;
        }
        temp = current;
    }
    return q;
}

void *peek(queue *q) {
    if (!q) return NULL;
    return q->data;
}

queue *pop(queue *q) {
    if (!q) return NULL;
    queue *next = q->next;
    free(q);
    return next;
}