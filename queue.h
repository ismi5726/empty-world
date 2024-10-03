#pragma once

typedef struct queue {
    void *data;
    int priority;
    struct queue *next;
} queue;

queue *enqueue(queue *q, void *data, int priority);
void *peek(queue *q);
queue *pop(queue *q);