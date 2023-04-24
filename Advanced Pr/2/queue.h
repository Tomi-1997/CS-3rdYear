#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct node_ 
{
  void* value;
  void* next;
} node;

typedef struct QUEUE_
{
  node* head;
  int size;
} Queue;

Queue* createQ();
void   enQ(Queue** header, void* elem);
void*  deQ(Queue** header);

