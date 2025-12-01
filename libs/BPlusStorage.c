#ifndef B_PLUS_STORAGE_C
#define B_PLUS_STORAGE_C

#include "BPlusStorage.h"
#include "BPlusTree2M.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static char *currentDirectory = NULL;

void bPlusTreeSetCurrentDirectory(const char *directory) {
    if (currentDirectory) {
        free(currentDirectory);
    }
    currentDirectory = directory ? strdup(directory) : NULL;
}

const char* bPlusTreeGetCurrentDirectory() {
    return currentDirectory;
}

char* buildFilePath(const char *directory, const char *filename) {
    size_t dirLen = strlen(directory);
    size_t fileLen = strlen(filename);
    size_t totalLen = dirLen + fileLen + 2;
    
    char *fullPath = (char*)malloc(totalLen);
    if (!fullPath) return NULL;
    
    snprintf(fullPath, totalLen, "%s/%s", directory, filename);
    return fullPath;
}

int directoryExists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

int createDirectory(const char *path) {
    if (directoryExists(path)) return 1;
    
    if (mkdir(path, 0755) == 0) return 1;
    
    if (errno == EEXIST) return 1;
    
    return 0;
}

BPlusTreeContext* bPlusTreeLoad(const char *directoryPath, int t) {
    if (!directoryPath || t <= 0) return NULL;
    
    BPlusTreeContext *context = (BPlusTreeContext*)malloc(sizeof(BPlusTreeContext));
    if (!context) return NULL;
    
    context->directoryPath = strdup(directoryPath);
    context->t = t;
    
    if (!createDirectory(directoryPath)) {
        free(context->directoryPath);
        free(context);
        return NULL;
    }
    
    bPlusTreeSetCurrentDirectory(directoryPath);
    
    char *indexPath = buildFilePath(directoryPath, "Index.dat");
    if (!indexPath) {
        free(context->directoryPath);
        free(context);
        return NULL;
    }
    
    context->indexFilePath = indexPath;
    
    FILE *testFile = fopen(indexPath, "rb");
    if (testFile) {
        fclose(testFile);
        context->indexFile = fopen(indexPath, "rb+");
    } else {
        context->indexFile = fopen(indexPath, "wb+");
        if (context->indexFile) {
            int fileCounter = 0;
            long root = HEADER_SIZE;
            
            fwrite(&fileCounter, sizeof(int), 1, context->indexFile);
            fwrite(&root, sizeof(long), 1, context->indexFile);
            
            BPlusTree2M *firstNode = bPlusTreeCreate2M(t);
            writeIndexNode(context->indexFile, firstNode, root);
            bPlusTreeFree2M(firstNode);
            fflush(context->indexFile);
        }
    }
    
    if (!context->indexFile) {
        free(context->indexFilePath);
        free(context->directoryPath);
        free(context);
        return NULL;
    }
    
    return context;
}

void bPlusTreeContextFree(BPlusTreeContext *context) {
    if (context) {
        if (context->indexFile) {
            fflush(context->indexFile);
            fclose(context->indexFile);
        }
        free(context->indexFilePath);
        free(context->directoryPath);
        free(context);
    }
}

char *nameToFile(char *fileName){

    char *str = (char*) malloc(sizeof(char)*10);
    strcpy(str, fileName);
    strcat(str, ".dat");

    return str;
}

char *nameToFileInDirectory(char *fileName, const char *directory) {
    char *baseName = nameToFile(fileName);
    if (!baseName) return NULL;
    
    if (!directory) return baseName;
    
    char *fullPath = buildFilePath(directory, baseName);
    free(baseName);
    return fullPath;
}

char *newFileName(FILE *indexFile){
    int counter;
    if(!indexFile){
        printf("newFileName: indexFile == NULL");
        exit(1);
    }
    if(fseek(indexFile, FILECOUNTER_OFFSET, SEEK_SET) != 0) exit(1);
    if(fread(&counter, sizeof(int), 1, indexFile) != 1) counter = 0;
    counter++;
    if(fseek(indexFile, FILECOUNTER_OFFSET, SEEK_SET) != 0) exit(1);
    if(fwrite(&counter, sizeof(int), 1, indexFile) != 1) exit(1);
    fflush(indexFile);

    char *newName = (char*) malloc(sizeof(char)*5);
    snprintf(newName, 5, "%04d", counter);

    return newName;


}

long bPlusTree2MSizeInDisk(int t){
    long total = 0;
    total += (sizeof(int)*3) + (sizeof(int)*((2*t)-1)) + (sizeof(long)*(2*t)) + ((sizeof(char)*5)*(2*t));
    return total;
}

long getRootOffset(FILE *indexFile){
    long root;
    if(!indexFile) {
        printf("getRootOffset: indexFile NULL\n");
        exit(1);
    }
    if(fseek(indexFile, ROOTOFFSET_OFFSET, SEEK_SET) != 0){
        printf("getRootOffset: fseek failed\n"); 
        exit(1);
    }
    if(fread(&root, sizeof(long), 1, indexFile) != 1){
        root = HEADER_SIZE;
    }
    return root;
}

int setRootOffset(FILE *indexFile, long rootOffset){
    if(!indexFile){
        printf("setRootOffset: indexFile NULL\n");
        return 0;
    }
    if(fseek(indexFile, ROOTOFFSET_OFFSET, SEEK_SET) != 0){
        printf("setRootOffset: fseek failed\n");
        return 0;
    }
    if(fwrite(&rootOffset, sizeof(long), 1, indexFile) != 1){
        printf("setRootOffset: fwrite failed\n");
        return 0;
    }
    fflush(indexFile);
    return 1;
}

char *bPlusTreeInitializeIndexFile(int t){

  int fileCounter = 0;
  long root = HEADER_SIZE;
  char *indexFileName = (char*) malloc(sizeof(char)*10);
  strcpy(indexFileName, "Index.dat");

  FILE *indexFile = fopen(indexFileName, "wb");
  if(!indexFile){
    printf("indexFile == NULL in initialize\n");
  }
  if(fwrite(&fileCounter, sizeof(int), 1, indexFile) != 1){
    printf("error writing initial fileCounter\n");
    exit(1);
  }
  if(fwrite(&root, sizeof(long), 1, indexFile) != 1){
    printf("bPlusTreeInitializeIndexFile: error writing root");
    exit(1);
  }

  BPlusTree2M *firstNode = bPlusTreeCreate2M(t);  
  writeIndexNode(indexFile, firstNode, root);

  bPlusTreeFree2M(firstNode);
  fclose(indexFile);

  return indexFileName;
}

int writeIndexNode(FILE *indexFile, BPlusTree2M *node, long offset){
    int i;

    if(!indexFile || !node){
        printf("error calling writeIndexNode");
        exit(1);
    }
    fseek(indexFile, offset, SEEK_SET);

    if(fwrite(&(node->t), sizeof(int), 1, indexFile) != 1){
        printf("error writing node->t");
        exit(1);
    }
    if(fwrite(&(node->numKeys), sizeof(int), 1, indexFile) != 1){
        printf("error writing node->numKeys");
        exit(1);
    }
    if(fwrite(&(node->isLeafsParent), sizeof(int), 1, indexFile) != 1){
        printf("error writing isLeaf");
        exit(1);
    }
    if(fwrite(node->key, sizeof(int), ((node->t)*2) - 1, indexFile) != ((node->t)*2) - 1){
        printf("error writing keys");
        exit(1);
    }
    if(fwrite(node->childOffsets, sizeof(long), (node->t)*2, indexFile) != (node->t)*2){
        printf("error writing childOffsets");
        exit(1);
    }
    for(i=0; i < (node->t)*2; i++){
        if(fwrite((node->leafFiles)[i], sizeof(char), 5, indexFile) != 5){
        printf("error writing leafFiles");
        exit(1);
        }
    }
    fflush(indexFile);
    return 1;
}

BPlusTree2M* readIndexNode(FILE *indexFile, long offset){
    if(!indexFile || offset == LONG_MAX){
        printf("error reading index node");
        exit(1);
    }

    if(fseek(indexFile, offset, SEEK_SET) != 0){
        printf("error reading index node");
        exit(1);
    };

    int i, t;


    if(fread(&t, sizeof(int), 1, indexFile) != 1) exit(1);

    BPlusTree2M *node = bPlusTreeCreate2M(t);

    if(fread(&(node->numKeys), sizeof(int), 1, indexFile) != 1) exit(1);

    if(fread(&(node->isLeafsParent), sizeof(int), 1, indexFile) != 1) exit(1);

    if(fread(node->key, sizeof(int), ((node->t)*2)-1, indexFile) != ((node->t)*2)-1) exit(1);

    if(fread(node->childOffsets, sizeof(long), (node->t)*2, indexFile) != (node->t)*2) exit(1);

    for(i=0; i < (node->t)*2; i++){
        if(fread(node->leafFiles[i], sizeof(char), 5, indexFile) != 5){
            printf("error reading leaf file names");
            exit(1);
        }
        node->leafFiles[i][4] = '\0';
    }

 
    return node;
}

int writeLeafData(LeafNode *leafNode, FILE *leafFile){

    if(!leafNode || !leafFile){
        printf("error writing leaf data!");
        exit(1);
    }

    int i;
    fseek(leafFile, 0L, SEEK_SET);

    if(fwrite(&(leafNode->t), sizeof(int), 1, leafFile) != 1){
        printf("error writing leaf data: (t)");
        exit(1);
    }

    if(fwrite(&(leafNode->numKeys), sizeof(int), 1, leafFile) != 1){
        printf("error writing leaf data: (nkeys)");
        exit(1);
    }

    if(fwrite(leafNode->nextLeafFile, sizeof(char), 5, leafFile) != 5){
        printf("error writing leaf data: (nextleaf)");
        exit(1);
    }

    for(i=0; i < ((leafNode->t)*2)-1; i++){
        if(fwrite((leafNode->YearFileArray)[i], sizeof(char), 5, leafFile) != 5){
        printf("error writing leaf data: (year files)");
        exit(1);
        }
    }
    fflush(leafFile);

    return 1;
}

LeafNode *readLeafData(FILE *leafFile){

    if(!leafFile){
        printf("error reading leaf data");
        return NULL;
    }

    int i, t;
    fseek(leafFile, 0L, SEEK_SET);

    if(fread(&t, sizeof(int), 1, leafFile) != 1) exit(1);

    LeafNode *leaf = leafNodeCreate2M(t);

    if(fread(&(leaf->numKeys), sizeof(int), 1, leafFile) != 1) exit(1);

    if(fread(leaf->nextLeafFile, sizeof(char), 5, leafFile) != 5) exit(1);

    leaf->nextLeafFile[4] = '\0';

    for(i=0; i < (2*t)-1; i++){
        if(fread(((leaf->YearFileArray)[i]), sizeof(char), 5, leafFile) != 5) exit(1);
        ((leaf->YearFileArray)[i])[4] = '\0';
    }

    return leaf;
}





#endif