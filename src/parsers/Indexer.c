#include "Indexer.h"
#include "../models/YearInfo.h"
#include "../models/SchoolInfo.h"
#include "../models/IndividualInfo.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static void trim(char* str){
    if(!str) return;
    //espaços estupidos atrapalhando meu parser
    char* start = str;
    while(isspace((unsigned char)*start)) start++;

    char* end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    if(start != str){
        memmove(str, start, strlen(start) + 1);
    }
}

static char* buildYearFilePath(const char* baseDataPath, int year){
    char filename[32];
    snprintf(filename, sizeof(filename), "year-%d.dat", year);
    
    size_t len = strlen(baseDataPath) + strlen("/YearInfo/") + strlen(filename) + 1;
    char* path = (char*)malloc(len);
    snprintf(path, len, "%s/YearInfo/%s", baseDataPath, filename);
    return path;
}

static char* buildSchoolFilePath(const char* baseDataPath, const char* schoolName){
    char cleanName[256];
    strncpy(cleanName, schoolName, 255);
    cleanName[255] = '\0';
    trim(cleanName);
    for(int i = 0; cleanName[i]; i++){
        if(cleanName[i] == ' ') cleanName[i] = '_';
    }
    
    char filename[300];
    snprintf(filename, sizeof(filename), "school-%s.dat", cleanName);
    
    size_t len = strlen(baseDataPath) + strlen("/SchoolInfo/") + strlen(filename) + 1;
    char* path = (char*)malloc(len);
    snprintf(path, len, "%s/SchoolInfo/%s", baseDataPath, filename);
    return path;
}
static char* buildIndividualFilePath(const char* baseDataPath, const char* personName){
    char cleanName[256];
    strncpy(cleanName, personName, 255);
    cleanName[255] = '\0';
    trim(cleanName);


    for(int i = 0; cleanName[i]; i++){
        if(cleanName[i] == ' ') cleanName[i] = '_';
    }
    
    char filename[300];
    snprintf(filename, sizeof(filename), "individual-%s.dat", cleanName);
    
    size_t len = strlen(baseDataPath) + strlen("/IndividualInfo/") + strlen(filename) + 1;
    char* path = (char*)malloc(len);
    snprintf(path, len, "%s/IndividualInfo/%s", baseDataPath, filename);
    return path;
}

IndexerContext* indexerCreate(const char* baseDataPath){
    IndexerContext* context = (IndexerContext*)malloc(sizeof(IndexerContext));
    if(!context) return NULL;
    char yearIndexPath[512];
    snprintf(yearIndexPath, sizeof(yearIndexPath), "%s/YearInfo", baseDataPath);
    char schoolIndexPath[512];
    snprintf(schoolIndexPath, sizeof(schoolIndexPath), "%s/SchoolInfo", baseDataPath);
    char individualIndexPath[512];
    snprintf(individualIndexPath, sizeof(individualIndexPath), "%s/IndividualInfo", baseDataPath);


    context->yearIndex = bPlusTreeLoad(yearIndexPath, 3);
    context->schoolIndex = hashMapM2Load(schoolIndexPath);
    context->individualIndex = hashMapM2Load(individualIndexPath);
    
    if(!context->yearIndex || !context->schoolIndex || !context->individualIndex){
        indexerFree(context);
        return NULL;
    }
    
    return context;
}

void indexerFree(IndexerContext* context){
    if(!context) return;
    
    if(context->yearIndex) bPlusTreeContextFree(context->yearIndex);
    if(context->schoolIndex) hashMapM2ContextFree(context->schoolIndex);
    if(context->individualIndex) hashMapM2ContextFree(context->individualIndex);
    
    free(context);
}

int indexerBuildFromParser(IndexerContext* context, ParserContext* parserContext, const char* baseDataPath){
    if(!context || !parserContext || !baseDataPath) return 0;
    
    LinkedList* yearNode = parserContext->yearInfoList;
    while(yearNode){
        YearInfo* yearInfo = (YearInfo*)yearNode->info;
        
        char* yearPath = buildYearFilePath(baseDataPath, yearInfo->year);
        
        bPlusTreeInsert2M(context->yearIndex->indexFile, yearInfo->year, context->yearIndex->t);
        
        free(yearPath);
        yearNode = yearNode->next;
    }
    
    LinkedList* schoolNode = parserContext->schoolInfoList;
    while(schoolNode){
        SchoolInfo* schoolInfo = (SchoolInfo*)schoolNode->info;
        
        char cleanSchoolName[256];
        strncpy(cleanSchoolName, schoolInfo->schoolName, 255);
        cleanSchoolName[255] = '\0';
        trim(cleanSchoolName);
        
        char* schoolPath = buildSchoolFilePath(baseDataPath, cleanSchoolName);
        
        hashMapM2Insert(context->schoolIndex, cleanSchoolName, schoolPath);
        
        free(schoolPath);
        schoolNode = schoolNode->next;
    }
    
    LinkedList* individualNode = parserContext->individualInfoList;
    while(individualNode){
        IndividualInfo* individualInfo = (IndividualInfo*)individualNode->info;
    
        char cleanPersonName[256];
        strncpy(cleanPersonName, individualInfo->personName, 255);
        cleanPersonName[255] = '\0';
        trim(cleanPersonName);
        
        char* individualPath = buildIndividualFilePath(baseDataPath, cleanPersonName);
        
        hashMapM2Insert(context->individualIndex, cleanPersonName, individualPath);
        
        free(individualPath);
        individualNode = individualNode->next;
    }
    
    return 1;
}

char* indexerSearchYear(IndexerContext* context, int year){
    if(!context || !context->yearIndex) return NULL;
    
    char* leafFile = bPlusTreeSearch2M(context->yearIndex->indexFile, year, getRootOffset(context->yearIndex->indexFile));
    
    if(!leafFile) return NULL;
    
    free(leafFile);
    
    return buildYearFilePath("data", year);
}

char* indexerSearchSchool(IndexerContext* context, const char* schoolName){
    if(!context || !context->schoolIndex || !schoolName) return NULL;
    
    return hashMapM2Get(context->schoolIndex, schoolName);
}

char* indexerSearchIndividual(IndexerContext* context, const char* personName){
    if(!context || !context->individualIndex || !personName) return NULL;
    
    return hashMapM2Get(context->individualIndex, personName);
}
