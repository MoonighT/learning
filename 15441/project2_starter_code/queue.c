#include "queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

queue_t *queue_init(void) {
    queue_t *q = (queue_t*)malloc(sizeof(queue_t));
    q->head = NULL;
    q->tail = NULL;
    q->n = 0;
    return q;
}

void enqueue(queue_t* q, void* data) {
    node_t *n = (node_t*)malloc(sizeof(node_t));
    n->data = data;
    n->next = NULL;

    if(q->tail == NULL) {
        q->head = n;
    } else {
        q->tail->next = n;
    }
    q->tail = n;
    q->n += 1;
}

void* dequeue(queue_t* q) {
    if(q->n == 0) {
        return NULL;
    }
    assert(q->head != NULL);
    node_t* node= q->head;
    q->head = q->head->next;
    if(q->head == NULL) {
        q->tail = NULL;
    }
    q->n -= 1;
    void* data = node->data;
    free(node);
    return data;
}

void free_queue(queue_t* q) {
    node_t* cur = q->head;
    node_t* next;
    if (cur != NULL) {
        next = cur->next;
        free(cur);
        cur = next;
    }
    free(q);
}
