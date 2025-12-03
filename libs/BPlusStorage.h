#ifndef B_PLUS_STORAGE_H
#define B_PLUS_STORAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BPlusTree2M.h"

#define FILECOUNTER_OFFSET 0
#define ROOTOFFSET_OFFSET  (FILECOUNTER_OFFSET + sizeof(int))
#define HEADER_SIZE        (sizeof(int) + sizeof(long))

typedef struct {
    char *directoryPath;
    char *indexFilePath;
    FILE *indexFile;
    int t;
} BPlusTreeContext;

BPlusTreeContext* bPlusTreeLoad(const char *directoryPath, int t);
void bPlusTreeContextFree(BPlusTreeContext *context);

void bPlusTreeSetCurrentDirectory(const char *directory);
const char* bPlusTreeGetCurrentDirectory();
void bPlusTreeCleanupCurrentDirectory();

long getRootOffset(FILE *indexFile);
int setRootOffset(FILE *indexFile, long rootOffset);

char *bPlusTreeInitializeIndexFile(int t);

char *nameToFile(char *fileName);
char *nameToFileInDirectory(char *fileName, const char *directory);

char *newFileName(FILE *indexFile);

long bPlusTree2MSizeInDisk(int t);

int writeIndexNode(FILE *indexFile, BPlusTree2M *node, long offset);

BPlusTree2M* readIndexNode(FILE *indexFile, long offset);

int writeLeafData(LeafNode *leafNode, FILE *leafFile);

LeafNode *readLeafData(FILE *leafFile);

char* buildFilePath(const char *directory, const char *filename);

#endif