#include<stdio.h>
#include<stdlib.h>
#include <pthread.h>

typedef struct Queue {
        size_t capacity;
        size_t size;
        int front;
        int rear;
        void* elements;
        size_t sizeJob;
} Queue;

Queue * createQueue(size_t nJob, void* src, size_t sizeJob) {
        Queue *Q;
        Q = (Queue *)malloc(sizeof(Queue));
        
        Q->elements = src;
        Q->size = nJob;
        Q->capacity = nJob;
        Q->front = 0;
        Q->rear = nJob - 1;
       
        return Q;
}
size_t Dequeue(Queue *Q) {
        
    if(Q->size==0) {
        return -1;
    } else {
        size_t output = Q->front;

        Q->size--;
        Q->front += Q->sizeJob;

        return output;
    }
    return -1;
}