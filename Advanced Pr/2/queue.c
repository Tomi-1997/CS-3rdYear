#include "queue.h"

Queue* createQ()
{
  Queue* handle = (Queue*)malloc(sizeof(Queue));
  handle->size = 0;
  handle->head = NULL;
  return handle;
}

void enQ(Queue** header, void* elem)
{
  node* element = (node*)malloc(sizeof(node));
  /* Point new elemenet's next node as the current head */
  element->value = elem;
  element->next = (*header)->head;
  /* Set new head as the newly malloc'd element */
  (*header)->head = element;
  (*header)->size++;
}

void* deQ(Queue** header)
{
	node* prev = (*header)->head;

  if ( prev == NULL )
		return  NULL;

  /* Keep memory if previous and current node */
  node* curr = prev->next;

  /* If curr is node, there is only one element.*/
  if (curr == NULL)
  {
    (*header)->head = NULL;
    (*header)->size--;
    return prev->value;
  }

  /* Keep iterating until we reach (prev -> curr -> null)
    Reroute prev to null, and return curr*/
  while (curr->next != NULL)
  {
    prev = prev->next;
    curr = curr->next;
  }

  prev->next = curr->next;
  (*header)->size--;
  return curr->value;
}
