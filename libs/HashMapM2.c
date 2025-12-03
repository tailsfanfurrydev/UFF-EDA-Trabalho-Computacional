#include "LinkedList.h"
#include "HashMapM2.h"
#include <sys/stat.h>
#include <unistd.h>

static int directoryExists(const char *path){
    struct stat stats;
    return stat(path, &stats) == 0 && S_ISDIR(stats.st_mode);
}

static int createDirectory(const char *path){
    if (directoryExists(path)) return 1;
    return mkdir(path, 0755) == 0;
}

static char* buildFilePath(const char *directory, const char *filename){
    size_t len = strlen(directory) + strlen(filename) + 2;
    char *path = (char*)malloc(len);
    snprintf(path, len, "%s/%s", directory, filename);
    return path;
}

static void initializeIndexFile(FILE *indexFile){
    for (int i = 0; i < DIVIDER; i++){
        long emptyPointer = -1;
        fwrite(&emptyPointer, sizeof(long), 1, indexFile);
    }
    fflush(indexFile);
}

HashMapM2Context* hashMapM2Load(const char *directory){
    if (!directory) return NULL;
    
    if (!createDirectory(directory)){
        return NULL;
    }
    
    HashMapM2Context *context = (HashMapM2Context*)malloc(sizeof(HashMapM2Context));
    context->directory = strdup(directory);
    
    char *indexPath = buildFilePath(directory, "Index.dat");
    char *dataPath = buildFilePath(directory, "Data.dat");
    
    int indexExists = access(indexPath, F_OK) == 0;
    
    context->indexFile = fopen(indexPath, indexExists ? "rb+" : "wb+");
    context->dataFile = fopen(dataPath, indexExists ? "rb+" : "wb+");
    
    if (!context->indexFile || !context->dataFile){
        if (context->indexFile) fclose(context->indexFile);
        if (context->dataFile) fclose(context->dataFile);
        free(indexPath);
        free(dataPath);
        free(context->directory);
        free(context);
        return NULL;
    }
    
    if (!indexExists){
        initializeIndexFile(context->indexFile);
    }
    
    free(indexPath);
    free(dataPath);
    
    return context;
}

void hashMapM2ContextFree(HashMapM2Context *context){
    if (!context) return;
    
    if (context->indexFile){
        fflush(context->indexFile);
        fclose(context->indexFile);
    }
    if (context->dataFile){
        fflush(context->dataFile);
        fclose(context->dataFile);
    }
    
    free(context->directory);
    free(context);
}

static long getIndexPointer(FILE *indexFile, int hash){
    fseek(indexFile, hash * sizeof(long), SEEK_SET);
    long pointer;
    fread(&pointer, sizeof(long), 1, indexFile);
    return pointer;
}

static void setIndexPointer(FILE *indexFile, int hash, long pointer){
    fseek(indexFile, hash * sizeof(long), SEEK_SET);
    fwrite(&pointer, sizeof(long), 1, indexFile);
    fflush(indexFile);
}
static long writeEntry(FILE *dataFile, const HashMapM2Entry *entry){
    fseek(dataFile, 0, SEEK_END);
    long position = ftell(dataFile);
    
    fwrite(entry->key, sizeof(char), MAX_KEY_LENGTH, dataFile);
    fwrite(entry->filePath, sizeof(char), MAX_PATH_LENGTH, dataFile);
    fwrite(&entry->nextLine, sizeof(long), 1, dataFile);
    fwrite(&entry->isValid, sizeof(int), 1, dataFile);
    
    fflush(dataFile);
    return position;
}

static int readEntry(FILE *dataFile, long position, HashMapM2Entry *entry){
    if (position < 0) return 0;
    
    fseek(dataFile, position, SEEK_SET);
    
    if (fread(entry->key, sizeof(char), MAX_KEY_LENGTH, dataFile) != MAX_KEY_LENGTH) return 0;
    if (fread(entry->filePath, sizeof(char), MAX_PATH_LENGTH, dataFile) != MAX_PATH_LENGTH) return 0;
    if (fread(&entry->nextLine, sizeof(long), 1, dataFile) != 1) return 0;
    if (fread(&entry->isValid, sizeof(int), 1, dataFile) != 1) return 0;
    
    return 1;
}

static void updateEntry(FILE *dataFile, long position, const HashMapM2Entry *entry){
    fseek(dataFile, position, SEEK_SET);
    
    fwrite(entry->key, sizeof(char), MAX_KEY_LENGTH, dataFile);
    fwrite(entry->filePath, sizeof(char), MAX_PATH_LENGTH, dataFile);
    fwrite(&entry->nextLine, sizeof(long), 1, dataFile);
    fwrite(&entry->isValid, sizeof(int), 1, dataFile);
    
    fflush(dataFile);
}

void hashMapM2Insert(HashMapM2Context *context, const char *key, const char *filePath){
    if (!context || !key || !filePath) return;
    
    int hash = hashFromString((char*)key);
    long headPointer = getIndexPointer(context->indexFile, hash);
    
    HashMapM2Entry newEntry;
    memset(&newEntry, 0, sizeof(HashMapM2Entry));
    strncpy(newEntry.key, key, MAX_KEY_LENGTH - 1);
    strncpy(newEntry.filePath, filePath, MAX_PATH_LENGTH - 1);
    newEntry.nextLine = -1;
    newEntry.isValid = 1;
    
    if (headPointer == -1){
        long position = writeEntry(context->dataFile, &newEntry);
        setIndexPointer(context->indexFile, hash, position);
        return;
    }
    
    long currentPos = headPointer;
    long prevPos = -1;
    HashMapM2Entry currentEntry;
    
    while (currentPos != -1){
        readEntry(context->dataFile, currentPos, &currentEntry);
        
        if (currentEntry.isValid && strcmp(currentEntry.key, key) == 0){
            strncpy(currentEntry.filePath, filePath, MAX_PATH_LENGTH - 1);
            updateEntry(context->dataFile, currentPos, &currentEntry);
            return;
        }
        
        prevPos = currentPos;
        currentPos = currentEntry.nextLine;
    }
    
    long newPosition = writeEntry(context->dataFile, &newEntry);
    
    readEntry(context->dataFile, prevPos, &currentEntry);
    currentEntry.nextLine = newPosition;
    updateEntry(context->dataFile, prevPos, &currentEntry);
}

char* hashMapM2Get(HashMapM2Context *context, const char *key){
    if (!context || !key) return NULL;
    
    int hash = hashFromString((char*)key);
    long currentPos = getIndexPointer(context->indexFile, hash);
    
    HashMapM2Entry entry;
    
    while (currentPos != -1){
        if (!readEntry(context->dataFile, currentPos, &entry)) break;
        
        if (entry.isValid && strcmp(entry.key, key) == 0){
            return strdup(entry.filePath);
        }
        
        currentPos = entry.nextLine;
    }
    
    return NULL;
}

bool hashMapM2Contains(HashMapM2Context *context, const char *key){
    char *result = hashMapM2Get(context, key);
    if (result){
        free(result);
        return true;
    }
    return false;
}

int hashMapM2Remove(HashMapM2Context *context, const char *key){
    if (!context || !key) return 0;
    
    int hash = hashFromString((char*)key);
    long currentPos = getIndexPointer(context->indexFile, hash);
    
    if (currentPos == -1) return 0;
    
    long prevPos = -1;
    HashMapM2Entry currentEntry;
    
    while (currentPos != -1){
        if (!readEntry(context->dataFile, currentPos, &currentEntry)) return 0;
        
        if (currentEntry.isValid && strcmp(currentEntry.key, key) == 0){
            currentEntry.isValid = 0;
            updateEntry(context->dataFile, currentPos, &currentEntry);
            
            if (prevPos == -1){
                setIndexPointer(context->indexFile, hash, currentEntry.nextLine);
            } else{
                HashMapM2Entry prevEntry;
                readEntry(context->dataFile, prevPos, &prevEntry);
                prevEntry.nextLine = currentEntry.nextLine;
                updateEntry(context->dataFile, prevPos, &prevEntry);
            }
            
            return 1;
        }
        
        prevPos = currentPos;
        currentPos = currentEntry.nextLine;
    }
    
    return 0;
}

void hashMapM2PrintAll(HashMapM2Context *context){
    if (!context || !context->indexFile || !context->dataFile) return;
    
    printf("\n=== conteudo da hashmap ===\n");
    int totalEntries = 0;
    
    for (int hash = 0; hash < DIVIDER; hash++){
        long currentPos = getIndexPointer(context->indexFile, hash);
        
        if (currentPos == -1) continue;
        
        HashMapM2Entry entry;
        while (currentPos != -1){
            if (!readEntry(context->dataFile, currentPos, &entry)) break;
            
            if (entry.isValid){
                printf("  [%s] -> %s\n", entry.key, entry.filePath);
                totalEntries++;
            }
            
            currentPos = entry.nextLine;
        }
    }
    
    printf("\mTotal: %d items\n\n", totalEntries);
   // printf("========================\n\n");
}
void hashMapM2ForEach(HashMapM2Context *context, HashMapM2IteratorCallback callback, void *userData){
    if (!context || !context->indexFile || !context->dataFile || !callback) return;
    
    for (int hash = 0; hash < DIVIDER; hash++){
        long currentPos = getIndexPointer(context->indexFile, hash);
        
        if (currentPos == -1) continue;
        
        HashMapM2Entry entry;
        while (currentPos != -1){
            if (!readEntry(context->dataFile, currentPos, &entry)) break;
            
            if (entry.isValid){
                callback(entry.key, entry.filePath, userData);
            }
            
            currentPos = entry.nextLine;
        }
    }
}
