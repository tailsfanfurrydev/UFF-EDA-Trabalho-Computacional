#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "GenericTypes.h"

typedef struct listNode{
  void *info;
  struct listNode *next;
}LinkedList;

LinkedList* linkedListInitialize(void);
LinkedList* linkedListInsert(LinkedList *l, void *elem);
void linkedListInsertVoid(LinkedList **l, void *elem);

void linkedListPrint(LinkedList *l, PrintFunc print);
void linkedListFree(LinkedList *l, FreeFunc freeData);
LinkedList* linkedListRemove(LinkedList *l, void *elem, CompareFunc compare, FreeFunc freeData);
LinkedList* linkedListSearch(LinkedList *l, void *elem, CompareFunc compare);

void linkedListPrintRecursive(LinkedList *l, PrintFunc print);
void linkedListPrintRecursiveReverse(LinkedList *l, PrintFunc print);
void linkedListFreeRecursive(LinkedList *l, FreeFunc freeData);
LinkedList* linkedListSearchRecursive(LinkedList *l, void *elem, CompareFunc compare);
LinkedList* linkedListRemoveRecursive(LinkedList *l, void *elem, CompareFunc compare, FreeFunc freeData);
int linkedListSize(LinkedList *l);

#endif