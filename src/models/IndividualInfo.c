#include "IndividualInfo.h"
#include "Participation.h"
#include "../../libs/GenericTypes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

IndividualInfo* individualInfoCreate(const char* personName) {
    IndividualInfo* info = (IndividualInfo*)malloc(sizeof(IndividualInfo));
    if (!info) return NULL;
    
    strncpy(info->personName, personName, 255);
    info->personName[255] = '\0';
    
    info->participationList = linkedListInitialize();
    
    return info;
}

void individualInfoFree(IndividualInfo* info) {
    if (info) {
        linkedListFree(info->participationList, (FreeFunc)participationFree);
        free(info);
    }
}

void individualInfoPrint(IndividualInfo* info) {
    if (!info) return;
    
    int count = linkedListSize(info->participationList);
    printf("\n=== %s ===\n", info->personName);
    printf("total de participações: %d\n", count);
    
    if (count > 0) {
        printf("\participações:\n");
        linkedListPrint(info->participationList, (PrintFunc)participationPrint);
    }
}

int individualInfoCompare(IndividualInfo* a, IndividualInfo* b) {
    if (!a || !b) return 0;
    return strcmp(a->personName, b->personName);
}

void individualInfoAddParticipation(IndividualInfo* info, void* participation) {
    if (info && participation) {
        info->participationList = linkedListInsert(info->participationList, participation);
    }
}

static void participationSaveToFile(Participation* participation, FILE* file) {
    if (!participation || !file) return;
    fprintf(file, "SCHOOL_NAME:%s\n", participation->schoolName);
    fprintf(file, "CATEGORY:%s\n", participation->category);
    fprintf(file, "YEAR:%u\n", participation->year);
}

static Participation* participationLoadFromFile(FILE* file) {
    if (!file) return NULL;
    
    char line[512];
    char schoolName[256] = "";
    char category[128] = "";
    uint16_t year = 0;
    int fieldsRead = 0;
    
    while (fieldsRead < 3 && fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* value = colon + 1;
            
            if (strcmp(line, "SCHOOL_NAME") == 0) {
                strncpy(schoolName, value, 255);
                schoolName[255] = '\0';
                fieldsRead++;
            } else if (strcmp(line, "CATEGORY") == 0) {
                strncpy(category, value, 127);
                category[127] = '\0';
                fieldsRead++;
            } else if (strcmp(line, "YEAR") == 0) {
                year = (uint16_t)atoi(value);
                fieldsRead++;
            }
        }
    }
    
    return participationCreate(schoolName, category, year);
}

int individualInfoSave(IndividualInfo* info, const char* filepath) {
    if (!info || !filepath) return -1;
    
    FILE* file = fopen(filepath, "w");
    if (!file) return -1;
    
    fprintf(file, "PERSON_NAME:%s\n", info->personName);
    
    int participationCount = linkedListSize(info->participationList);
    fprintf(file, "PARTICIPATIONS_COUNT:%d\n", participationCount);
    LinkedList* curr = info->participationList;
    while (curr) {
        participationSaveToFile((Participation*)curr->info, file);
        curr = curr->next;
    }
    
    fclose(file);
    return 0;
}

IndividualInfo* individualInfoLoad(const char* filepath) {
    if (!filepath) return NULL;
    
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;
    
    char line[512];
    IndividualInfo* info = NULL;
    char personName[256] = "";
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* value = colon + 1;
            
            if (strcmp(line, "PERSON_NAME") == 0) {
                strncpy(personName, value, 255);
                personName[255] = '\0';
                info = individualInfoCreate(personName);
                if (!info) {
                    fclose(file);
                    return NULL;
                }
            } else if (strcmp(line, "PARTICIPATIONS_COUNT") == 0) {
                int count = atoi(value);
                for (int i = 0; i < count; i++) {
                    Participation* participation = participationLoadFromFile(file);
                    if (participation) {
                        individualInfoAddParticipation(info, participation);
                    }
                }
            }
        }
    }
    
    fclose(file);
    return info;
}
