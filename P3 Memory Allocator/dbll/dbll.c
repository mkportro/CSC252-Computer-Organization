#include <stdlib.h>
#include <stdio.h>
#include "dbll.h"

struct llnode *newNode(void *data);
void dbll_pop(struct dbll *list);
struct llnode *dbll_preppend(struct dbll *list, void *user_data);

/* Routines to create and manipulate a doubly-linked list */

/* create new node */
/* returns an empty node with all values initialized to NULL or NULL if memory allocation failed */
struct llnode *newNode(void *data)
{
  struct llnode *new = (struct llnode*)malloc(sizeof(struct llnode));

  if (new != NULL) {
    new->user_data = data;
    new->prev = new->next = NULL;
    return new;
  }

  return NULL;
}

/* create a doubly-linked list */
/* returns an empty list or NULL if memory allocation failed */
struct dbll *dbll_create()
{
  struct dbll *list = (struct dbll*)malloc(sizeof(struct dbll));
  
  if (list != NULL) {
    list->first = list->last = NULL;
    return list;
  }

  return NULL;
}

/* frees all memory associated with a doubly-linked list */
/* this must also free all memory associated with the linked list nodes */
/* assumes user data has already been freed */
void dbll_free(struct dbll *list)
{
  if (list == NULL) {
    return;
  }

  //free nodes
  struct llnode *node = list->first;
  while (node != NULL) {
    struct llnode *next = node->next;
    free(node);
    node = next;
  }

  //free list
  free(list);
}

/* iterate over items stored in a doubly-linked list */

/* begin iteration from node start, end iteration at node stop (include stop in iteration) */
/* the list is traversed using the next pointer of each node */

/* if start is NULL, then start is the first node */
/* if stop is NULL, then stop is the last node */

/* at each node, call the user function f with a pointer to the list,
   a pointer to the current node, and the value of ctx */

/* if f returns 0, stop iteration and return 1 */

/* return 0 if you reached the end of the list without encountering stop */
/* return 1 on successful iteration */

int dbll_iterate(struct dbll *list,
				 struct llnode *start,
				 struct llnode *stop,
				 void *ctx,
				 int (*f)(struct dbll *, struct llnode *, void *))
{
  if (start == NULL) {
    start = list->first;
  }
  if (stop == NULL) { //NOT SURE IF NECESSARY
    stop = list->last;
  }

  struct llnode *it = start;
  int call;

  do {
    call = f(list, it->next, ctx);
      if (call == 0) {
        return 1;
      }
      it = it->next;
  } while (it != stop);
  
  if (it == stop) {
    return 1;
  } else {
    return 0;
  }
}

/* similar to dbll_iterate, except that the list is traversed using
   the prev pointer of each node (i.e. in the reverse direction).

   Adjust accordingly. For example if start is NULL, then start is the
   last node in the list.  If stop is NULL, then stop is the first node
   in the list.

*/

int dbll_iterate_reverse(struct dbll *list,
						 struct llnode *start,
						 struct llnode *stop,
						 void *ctx,
						 int (*f)(struct dbll *, struct llnode *, void *)
						 )
{
  if (start == NULL) {
    start = list->last;
  }
  if (stop == NULL) { //NOT SURE IF NECESSARY
    stop = list->first;
  }

  struct llnode *it = start;
  int call;

  do {
    call = f(list, it, ctx);
      if (call == 0) {
        return 1;
      }
      it = it->prev;
  } while (it != stop);
  
  if (it == stop) {
    return 1;
  } else {
    return 0;
  }
}


/* Remove `llnode` from `list` */
/* Memory associated with `node` must be freed */
/* You can assume user_data will be freed by somebody else (or has already been freed) */
void dbll_remove(struct dbll *list, struct llnode *node)
{
  struct llnode *it = list->first;

  while (it != node) {
    it = it->next;
  }

  if (it == list->first) {
    list->first = it->next;
  }
  if (it == list->last) {
    list->last = it->prev;
  }
  if (it->prev != NULL) {
    it->prev->next = it->next;
  }
  if (it->next != NULL) {
    it->next->prev = it->prev;
  }

  free(it);
  return;
}

/* Removes last 'llnode' from 'list' */
void dbll_pop(struct dbll *list) {
  if (list->last != NULL) {
    list->last = list->last->prev;
    free(list->last->next);
    list->last->next = NULL;
    return;
  }
  return;
}

/* Create and return a new node containing `user_data` */
/* The new node must be inserted after `node` */
/* if node is NULL, then insert the node at the end of the list */
/* return NULL if memory could not be allocated */
struct llnode *dbll_insert_after(struct dbll *list, struct llnode *node, void *user_data)
{
  struct llnode *new = newNode(user_data);
  if (new == NULL) { //check mem allocation
      return NULL;
    }
  
  if (node != NULL && node != list->last) { //insert between
    new->prev = node;
    new->next = node->next;
    node->next->prev = new;
    node->next = new;
    return new;
  } else { //empty list, or insert last
    return dbll_append(list, user_data);
  }
}

/* Create and return a new node containing `user_data` */
/* The new node must be inserted before `node` */
/* if node is NULL, then insert the new node at the beginning of the list */
/* return NULL if memory could not be allocated */
struct llnode *dbll_insert_before(struct dbll *list, struct llnode *node, void *user_data)
{
  struct llnode *new = newNode(user_data);
  if (new == NULL) { //check mem allocation
      return NULL;
    }
  
  if (node != NULL && node != list->first) { //insert between
    new->next = node;
    new->prev = node->prev;
    node->prev->next = new;
    node->prev = new;
    return new;
  } else { //empty list, or insert first
    return dbll_preppend(list, user_data);
  }
}

/* create and return an `llnode` that stores `user_data` */
/* the `llnode` is added to the end of list */
/* return NULL if memory could not be allocated */
/* this function is a convenience function and can use the dbll_insert_after function */
struct llnode *dbll_append(struct dbll *list, void *user_data)
{
  struct llnode *new = newNode(user_data);

  if (new == NULL) { //check mem allocation
    return NULL;
  }

  new->prev = list->last;
  if (list->last != NULL) {
    list->last->next = new;
  }
  list->last = new;
  if (list->first == NULL) { //empty list
    list->first = new;
  }

  return new;
}

/* create and return an `llnode` that stores `user_data` */
/* the `llnode` is added to the front of list */
/* return NULL if memory could not be allocated */
/* this function is a convenience function and can use the dbll_insert_before function */
struct llnode *dbll_preppend(struct dbll *list, void *user_data)
{
  struct llnode *new = newNode(user_data);

  if (new == NULL) { //check mem allocation
    return NULL;
  }

  new->next = list->first;
  if (list->first != NULL) {
    list->first->prev = new;
  }
  list->first = new;
  if (list->last == NULL) { //empty list
    list->last = new;
  }

  return new;
}