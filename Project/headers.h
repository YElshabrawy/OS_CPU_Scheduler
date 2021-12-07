#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

#include "cvector.h"
#define CVECTOR_LOGARITHMIC_GROWTH

typedef short bool;
#define true 1
#define false 1

#define SHKEY 300


///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

struct ProcessInfo
{
    int id;
    int arrival_time;
    int runtime;
    int priority;
};

struct QNode
{
    struct ProcessInfo* key;
    struct QNode *next;
};

struct Queue
{
    struct QNode *front, *rear;
};

struct QNode *newNode(struct ProcessInfo* k)
{
    struct QNode *temp = (struct QNode *)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}

struct Queue *createQueue()
{
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

void enQueue(struct Queue *q, struct ProcessInfo * k)
{
    // Create a new LL node
    struct QNode *temp = newNode(k);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

void deQueue(struct Queue *q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return;

    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}

// // Queue of structs
// struct Queue
// {
//     int front, rear, size;
//     unsigned capacity;
//     struct ProcessInfo* *array;
// };

// struct Queue *createQueue(unsigned capacity)
// {
//     struct Queue *queue = (struct Queue *)malloc(
//         sizeof(struct Queue));
//     queue->capacity = capacity;
//     queue->front = queue->size = 0;

//     // This is important, see the enqueue
//     queue->rear = capacity - 1;
//     queue->array = (int *)malloc(
//         queue->capacity * sizeof(int));
//     return queue;
// }

// int isFull(struct Queue *queue)
// {
//     return (queue->size == queue->capacity);
// }

// // Queue is empty when size is 0
// int isEmpty(struct Queue *queue)
// {
//     return (queue->size == 0);
// }

// void enqueue(struct Queue *queue, struct ProcessInfo* item)
// {
//     if (isFull(queue))
//         return;
//     queue->rear = (queue->rear + 1) % queue->capacity;
//     queue->array[queue->rear] = item;
//     queue->size = queue->size + 1;
// }

// struct ProcessInfo* dequeue(struct Queue *queue)
// {
//     if (isEmpty(queue))
//         return NULL;
//     struct ProcessInfo* item = queue->array[queue->front];
//     queue->front = (queue->front + 1) % queue->capacity;
//     queue->size = queue->size - 1;
//     return item;
// }

// struct ProcessInfo* front(struct Queue *queue)
// {
//     if (isEmpty(queue))
//         return NULL;
//     return queue->array[queue->front];
// }

// // Function to get rear of queue
// struct ProcessInfo* rear(struct Queue *queue)
// {
//     if (isEmpty(queue))
//         return NULL;
//     return queue->array[queue->rear];
// }