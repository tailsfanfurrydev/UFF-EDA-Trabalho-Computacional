#include "YearInfo.h"
#include "ChampionshipInfo.h"
#include "../../libs/GenericTypes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void championshipInfoSaveToFile(ChampionshipInfo* info, FILE* file) {
    if (!info || !file) return;
    fprintf(file, "YEAR:%u\n", info->year);
    fprintf(file, "TITLE_NUMBER:%u\n", info->titleNumber);
    fprintf(file, "SCHOOL_NAME:%s\n", info->schoolName);
    fprintf(file, "THEME:%s\n", info->theme);
    fprintf(file, "DESIGNER:%s\n", info->carnivalDesigner);
}

static ChampionshipInfo* championshipInfoLoadFromFile(FILE* file) {
    if (!file) return NULL;
    
    char line[512];
    uint16_t year = 0;
    uint8_t titleNumber = 0;
    char schoolName[256] = "";
    char theme[256] = "";
    char designer[256] = "";
    int fieldsRead = 0;
    
    while (fieldsRead < 5 && fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* value = colon + 1;
            
            if (strcmp(line, "YEAR") == 0) {
                year = (uint16_t)atoi(value);
                fieldsRead++;
            } else if (strcmp(line, "TITLE_NUMBER") == 0) {
                titleNumber = (uint8_t)atoi(value);
                fieldsRead++;
            } else if (strcmp(line, "SCHOOL_NAME") == 0) {
                strncpy(schoolName, value, 255);
                schoolName[255] = '\0';
                fieldsRead++;
            } else if (strcmp(line, "THEME") == 0) {
                strncpy(theme, value, 255);
                theme[255] = '\0';
                fieldsRead++;
            } else if (strcmp(line, "DESIGNER") == 0) {
                strncpy(designer, value, 255);
                designer[255] = '\0';
                fieldsRead++;
            }
        }
    }
    
    return championshipInfoCreate(year, titleNumber, schoolName, theme, designer);
}

YearInfo* yearInfoCreate(uint16_t year) {
    YearInfo* info = (YearInfo*)malloc(sizeof(YearInfo));
    if (!info) return NULL;
    
    info->year = year;
    info->champions = linkedListInitialize();
    info->runnersUp = linkedListInitialize();
    
    return info;
}

void yearInfoFree(YearInfo* info) {
    if (info) {
        linkedListFree(info->champions, (FreeFunc)championshipInfoFree);
        linkedListFree(info->runnersUp, (FreeFunc)championshipInfoFree);
        free(info);
    }
}

void yearInfoPrint(YearInfo* info) {
    if (!info) return;
    
    printf("\n=== carnaval %d ===\n", info->year);
    
    int championCount = linkedListSize(info->champions);
    int runnerUpCount = linkedListSize(info->runnersUp);
    
    printf("\nCampeoes: %d\n", championCount);
    if (championCount > 0) {
        LinkedList* current = info->champions;
        int i = 1;
        while(current) {
            ChampionshipInfo* champ = (ChampionshipInfo*)current->info;
            printf("  %d. Escola: %s\n", i, champ->schoolName);
            if(champ->titleNumber > 0) {
                printf("     Titulo n.%d\n", champ->titleNumber);
            }
            printf("     Enredo: %s\n", champ->theme);
            printf("     Carnavalesco: %s\n", champ->carnivalDesigner);
            current = current->next;
            i++;
        }
    }
    
    printf("\nVice-campeoes: %d\n", runnerUpCount);
    if (runnerUpCount > 0) {
        LinkedList* current = info->runnersUp;
        int i = 1;
        while(current) {
            ChampionshipInfo* runner = (ChampionshipInfo*)current->info;
            printf("  %d. Escola: %s\n", i, runner->schoolName);
            printf("     Enredo: %s\n", runner->theme);
            printf("     Carnavalesco: %s\n", runner->carnivalDesigner);
            current = current->next;
            i++;
        }
    }
    
    printf("\n");
}

int yearInfoCompare(YearInfo* a, YearInfo* b) {
    if (!a || !b) return 0;
    return (int)a->year - (int)b->year;
}

void yearInfoAddChampion(YearInfo* info, void* championshipInfo) {
    if (info && championshipInfo) {
        info->champions = linkedListInsert(info->champions, championshipInfo);
    }
}

void yearInfoAddRunnerUp(YearInfo* info, void* championshipInfo) {
    if (info && championshipInfo) {
        info->runnersUp = linkedListInsert(info->runnersUp, championshipInfo);
    }
}

int yearInfoSave(YearInfo* info, const char* filepath) {
    if (!info || !filepath) return -1;
    
    FILE* file = fopen(filepath, "w");
    if (!file) return -1;
    
    fprintf(file, "YEAR:%u\n", info->year);
    
    int champCount = linkedListSize(info->champions);
    fprintf(file, "CHAMPIONS_COUNT:%d\n", champCount);
    LinkedList* curr = info->champions;
    while (curr) {
        championshipInfoSaveToFile((ChampionshipInfo*)curr->info, file);
        curr = curr->next;
    }
    
    int runnerUpCount = linkedListSize(info->runnersUp);
    fprintf(file, "RUNNERSUP_COUNT:%d\n", runnerUpCount);
    curr = info->runnersUp;
    while (curr) {
        championshipInfoSaveToFile((ChampionshipInfo*)curr->info, file);
        curr = curr->next;
    }
    
    fclose(file);
    return 0;
}

YearInfo* yearInfoLoad(const char* filepath) {
    if (!filepath) return NULL;
    
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;
    
    char line[512];
    YearInfo* info = NULL;
    uint16_t year = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* value = colon + 1;
            
            if (strcmp(line, "YEAR") == 0) {
                year = (uint16_t)atoi(value);
                info = yearInfoCreate(year);
                if (!info) {
                    fclose(file);
                    return NULL;
                }
            } else if (strcmp(line, "CHAMPIONS_COUNT") == 0) {
                int count = atoi(value);
                for (int i = 0; i < count; i++) {
                    ChampionshipInfo* champ = championshipInfoLoadFromFile(file);
                    if (champ) {
                        yearInfoAddChampion(info, champ);
                    }
                }
            } else if (strcmp(line, "RUNNERSUP_COUNT") == 0) {
                int count = atoi(value);
                for (int i = 0; i < count; i++) {
                    ChampionshipInfo* runner = championshipInfoLoadFromFile(file);
                    if (runner) {
                        yearInfoAddRunnerUp(info, runner);
                    }
                }
            }
        }
    }
    
    fclose(file);
    return info;
}
