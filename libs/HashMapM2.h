#ifndef HASH_MAP_M2_H
#define HASH_MAP_M2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "HashUtils.h"

#define MAX_KEY_LENGTH 256
#define MAX_PATH_LENGTH 512

typedef struct {
    char *directory;
    FILE *indexFile;
    FILE *dataFile;
} HashMapM2Context;

typedef struct {
    char key[MAX_KEY_LENGTH];
    char filePath[MAX_PATH_LENGTH];
    long nextLine;
    int isValid;
} HashMapM2Entry;

HashMapM2Context* hashMapM2Load(const char *directory);
void hashMapM2ContextFree(HashMapM2Context *context);

void hashMapM2Insert(HashMapM2Context *context, const char *key, const char *filePath);
char* hashMapM2Get(HashMapM2Context *context, const char *key);
bool hashMapM2Contains(HashMapM2Context *context, const char *key);
int hashMapM2Remove(HashMapM2Context *context, const char *key);
void hashMapM2PrintAll(HashMapM2Context *context);

typedef void (*HashMapM2IteratorCallback)(const char *key, const char *filePath, void *userData);
void hashMapM2ForEach(HashMapM2Context *context, HashMapM2IteratorCallback callback, void *userData);

#endif
