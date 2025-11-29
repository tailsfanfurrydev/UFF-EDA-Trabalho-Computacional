#ifndef B_PLUS_TREE_2_M_H
#define B_PLUS_TREE_2_M_H


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct bPlusNode2{
  int t, numKeys, isLeafsParent, *key;
  long *childOffsets;
  char **leafFiles;
}BPlusTree2M;

typedef struct leafnode{
  int t, numKeys;
  char *nextLeafFile, **YearFileArray;
}LeafNode;

BPlusTree2M *bPlusTreeCreate2M(int t);        
BPlusTree2M *bPlusTreeInitialize2M(void);     
char *bPlusTreeSearch2M(FILE *indexFileP, int k, long offset);
int bPlusTreeInsert2M(FILE *indexFile, int key, int t);
void bPlusTreeFree2M(BPlusTree2M *a);        
void bPlusTreePrintNode2M(BPlusTree2M *a);   



void leafNodePrint(LeafNode *l);
LeafNode *leafNodeCreate2M(int t);     
void leafNodeFree2M(LeafNode *l);      


#endif