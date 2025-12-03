#include "BPlusTree2M.h"
#include <string.h>
#include "BPlusStorage.h"
#include <stdlib.h>

BPlusTree2M *bPlusTreeCreate2M(int t){
  if(t<=0){
    printf("error creating node");
    return NULL;
  }
  BPlusTree2M* new  = (BPlusTree2M*)malloc(sizeof(BPlusTree2M));
  if(!new){  
    printf("malloc error in bPlusTreeCreate2M");
    exit(1);
  }

  new->t = t;
  new->numKeys = 0;
  new->key =(int*)malloc(sizeof(int)*((t*2)-1));
  new->isLeafsParent = 1;
  new->childOffsets = (long*) malloc(sizeof(long)*2*t);

  int i;
  for(i=0; i<(t*2); i++) new->childOffsets[i] = LONG_MAX;
  for(i=0; i<(t*2)-1; i++) new->key[i] = INT_MAX;

  new->leafFiles = (char**) malloc(sizeof(char*)*2*t);

  for(i=0; i<(t*2); i++){
    new->leafFiles[i] = (char*) malloc(sizeof(char)*5);
    strcpy(new->leafFiles[i], "XXXX" );
  }
  
  return new;
}

BPlusTree2M *bPlusTreeInitialize2M(void){
  return NULL;
}

void bPlusTreeFree2M(BPlusTree2M *a){
  if(a){
    int i;
    for(i = 0; i < (2*(a->t)); i++){
      if(a->leafFiles[i]) free(a->leafFiles[i]);
    }
    free(a->leafFiles);
    free(a->childOffsets);
    free(a->key);
    free(a);
  }
}

void bPlusTreePrintNode2M(BPlusTree2M *a){
  int i;
  char *str;
  printf("\n========================================\n");
  printf("t = %d\n", a->t);
  printf("numKeys = %d\n", a->numKeys);
  printf("isLeafsParent = %d\n", a->isLeafsParent);

  printf("key array: [");
  for(i=0; i< ((a->t)*2)-1; i++){
    if(i!=0) printf(",");
    printf("%d", (a->key)[i] );
  }
  printf("]\n");

  printf("child array: [");
    for(i=0; i< ((a->t)*2); i++){
        if(i!=0) printf(",");
        printf("%ld", (a->childOffsets)[i]);
    }
    printf("]\n");

  printf("leaf files array: [");
  for(i=0; i< ((a->t)*2); i++){
    if(i!=0) printf(",");
    str = nameToFile((a->leafFiles)[i]);
    printf("'%s'", str);
    free(str);
  }
  printf("]\n");
  printf("\n========================================\n");
}

LeafNode *leafNodeCreate2M(int t){
  if(t<=0){
    printf("t = 0");
    return NULL;
  }

  LeafNode *new = (LeafNode*) malloc(sizeof(LeafNode));
  if(!new){
    printf("malloc error leafNodeCreate2M\n");
    exit(1);
  }

  new->t = t;
  new->numKeys = 0;

  new->nextLeafFile = (char*) malloc(sizeof(char)*5);
  if(!new->nextLeafFile){
    printf("malloc error nextLeafFile\n");
    exit(1);
  }

  strcpy(new->nextLeafFile, "XXXX");

  new->YearFileArray = (char**) malloc(sizeof(char*)*((2*t)-1));
  if(!new->YearFileArray){
    printf("malloc error YearFileArray\n");
    exit(1);
  }

  int i;
  for(i=0; i < (2*t)-1; i++){
    new->YearFileArray[i] = (char*) malloc(sizeof(char)*5);
    if(!new->YearFileArray[i]){
      printf("malloc error YearFileArray[i]\n");
      exit(1);
    }
    strcpy((new->YearFileArray)[i], "XXXX");
  }
  return new;
}

void leafNodeFree2M(LeafNode *l){
  int i;
  if(l){
    for(i = 0; i < (2*(l->t)) - 1; i++) free((l->YearFileArray)[i]);
    free(l->YearFileArray);
    free(l->nextLeafFile);
    free(l);
  }
}

void leafNodePrint(LeafNode *l){
  if(!l){
    printf("error printing leafNode");
    exit(1);
  }

  int i;
  printf("\n========================================\n");
  printf("t = %d\n", l->t);
  printf("numKeys = %d\n", (l->numKeys));
  char *leafNameWithExt = nameToFile(l->nextLeafFile);
  printf("nextLeafFile = '%s'\n", leafNameWithExt);
  free(leafNameWithExt);

  printf("year files array: [");
  for(i=0; i< ((l->t)*2)-1; i++){
    if(i!=0) printf(",");
    leafNameWithExt = nameToFile((l->YearFileArray)[i]);
    printf("'%s'", leafNameWithExt);
    free(leafNameWithExt);
  }
  printf("]\n");
  printf("\n========================================\n");
}

int isInLeaf(FILE *leafFile, int k){
  rewind(leafFile);
  LeafNode *leafNode = readLeafData(leafFile);
  int i, test = 0;
  
  char strk[5];
  snprintf(strk, sizeof(strk), "%04d", k);

  for(i=0; i < leafNode->numKeys; i++){
    if(strcmp(leafNode->YearFileArray[i], strk) == 0){
      test++;
      break;
    }
  }
  leafNodeFree2M(leafNode);
  return test;
}

char *bPlusTreeSearch2M(FILE *indexFileP, int k, long offset){
  if(!indexFileP || (offset == LONG_MAX)) return NULL;
  
  BPlusTree2M *node = readIndexNode(indexFileP, offset);
  if(!node){
    printf("error during search process");
    exit(1);
  };

  char *leafNameWithExt;
  int i = 0, test;

  while((i < node->numKeys) && (k > node->key[i])) i++;
  if(i < node->numKeys && k == node->key[i]) i++;

  if(node->isLeafsParent){
    leafNameWithExt = nameToFileInDirectory(node->leafFiles[i], bPlusTreeGetCurrentDirectory());
    if (!leafNameWithExt){
      printf("error accessing leafFileNames in a leaf parent");
      exit(1);
    }

    FILE *leafFile = fopen(leafNameWithExt, "rb");
    if(!leafFile){
      free(leafNameWithExt);
      bPlusTreeFree2M(node);
      return NULL;
    }

    test = isInLeaf(leafFile, k);
    fclose(leafFile);
    
    bPlusTreeFree2M(node);
    
    if(test) return leafNameWithExt;
    free(leafNameWithExt);
    return NULL;
  }
  long nextOffset = node->childOffsets[i];
  bPlusTreeFree2M(node);
  
  if(nextOffset == LONG_MAX){
    return NULL;
  }
  return bPlusTreeSearch2M(indexFileP, k, nextOffset);
}

long splitWithIndexChild2M(FILE *indexFile, long parentOffset, int newChildIndex, long fullNodeOffset, int t){
  BPlusTree2M *fullNode = readIndexNode(indexFile, fullNodeOffset);
  BPlusTree2M *brother =  bPlusTreeCreate2M(t);
  
  brother->isLeafsParent = fullNode->isLeafsParent;

  int j;

  brother->numKeys = t-1;
  for(j=0; j < (t-1); j++){
    brother->key[j] = fullNode->key[j+t];
    fullNode->key[j+t] = INT_MAX;
  }

  for(j=0; j < t; j++){
    brother->childOffsets[j] = fullNode->childOffsets[j+t];
    fullNode->childOffsets[j+t] = LONG_MAX;
  }

  if(fullNode->isLeafsParent){
    for(j=0; j < t; j++){
      strcpy(brother->leafFiles[j], fullNode->leafFiles[j+t]);
      strcpy(fullNode->leafFiles[j+t], "XXXX");
    }
  }

  fullNode->numKeys = t-1;

  if(fseek(indexFile, 0L, SEEK_END) != 0){
    printf("fseek error");
    exit(1);
  }
  long brotherOffset = ftell(indexFile);

  if(writeIndexNode(indexFile, brother, brotherOffset) != 1){
    printf("error writing to file");
    exit(1);
  }
  fflush(indexFile);
  bPlusTreeFree2M(brother);
  
  BPlusTree2M *parent = readIndexNode(indexFile, parentOffset);

  for(j = parent->numKeys; j >= newChildIndex; j--) parent->childOffsets[j+1] = parent->childOffsets[j];
  parent->childOffsets[newChildIndex] = brotherOffset;

  for(j = parent->numKeys; j >= newChildIndex; j--) parent->key[j] = parent->key[j-1];
  parent->key[newChildIndex-1] = fullNode->key[t-1];
  fullNode->key[t-1] = INT_MAX;
  parent->numKeys++;
  
  writeIndexNode(indexFile, fullNode, fullNodeOffset);
  writeIndexNode(indexFile, parent, parentOffset);
  fflush(indexFile);
  bPlusTreeFree2M(fullNode);
  bPlusTreeFree2M(parent);

  return parentOffset;
}

long splitWithLeafChild2M(FILE *indexFile, long parentOffset, int newLeafIndex, FILE *fullLeafFile, int t){
  if(!fullLeafFile){
    printf("error  opening leaf");
    exit(1);
  }

  rewind(fullLeafFile);
  LeafNode *fullLeaf = readLeafData(fullLeafFile);
  LeafNode *newLeaf  = leafNodeCreate2M(t);

  newLeaf->numKeys = t;
  int j;
  char *newLeafName, *newLeafNameWithExt;

  for(j=0; j < t; j++){
    strcpy(newLeaf->YearFileArray[j], fullLeaf->YearFileArray[j+t-1]);
    strcpy(fullLeaf->YearFileArray[j+t-1], "XXXX");
  }

  strcpy(newLeaf->nextLeafFile, fullLeaf->nextLeafFile);

  newLeafName = newFileName(indexFile);
  newLeafNameWithExt = nameToFileInDirectory(newLeafName, bPlusTreeGetCurrentDirectory());

  FILE *newLeafFile = fopen(newLeafNameWithExt, "wb");
  if(!newLeafFile){
    printf("error creating new leaf");
    exit(1);
  }
  writeLeafData(newLeaf, newLeafFile);
  fclose(newLeafFile);

  strcpy(fullLeaf->nextLeafFile, newLeafName);
  fullLeaf->numKeys = t-1;
  
  if(fseek(fullLeafFile, 0L, SEEK_SET) != 0){
    printf("error with fseek in splitWithLeafChild2M");
    exit(1);
  }

  writeLeafData(fullLeaf, fullLeafFile);
  fflush(fullLeafFile);
  fclose(fullLeafFile);

  BPlusTree2M *parent = readIndexNode(indexFile, parentOffset);

  int limit = (2 * parent->t) - 1;
  for(j = parent->numKeys; j >= newLeafIndex && j+1 <= limit; j--) strcpy(parent->leafFiles[j+1],parent->leafFiles[j]);
  strcpy(parent->leafFiles[newLeafIndex], newLeafName);

  for(j = parent->numKeys; j >= newLeafIndex; j--) parent->key[j] = parent->key[j-1];
  parent->key[newLeafIndex-1] = (int)atol(newLeaf->YearFileArray[0]);
  parent->numKeys++;

  writeIndexNode(indexFile, parent, parentOffset);
  fflush(indexFile);

  free(newLeafName);
  free(newLeafNameWithExt);
  leafNodeFree2M(newLeaf);
  leafNodeFree2M(fullLeaf);
  bPlusTreeFree2M(parent);

  return parentOffset;
}

int insertNonFullLeaf2M(FILE *nonFullLeafFile, int key){
  if(!nonFullLeafFile){
    printf("error  opening leaf");
    exit(1);
  }
  rewind(nonFullLeafFile);

  LeafNode *leafNode = readLeafData(nonFullLeafFile);
  int i = leafNode->numKeys - 1;

  for(int k = 0; k < leafNode->numKeys; k++){
    if(atol(leafNode->YearFileArray[k]) == key){
      leafNodeFree2M(leafNode);
      return 0;
    }
  }

  while((i>=0) && (key < atol(leafNode->YearFileArray[i]) )){
    memmove(leafNode->YearFileArray[i+1], leafNode->YearFileArray[i], 5);
    i--;
  }

  char strkey[5];
  snprintf(strkey, sizeof(strkey), "%04d", key);

  strcpy(leafNode->YearFileArray[i+1], strkey);
  leafNode->numKeys++;

  rewind(nonFullLeafFile);
  writeLeafData(leafNode, nonFullLeafFile);

  leafNodeFree2M(leafNode);

  return 1;
}

long insertNonFullNode2M(FILE *indexFile, long nonFullNodeOffset, int key, int t){
  BPlusTree2M *nonFullNode = readIndexNode(indexFile, nonFullNodeOffset);
  int i = nonFullNode->numKeys-1;

  while((i>=0) && (key < nonFullNode->key[i])) i--;
  i++;

  if(nonFullNode->isLeafsParent){
    char *leafFileNameWithExt = nameToFileInDirectory(nonFullNode->leafFiles[i], bPlusTreeGetCurrentDirectory()); 
    FILE *leafFile = fopen(leafFileNameWithExt, "rb+");
    if(!leafFile){
      printf("error inserting key");
      exit(1);
    }
    free(leafFileNameWithExt);

    int leafNK;
    if(fseek(leafFile, 4L, SEEK_SET) != 0){
      printf("error reading in function insertNonFullNode2M");
      exit(1);
    }
    if(fread(&leafNK, sizeof(int), 1, leafFile) != 1){
      printf("error reading in function insertNonFullNode2M");
      exit(1);
    }
    rewind(leafFile);

    if(leafNK == ((2*t)-1)){
      rewind(leafFile);
      if(isInLeaf(leafFile, key)){
        fclose(leafFile);
        bPlusTreeFree2M(nonFullNode);
        return nonFullNodeOffset;
      }
      rewind(leafFile);
      bPlusTreeFree2M(nonFullNode);
      if(splitWithLeafChild2M(indexFile, nonFullNodeOffset, (i+1), leafFile, t) != nonFullNodeOffset){
        printf("error spliting leaf");
        exit(1);
      }
      nonFullNode = readIndexNode(indexFile, nonFullNodeOffset);
      int j = 0;
      while ((j < nonFullNode->numKeys) && (key > nonFullNode->key[j])) j++;
      if (j < nonFullNode->numKeys && key == nonFullNode->key[j]) j++;
      i = j;

      leafFileNameWithExt = nameToFileInDirectory(nonFullNode->leafFiles[i], bPlusTreeGetCurrentDirectory()); 
      leafFile = fopen(leafFileNameWithExt, "rb+");
      if(!leafFile){
        printf("error inserting key");
        exit(1);
      }
      free(leafFileNameWithExt);
    }
    if(!insertNonFullLeaf2M(leafFile, key)){
      fclose(leafFile);
      bPlusTreeFree2M(nonFullNode);
      return nonFullNodeOffset;
    }
    fclose(leafFile);
    bPlusTreeFree2M(nonFullNode);
  }
  else{
    fseek(indexFile, (nonFullNode->childOffsets[i]) + 4L, SEEK_SET);
    int nodeNK;
    if(fread(&nodeNK, sizeof(int), 1, indexFile) != 1){
      printf("error in insertNonFullNode2M with isleaParent == 0");
      exit(1);
    }
    if(nodeNK == ((2*t)-1)){
      if(splitWithIndexChild2M(indexFile, nonFullNodeOffset, (i+1), nonFullNode->childOffsets[i], t) != nonFullNodeOffset){
        printf("error spliting node");
        exit(1);
      }
      bPlusTreeFree2M(nonFullNode);
      nonFullNode = readIndexNode(indexFile, nonFullNodeOffset);
      int j = 0;
      while ((j < nonFullNode->numKeys) && (key > nonFullNode->key[j])) j++;
      if (j < nonFullNode->numKeys && key == nonFullNode->key[j]) j++;
      i = j;
    }
    insertNonFullNode2M(indexFile, nonFullNode->childOffsets[i], key, t);
    bPlusTreeFree2M(nonFullNode);
  }
  return nonFullNodeOffset;
}

int bPlusTreeInsert2M(FILE *indexFile, int key, int t){
  long rootOffset = getRootOffset(indexFile);
  long nonFullIndexOffset = rootOffset;
  BPlusTree2M *indexNode = readIndexNode(indexFile, rootOffset);
  FILE *leafFile;
  char *leafFileNameWithExt;
  LeafNode *leafNode;
  long end;

  if(indexNode->numKeys == 0){
    if(strcmp(indexNode->leafFiles[0], "XXXX") == 0){
      leafNode = leafNodeCreate2M(t);

      char newYear[6];
      snprintf(newYear, 5, "%04d", key);
      newYear[4] = '\0';

      strcpy(leafNode->YearFileArray[0], newYear);
      (leafNode->numKeys)++;

      char *newLeafName = newFileName(indexFile);
      char *newLeafNameWithExt = nameToFileInDirectory(newLeafName, bPlusTreeGetCurrentDirectory());

      leafFile = fopen(newLeafNameWithExt, "wb");
      if(!leafFile){
        printf("erro creating new leaf in bPlusTreeInsert2M");
        exit(1);
      }


      strcpy(indexNode->leafFiles[0], newLeafName);
      writeLeafData(leafNode, leafFile);
      writeIndexNode(indexFile, indexNode, nonFullIndexOffset);
      bPlusTreeFree2M(indexNode);
      free(newLeafName);
      free(newLeafNameWithExt);
      fclose(leafFile);
      leafNodeFree2M(leafNode);
      return 1;
    }
    else{
      leafFileNameWithExt = nameToFileInDirectory(indexNode->leafFiles[0], bPlusTreeGetCurrentDirectory());
      leafFile = fopen(leafFileNameWithExt, "rb+");
      if(!leafFile){
        printf("error opening leaf in bPlusTreeInsert2M");
        exit(1);
      }
      if(isInLeaf(leafFile, key)){
        fclose(leafFile);
        free(leafFileNameWithExt);
        bPlusTreeFree2M(indexNode);
        return 0;
      }

      leafNode = readLeafData(leafFile);

      if(leafNode->numKeys == (2*t)-1){
        splitWithLeafChild2M(indexFile, rootOffset, 1, leafFile, t);
        leafNodeFree2M(leafNode); //burrao 2 horas caçando vazamento pra ser isso =/
        bPlusTreeFree2M(indexNode);
        indexNode = readIndexNode(indexFile, rootOffset);
        free(leafFileNameWithExt);
       {
          int j = 0;
          while ((j < indexNode->numKeys) && (key > indexNode->key[j])) j++;
          if (j < indexNode->numKeys && key == indexNode->key[j]) j++;
          leafFileNameWithExt = nameToFileInDirectory(indexNode->leafFiles[j], bPlusTreeGetCurrentDirectory());
        }
        leafFile = fopen(leafFileNameWithExt, "rb+");
        if(!leafFile){
          printf("error opening leaf after split in bPlusTreeInsert2M");
          exit(1);
        }
        leafNode = readLeafData(leafFile);
      }
      if(!insertNonFullLeaf2M(leafFile, key)){
        leafNodeFree2M(leafNode);
        fclose(leafFile);
        bPlusTreeFree2M(indexNode);
        free(leafFileNameWithExt);
        return 0;
      }

      leafNodeFree2M(leafNode);
      fclose(leafFile);
      bPlusTreeFree2M(indexNode);
      free(leafFileNameWithExt);
      return 1;
    }
  }
  else{
    if(bPlusTreeSearch2M(indexFile, key, rootOffset)){
      bPlusTreeFree2M(indexNode);
      return 0;
    }
    if(indexNode->numKeys == (2*t)-1){
      BPlusTree2M *newRoot = bPlusTreeCreate2M(t);
      newRoot->numKeys = 0;
      newRoot->isLeafsParent = 0;
      newRoot->childOffsets[0] = rootOffset;
      if(fseek(indexFile, 0L, SEEK_END) != 0){
        printf("bPlusTreeInsert2M: fseek error");
        exit(1);
      }
      end = ftell(indexFile);
      writeIndexNode(indexFile, newRoot, end);
      fflush(indexFile);

      splitWithIndexChild2M(indexFile, end, 1, rootOffset, t);
      bPlusTreeFree2M(indexNode);

      setRootOffset(indexFile, end);
    
      fflush(indexFile);
      insertNonFullNode2M(indexFile, end, key, t);
      bPlusTreeFree2M(newRoot);
      return 1;
    }
    insertNonFullNode2M(indexFile, nonFullIndexOffset, key, t);
    bPlusTreeFree2M(indexNode);
    return 1;
  }
}

int removeFromLeaf(FILE *leafFile, int key){
  if(!leafFile) return 0;
  
  rewind(leafFile);
  LeafNode *leaf = readLeafData(leafFile);
  
  int i, found = -1;
  char strkey[5];
  snprintf(strkey, sizeof(strkey), "%04d", key);
  
  for(i = 0; i < leaf->numKeys; i++){
    if(strcmp(leaf->YearFileArray[i], strkey) == 0){
      found = i;
      break;
    }
  }
  
  if(found == -1){
    leafNodeFree2M(leaf);
    return 0;
  }
  
  char *yearFileName = nameToFileInDirectory(leaf->YearFileArray[found], bPlusTreeGetCurrentDirectory());
  if(yearFileName){
    remove(yearFileName);
    free(yearFileName);
  }
  
  for(i = found; i < leaf->numKeys - 1; i++){
    strcpy(leaf->YearFileArray[i], leaf->YearFileArray[i+1]);
  }
  strcpy(leaf->YearFileArray[leaf->numKeys - 1], "XXXX");
  leaf->numKeys--;
  
  rewind(leafFile);
  writeLeafData(leaf, leafFile);
  fflush(leafFile);
  
  leafNodeFree2M(leaf);
  return 1;
}

int getMinKeyFromLeaf(FILE *leafFile){
  if(!leafFile) return -1;
  
  rewind(leafFile);
  LeafNode *leaf = readLeafData(leafFile);
  
  if(leaf->numKeys == 0){
    leafNodeFree2M(leaf);
    return -1;
  }
  
  int minKey = atol(leaf->YearFileArray[0]);
  leafNodeFree2M(leaf);
  return minKey;
}

int borrowFromLeftLeaf(FILE *leftLeafFile, FILE *currentLeafFile, FILE *indexFile, long parentOffset, int parentKeyIndex, int t){
  if(!leftLeafFile || !currentLeafFile) return 0;
  
  rewind(leftLeafFile);
  rewind(currentLeafFile);
  
  LeafNode *leftLeaf = readLeafData(leftLeafFile);
  LeafNode *currentLeaf = readLeafData(currentLeafFile);
  
  if(leftLeaf->numKeys <= t-1){
    leafNodeFree2M(leftLeaf);
    leafNodeFree2M(currentLeaf);
    return 0;
  }
  
  int i;
  for(i = currentLeaf->numKeys; i > 0; i--){
    strcpy(currentLeaf->YearFileArray[i], currentLeaf->YearFileArray[i-1]);
  }
  strcpy(currentLeaf->YearFileArray[0], leftLeaf->YearFileArray[leftLeaf->numKeys - 1]);
  strcpy(leftLeaf->YearFileArray[leftLeaf->numKeys - 1], "XXXX");
  
  currentLeaf->numKeys++;
  leftLeaf->numKeys--;
  
  rewind(leftLeafFile);
  writeLeafData(leftLeaf, leftLeafFile);
  fflush(leftLeafFile);
  
  rewind(currentLeafFile);
  writeLeafData(currentLeaf, currentLeafFile);
  fflush(currentLeafFile);
  
  BPlusTree2M *parent = readIndexNode(indexFile, parentOffset);
  parent->key[parentKeyIndex] = atol(currentLeaf->YearFileArray[0]);
  writeIndexNode(indexFile, parent, parentOffset);
  fflush(indexFile);
  bPlusTreeFree2M(parent);
  
  leafNodeFree2M(leftLeaf);
  leafNodeFree2M(currentLeaf);
  return 1;
}

int borrowFromRightLeaf(FILE *rightLeafFile, FILE *currentLeafFile, FILE *indexFile, long parentOffset, int parentKeyIndex, int t){
  if(!rightLeafFile || !currentLeafFile) return 0;
  
  rewind(rightLeafFile);
  rewind(currentLeafFile);
  
  LeafNode *rightLeaf = readLeafData(rightLeafFile);
  LeafNode *currentLeaf = readLeafData(currentLeafFile);
  
  if(rightLeaf->numKeys <= t-1){
    leafNodeFree2M(rightLeaf);
    leafNodeFree2M(currentLeaf);
    return 0;
  }
  
  strcpy(currentLeaf->YearFileArray[currentLeaf->numKeys], rightLeaf->YearFileArray[0]);
  currentLeaf->numKeys++;
  
  int i;
  for(i = 0; i < rightLeaf->numKeys - 1; i++){
    strcpy(rightLeaf->YearFileArray[i], rightLeaf->YearFileArray[i+1]);
  }
  strcpy(rightLeaf->YearFileArray[rightLeaf->numKeys - 1], "XXXX");
  rightLeaf->numKeys--;
  
  rewind(rightLeafFile);
  writeLeafData(rightLeaf, rightLeafFile);
  fflush(rightLeafFile);
  
  rewind(currentLeafFile);
  writeLeafData(currentLeaf, currentLeafFile);
  fflush(currentLeafFile);
  
  BPlusTree2M *parent = readIndexNode(indexFile, parentOffset);
  parent->key[parentKeyIndex] = atol(rightLeaf->YearFileArray[0]);
  writeIndexNode(indexFile, parent, parentOffset);
  fflush(indexFile);
  bPlusTreeFree2M(parent);
  
  leafNodeFree2M(rightLeaf);
  leafNodeFree2M(currentLeaf);
  return 1;
}

int mergeLeaves(FILE *leftLeafFile, FILE *rightLeafFile, FILE *indexFile, long parentOffset, int parentKeyIndex, int t){
  if(!leftLeafFile || !rightLeafFile) return 0;
  
  rewind(leftLeafFile);
  rewind(rightLeafFile);
  
  LeafNode *leftLeaf = readLeafData(leftLeafFile);
  LeafNode *rightLeaf = readLeafData(rightLeafFile);
  
  int i;
  for(i = 0; i < rightLeaf->numKeys; i++){
    strcpy(leftLeaf->YearFileArray[leftLeaf->numKeys + i], rightLeaf->YearFileArray[i]);
  }
  leftLeaf->numKeys += rightLeaf->numKeys;
  
  strcpy(leftLeaf->nextLeafFile, rightLeaf->nextLeafFile);
  
  rewind(leftLeafFile);
  writeLeafData(leftLeaf, leftLeafFile);
  fflush(leftLeafFile);
  
  BPlusTree2M *parent = readIndexNode(indexFile, parentOffset);
  
  for(i = parentKeyIndex; i < parent->numKeys - 1; i++){
    parent->key[i] = parent->key[i+1];
  }
  parent->key[parent->numKeys - 1] = INT_MAX;
  
  for(i = parentKeyIndex + 1; i < parent->numKeys; i++){
    strcpy(parent->leafFiles[i], parent->leafFiles[i+1]);
  }
  strcpy(parent->leafFiles[parent->numKeys], "XXXX");
  
  parent->numKeys--;
  
  writeIndexNode(indexFile, parent, parentOffset);
  fflush(indexFile);
  bPlusTreeFree2M(parent);
  
  leafNodeFree2M(leftLeaf);
  leafNodeFree2M(rightLeaf);
  return 1;
}

long removeFromNode(FILE *indexFile, long nodeOffset, int key, int t);

long handleLeafUnderflow(FILE *indexFile, long parentOffset, int leafIndex, int key, int t){
  BPlusTree2M *parent = readIndexNode(indexFile, parentOffset);
  
  char *currentLeafName = nameToFileInDirectory(parent->leafFiles[leafIndex], bPlusTreeGetCurrentDirectory());
  FILE *currentLeafFile = fopen(currentLeafName, "rb+");
  free(currentLeafName);
  
  if(!currentLeafFile){
    bPlusTreeFree2M(parent);
    return parentOffset;
  }
  
  rewind(currentLeafFile);
  LeafNode *currentLeaf = readLeafData(currentLeafFile);
  
  if(currentLeaf->numKeys >= t){
    leafNodeFree2M(currentLeaf);
    fclose(currentLeafFile);
    bPlusTreeFree2M(parent);
    return parentOffset;
  }
  
  leafNodeFree2M(currentLeaf);
  
  if(leafIndex > 0){
    char *leftLeafName = nameToFileInDirectory(parent->leafFiles[leafIndex - 1], bPlusTreeGetCurrentDirectory());
    FILE *leftLeafFile = fopen(leftLeafName, "rb+");
    free(leftLeafName);
    
    if(leftLeafFile){
      if(borrowFromLeftLeaf(leftLeafFile, currentLeafFile, indexFile, parentOffset, leafIndex - 1, t)){
        fclose(leftLeafFile);
        fclose(currentLeafFile);
        bPlusTreeFree2M(parent);
        return parentOffset;
      }
      fclose(leftLeafFile);
    }
  }
  
  if(leafIndex < parent->numKeys){
    char *rightLeafName = nameToFileInDirectory(parent->leafFiles[leafIndex + 1], bPlusTreeGetCurrentDirectory());
    FILE *rightLeafFile = fopen(rightLeafName, "rb+");
    free(rightLeafName);
    
    if(rightLeafFile){
      if(borrowFromRightLeaf(rightLeafFile, currentLeafFile, indexFile, parentOffset, leafIndex, t)){
        fclose(rightLeafFile);
        fclose(currentLeafFile);
        bPlusTreeFree2M(parent);
        return parentOffset;
      }
      fclose(rightLeafFile);
    }
  }
  
  if(leafIndex > 0){
    char *leftLeafName = nameToFileInDirectory(parent->leafFiles[leafIndex - 1], bPlusTreeGetCurrentDirectory());
    FILE *leftLeafFile = fopen(leftLeafName, "rb+");
    free(leftLeafName);
    
    if(leftLeafFile){
      mergeLeaves(leftLeafFile, currentLeafFile, indexFile, parentOffset, leafIndex - 1, t);
      fclose(leftLeafFile);
    }
  }
  else if(leafIndex < parent->numKeys){
    char *rightLeafName = nameToFileInDirectory(parent->leafFiles[leafIndex + 1], bPlusTreeGetCurrentDirectory());
    FILE *rightLeafFile = fopen(rightLeafName, "rb+");
    free(rightLeafName);
    
    if(rightLeafFile){
      mergeLeaves(currentLeafFile, rightLeafFile, indexFile, parentOffset, leafIndex, t);
      fclose(rightLeafFile);
    }
  }
  
  fclose(currentLeafFile);
  bPlusTreeFree2M(parent);
  
  BPlusTree2M *updatedParent = readIndexNode(indexFile, parentOffset);
  if(updatedParent->numKeys == 0 && parentOffset == getRootOffset(indexFile)){
    bPlusTreeFree2M(updatedParent);
    return parentOffset;
  }
  bPlusTreeFree2M(updatedParent);
  
  return parentOffset;
}

long removeFromNode(FILE *indexFile, long nodeOffset, int key, int t){
  if(nodeOffset == LONG_MAX) return LONG_MAX;
  
  BPlusTree2M *node = readIndexNode(indexFile, nodeOffset);
  
  int i = 0;
  while(i < node->numKeys && key > node->key[i]) i++;
  if(i < node->numKeys && key == node->key[i]) i++;
  
  if(node->isLeafsParent){
    char *leafName = nameToFileInDirectory(node->leafFiles[i], bPlusTreeGetCurrentDirectory());
    FILE *leafFile = fopen(leafName, "rb+");
    free(leafName);
    
    if(!leafFile){
      bPlusTreeFree2M(node);
      return nodeOffset;
    }
    
    if(!removeFromLeaf(leafFile, key)){
      fclose(leafFile);
      bPlusTreeFree2M(node);
      return nodeOffset;
    }
    
    fclose(leafFile);
    bPlusTreeFree2M(node);
    
    return handleLeafUnderflow(indexFile, nodeOffset, i, key, t);
  }
  else{
    long childOffset = node->childOffsets[i];
    bPlusTreeFree2M(node);
    
    removeFromNode(indexFile, childOffset, key, t);
    return nodeOffset;
  }
}

int bPlusTreeRemove2M(FILE *indexFile, int key, int t){
  if(!indexFile) return 0;
  
  long rootOffset = getRootOffset(indexFile);
  
  char *result = bPlusTreeSearch2M(indexFile, key, rootOffset);
  if(!result){
    return 0;
  }
  free(result);
  
  removeFromNode(indexFile, rootOffset, key, t);
  
  BPlusTree2M *root = readIndexNode(indexFile, rootOffset);
  if(root->numKeys == 0 && !root->isLeafsParent && root->childOffsets[0] != LONG_MAX){
    setRootOffset(indexFile, root->childOffsets[0]);
  }
  bPlusTreeFree2M(root);
  
  return 1;
}

char **bPlusTreeRangeSearch2M(FILE *indexFile, int minKey, int maxKey, int *resultCount){
  if(!indexFile || minKey > maxKey){
    *resultCount = 0;
    return NULL;
  }
  
  long rootOffset = getRootOffset(indexFile);
  char *firstLeafName = bPlusTreeSearch2M(indexFile, minKey, rootOffset);
  
  if(!firstLeafName){
    BPlusTree2M *root = readIndexNode(indexFile, rootOffset);
    if(root->isLeafsParent && strcmp(root->leafFiles[0], "XXXX") != 0){
      firstLeafName = nameToFileInDirectory(root->leafFiles[0], bPlusTreeGetCurrentDirectory());
    }
    else{
      bPlusTreeFree2M(root);
      *resultCount = 0;
      return NULL;
    }
    bPlusTreeFree2M(root);
  }
  
  char **results = (char**)malloc(sizeof(char*) * 1000);
  int count = 0;
  
  char *currentLeafName = firstLeafName;
  
  while(currentLeafName && strcmp(currentLeafName, "XXXX.dat") != 0){
    FILE *leafFile = fopen(currentLeafName, "rb");
    if(!leafFile) break;
    
    LeafNode *leaf = readLeafData(leafFile);
    fclose(leafFile);
    
    for(int i = 0; i < leaf->numKeys; i++){
      int yearKey = atol(leaf->YearFileArray[i]);
      if(yearKey >= minKey && yearKey <= maxKey){
        results[count] = (char*)malloc(sizeof(char) * 5);
        strcpy(results[count], leaf->YearFileArray[i]);
        count++;
      }
      if(yearKey > maxKey) break;
    }
    
    char *nextLeafName = NULL;
    if(strcmp(leaf->nextLeafFile, "XXXX") != 0){
      nextLeafName = nameToFileInDirectory(leaf->nextLeafFile, bPlusTreeGetCurrentDirectory());
    }
    
    leafNodeFree2M(leaf);
    if(currentLeafName != firstLeafName) free(currentLeafName);
    currentLeafName = nextLeafName;
    
    if(currentLeafName){
      FILE *testFile = fopen(currentLeafName, "rb");
      if(!testFile){
        free(currentLeafName);
        currentLeafName = NULL;
        break;
      }
      LeafNode *testLeaf = readLeafData(testFile);
      fclose(testFile);
      if(testLeaf->numKeys > 0 && atol(testLeaf->YearFileArray[0]) > maxKey){
        leafNodeFree2M(testLeaf);
        free(currentLeafName);
        currentLeafName = NULL;
        break;
      }
      leafNodeFree2M(testLeaf);
    }
  }
  
  if(currentLeafName && currentLeafName != firstLeafName) free(currentLeafName);
  if(firstLeafName) free(firstLeafName);
  
  *resultCount = count;
  
  if(count == 0){
    free(results);
    return NULL;
  }
  
  return results;
}

void bPlusTreePrintAll2MAux(FILE *indexFile, long nodeOffset, int level, int t);

void bPlusTreePrintAll2M(FILE *indexFile, int t){
  if(!indexFile) return;
  
  long rootOffset = getRootOffset(indexFile);
  bPlusTreePrintAll2MAux(indexFile, rootOffset, 0, t);
}

void bPlusTreePrintAll2MAux(FILE *indexFile, long nodeOffset, int level, int t){
  if(nodeOffset == LONG_MAX) return;
  
  BPlusTree2M *node = readIndexNode(indexFile, nodeOffset);
  if(!node) return;
  
  int i, j;
  
  if(node->isLeafsParent){
    bPlusTreePrintAll2MAux(indexFile, node->childOffsets[node->numKeys], level + 1, t);
    
    char *leafFileName = node->leafFiles[node->numKeys];
    if(strcmp(leafFileName, "XXXX") != 0){
      char *leafFileNameWithExt = nameToFileInDirectory(leafFileName, bPlusTreeGetCurrentDirectory());
      FILE *leafFile = fopen(leafFileNameWithExt, "rb");
      if(leafFile){
        LeafNode *leaf = readLeafData(leafFile);
        if(leaf){
          for(int k = leaf->numKeys - 1; k >= 0; k--){
            for(j = 0; j < level; j++) printf("      ");
            printf("      %s\n", leaf->YearFileArray[k]);
          }
          leafNodeFree2M(leaf);
        }
        fclose(leafFile);
      }
      free(leafFileNameWithExt);
    }
    
    for(i = node->numKeys - 1; i >= 0; i--){
      for(j = 0; j < level; j++) printf("      ");
      printf("%d\n\n", node->key[i]);
      
      leafFileName = node->leafFiles[i];
      if(strcmp(leafFileName, "XXXX") != 0){
        char *leafFileNameWithExt = nameToFileInDirectory(leafFileName, bPlusTreeGetCurrentDirectory());
        FILE *leafFile = fopen(leafFileNameWithExt, "rb");
        if(leafFile){
          LeafNode *leaf = readLeafData(leafFile);
          if(leaf){
            for(int k = leaf->numKeys - 1; k >= 0; k--){
              for(j = 0; j < level; j++) printf("      ");
              printf("      %s\n", leaf->YearFileArray[k]);
            }
            leafNodeFree2M(leaf);
          }
          fclose(leafFile);
        }
        free(leafFileNameWithExt);
      }
      
      bPlusTreePrintAll2MAux(indexFile, node->childOffsets[i], level + 1, t);
    }
  } else{
    bPlusTreePrintAll2MAux(indexFile, node->childOffsets[node->numKeys], level + 1, t);
    for(i = node->numKeys - 1; i >= 0; i--){
      for(j = 0; j < level; j++) printf("      ");
      printf("%d\n\n", node->key[i]);
      bPlusTreePrintAll2MAux(indexFile, node->childOffsets[i], level + 1, t);
    }
  }
  
  bPlusTreeFree2M(node);
}

void bPlusTreeForEach(FILE *indexFile, int t, BPlusTreeIteratorCallback callback, void *userData){
    if (!indexFile || !callback) return;
    
    long rootOffset = getRootOffset(indexFile);
    BPlusTree2M *root = readIndexNode(indexFile, rootOffset);
    
    if (!root || root->numKeys == 0){
        if (root) bPlusTreeFree2M(root);
        return;
    }
    
    char *firstLeafName = NULL;
    if (root->isLeafsParent && strcmp(root->leafFiles[0], "XXXX") != 0){
        firstLeafName = nameToFileInDirectory(root->leafFiles[0], bPlusTreeGetCurrentDirectory());
    } else if (!root->isLeafsParent){
        BPlusTree2M *current = root;
        long currentOffset = rootOffset;
        
        while (!current->isLeafsParent && current->childOffsets[0] != LONG_MAX){
            long nextOffset = current->childOffsets[0];
            bPlusTreeFree2M(current);
            current = readIndexNode(indexFile, nextOffset);
            currentOffset = nextOffset;
        }
        
        if (current->isLeafsParent && strcmp(current->leafFiles[0], "XXXX") != 0){
            firstLeafName = nameToFileInDirectory(current->leafFiles[0], bPlusTreeGetCurrentDirectory());
        }
        
        if (current != root) bPlusTreeFree2M(current);
    }
    
    if (root != NULL) bPlusTreeFree2M(root);
    
    if (!firstLeafName) return;
    
    char *currentLeafName = firstLeafName;
    
    while (currentLeafName && strcmp(currentLeafName, "XXXX.dat") != 0){
        FILE *leafFile = fopen(currentLeafName, "rb");
        if (!leafFile){
            free(currentLeafName);
            break;
        }
        
        LeafNode *leaf = readLeafData(leafFile);
        fclose(leafFile);
        
        if (leaf){
            for (int i = 0; i < leaf->numKeys; i++){
                int year = atol(leaf->YearFileArray[i]);
                char *yearFilePath = nameToFileInDirectory(leaf->YearFileArray[i], bPlusTreeGetCurrentDirectory());
                if (yearFilePath){
                    callback(year, yearFilePath, userData);
                    free(yearFilePath);
                }
            }
            
            char *nextLeafName = NULL;
            if (strcmp(leaf->nextLeafFile, "XXXX") != 0){
                nextLeafName = nameToFileInDirectory(leaf->nextLeafFile, bPlusTreeGetCurrentDirectory());
            }
            
            leafNodeFree2M(leaf);
            free(currentLeafName);
            currentLeafName = nextLeafName;
        } else{
            free(currentLeafName);
            break;
        }
    }
}
