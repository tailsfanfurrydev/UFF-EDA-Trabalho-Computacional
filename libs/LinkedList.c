#include "LinkedList.h"

LinkedList* linkedListInitialize(void){
  return NULL;
}

LinkedList* linkedListInsert(LinkedList *l, void *elem){
  LinkedList *new = (LinkedList *) malloc(sizeof(LinkedList));
  new->next = l;
  new->info = elem;
  return new;
}
void linkedListInsertVoid(LinkedList **l, void *elem){
  LinkedList *new = (LinkedList *) malloc(sizeof(LinkedList));
  new->next = *l;
  new->info = elem;
  (*l) = new;
}

void linkedListPrint(LinkedList *l, PrintFunc print){
  LinkedList *p = l;
  while(p){
    print(p->info);
    p = p->next;
  } 
}

void linkedListPrintRecursive(LinkedList *l, PrintFunc print){
  if(l){
    print(l->info);
    linkedListPrintRecursive(l->next, print);
  }
}

void linkedListPrintRecursiveReverse(LinkedList *l, PrintFunc print){
  if(l){
    linkedListPrintRecursiveReverse(l->next, print);
    print(l->info);
  }
}

void linkedListFree(LinkedList *l, FreeFunc freeData){
  LinkedList *p = l, *q;
  while(p){
    q = p;
    p = p->next;
    if(freeData) freeData(q->info);
    free(q);
  } 
}

void linkedListFreeRecursive(LinkedList *l, FreeFunc freeData){
  if(l){
    linkedListFreeRecursive(l->next, freeData);
    if(freeData) freeData(l->info);
    free(l);
  }
}

LinkedList* linkedListRemove(LinkedList *l, void *elem, CompareFunc compare, FreeFunc freeData){
  LinkedList *p = l, *ant = NULL;
  while((p) && (compare(p->info, elem) != 0)){
    ant = p;
    p = p->next;
  }
  if(!p) return l;
  if(!ant) l = l->next;
  else ant->next = p->next;
  if(freeData) freeData(p->info);
  free(p);
  return l;
}

LinkedList* linkedListRemoveRecursive(LinkedList *l, void *elem, CompareFunc compare, FreeFunc freeData){
  if(!l) return l;
  if(compare(l->info, elem) == 0){
    LinkedList *p = l;
    l = l->next;
    if(freeData) freeData(p->info);
    free(p);
  }
  else l->next = linkedListRemoveRecursive(l->next, elem, compare, freeData);
  return l;
}

LinkedList* linkedListSearch(LinkedList *l, void *elem, CompareFunc compare){
  LinkedList *p = l;
  while((p) && (compare(p->info, elem) != 0)) p = p->next; 
  return p;
}

LinkedList* linkedListSearchRecursive(LinkedList *l, void *elem, CompareFunc compare){
  if((!l) || (compare(l->info, elem) == 0)) return l;
  return linkedListSearchRecursive(l->next, elem, compare);
}

int linkedListSize(LinkedList *l){
  int count = 0;
  LinkedList *p = l;
  while(p){
    count++;
    p = p->next;
  }
  return count;
}