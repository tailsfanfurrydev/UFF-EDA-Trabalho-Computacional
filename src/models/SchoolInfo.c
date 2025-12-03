#include "SchoolInfo.h"
#include "ChampionshipInfo.h"
#include "EstandarteAward.h"
#include "../../libs/GenericTypes.h"
#include "../../libs/GenericImplementations.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

SchoolInfo* schoolInfoCreate(const char* schoolName, uint8_t isActive) {
    SchoolInfo* info = (SchoolInfo*)malloc(sizeof(SchoolInfo));
    if (!info) return NULL;
    
    strncpy(info->schoolName, schoolName, 255);
    info->schoolName[255] = '\0';
    info->isActive = isActive;
    
    info->titleList = linkedListInitialize();
    info->runnerUpList = linkedListInitialize();
    info->awardList = linkedListInitialize();
    
    return info;
}

void schoolInfoFree(SchoolInfo* info) {
    if (info) {
        linkedListFree(info->titleList, (FreeFunc)championshipInfoFree);
        linkedListFree(info->runnerUpList, (FreeFunc)championshipInfoFree);
        linkedListFree(info->awardList, (FreeFunc)estandarteAwardFree);
        free(info);
    }
}

void schoolInfoPrint(SchoolInfo* info) {
    if (!info) return;
    
    printf("\n=== %s %s ===\n", info->schoolName, info->isActive ? "" : "[EXTINTA]"); //amo ternario
    
    int titleCount = linkedListSize(info->titleList);
    int runnerUpCount = linkedListSize(info->runnerUpList);
    int awardCount = linkedListSize(info->awardList);
    
    printf("titulos: %d\n", titleCount);
    if (titleCount > 0) {
        linkedListPrint(info->titleList, (PrintFunc)championshipInfoPrint);
    }
    
    printf("\nvices: %d\n", runnerUpCount);
    if (runnerUpCount > 0) {
        linkedListPrint(info->runnerUpList, (PrintFunc)championshipInfoPrint);
    }
    
    printf("\mestandarte de ouro: %d\n", awardCount);
    if (awardCount > 0) {
        linkedListPrint(info->awardList, (PrintFunc)estandarteAwardPrint);
    }
    
    printf("\n");
}

int schoolInfoCompare(SchoolInfo* a, SchoolInfo* b) {
    if (!a || !b) return 0;
    return strcmp(a->schoolName, b->schoolName);
}

void schoolInfoAddTitle(SchoolInfo* info, void* championshipInfo) {
    if (!info || !championshipInfo) return;
    
    ChampionshipInfo* newChamp = (ChampionshipInfo*)championshipInfo;
    
    LinkedList* current = info->titleList;
    while(current && current->info) {
        ChampionshipInfo* existing = (ChampionshipInfo*)current->info;
        if(existing->year == newChamp->year) {
            championshipInfoFree(newChamp);
            return;
        }
        current = current->next;
    }
    
    info->titleList = linkedListInsert(info->titleList, championshipInfo);
}

void schoolInfoAddRunnerUp(SchoolInfo* info, void* championshipInfo) {
    if (!info || !championshipInfo) return;
    
    ChampionshipInfo* newChamp = (ChampionshipInfo*)championshipInfo;
    
    LinkedList* current = info->runnerUpList;
    while(current && current->info) {
        ChampionshipInfo* existing = (ChampionshipInfo*)current->info;
        if(existing->year == newChamp->year) {
            championshipInfoFree(newChamp); //o arquivo tem duplicatas, ignorar elas e liberar pra tirar o leak
            return;
        }
        current = current->next;
    }
    
    info->runnerUpList = linkedListInsert(info->runnerUpList, championshipInfo);
}
void schoolInfoAddAward(SchoolInfo* info, void* estandarteAward) {
   
    if (info && estandarteAward)
        info->awardList = linkedListInsert(info->awardList, estandarteAward);
}

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

static void estandarteAwardSaveToFile(EstandarteAward* award, FILE* file) {
    if (!award || !file) return;
    fprintf(file, "YEAR:%u\n", award->year);
    fprintf(file, "CATEGORY:%s\n", award->category);
    fprintf(file, "WINNER:%s\n", award->winner);
}

static EstandarteAward* estandarteAwardLoadFromFile(FILE* file) {
    if (!file) return NULL;
    
    char line[512];
    uint16_t year = 0;
    char category[128] = "";
    char winner[256] = "";
    int fieldsRead = 0;
    
    while (fieldsRead < 3 && fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* value = colon + 1;
            
            if (strcmp(line, "YEAR") == 0) {
                year = (uint16_t)atoi(value);
                fieldsRead++;
            } else if (strcmp(line, "CATEGORY") == 0) {
                strncpy(category, value, 127);
                category[127] = '\0';
                fieldsRead++;
            } else if (strcmp(line, "WINNER") == 0) {
                strncpy(winner, value, 255);
                winner[255] = '\0';
                fieldsRead++;
            }
        }
    }
    
    return estandarteAwardCreate(year, category, winner);
}

int schoolInfoSave(SchoolInfo* info, const char* filepath) {
    if (!info || !filepath) return -1;
    
    FILE* file = fopen(filepath, "w");
    if (!file) return -1;
    
    fprintf(file, "SCHOOL_NAME:%s\n", info->schoolName);
    fprintf(file, "IS_ACTIVE:%u\n", info->isActive);
    
    int titleCount = linkedListSize(info->titleList);
    fprintf(file, "TITLES_COUNT:%d\n", titleCount);
    LinkedList* curr = info->titleList;
    while (curr) {
        championshipInfoSaveToFile((ChampionshipInfo*)curr->info, file);
        curr = curr->next;
    }
    
    int runnerUpCount = linkedListSize(info->runnerUpList);
    fprintf(file, "RUNNERSUP_COUNT:%d\n", runnerUpCount);
    curr = info->runnerUpList;
    while (curr) {
        championshipInfoSaveToFile((ChampionshipInfo*)curr->info, file);
        curr = curr->next;
    }
    
    int awardCount = linkedListSize(info->awardList);
    fprintf(file, "AWARDS_COUNT:%d\n", awardCount);
    curr = info->awardList;
    while (curr) {
        estandarteAwardSaveToFile((EstandarteAward*)curr->info, file);
        curr = curr->next;
    }
    
    fclose(file);
    return 0;
}

SchoolInfo* schoolInfoLoad(const char* filepath) {
    if (!filepath) return NULL;
    
    FILE* file = fopen(filepath, "r");
    if (!file) return NULL;
    
    char line[512];
    SchoolInfo* info = NULL;
    char schoolName[256] = "";
    uint8_t isActive = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char* value = colon + 1;
            
            if (strcmp(line, "SCHOOL_NAME") == 0) {
                strncpy(schoolName, value, 255);
                schoolName[255] = '\0';
            } else if (strcmp(line, "IS_ACTIVE") == 0) {
                isActive = (uint8_t)atoi(value);
                info = schoolInfoCreate(schoolName, isActive);
                if (!info) {
                    fclose(file);
                    return NULL;
                }
            } else if (strcmp(line, "TITLES_COUNT") == 0) {
                int count = atoi(value);
                for (int i = 0; i < count; i++) {
                    ChampionshipInfo* title = championshipInfoLoadFromFile(file);
                    if (title) {
                        schoolInfoAddTitle(info, title);
                    }
                }
            } else if (strcmp(line, "RUNNERSUP_COUNT") == 0) {
                int count = atoi(value);
                for (int i = 0; i < count; i++) {
                    ChampionshipInfo* runner = championshipInfoLoadFromFile(file);
                    if (runner) {
                        schoolInfoAddRunnerUp(info, runner);
                    }
                }
            } else if (strcmp(line, "AWARDS_COUNT") == 0) {
                int count = atoi(value);
                for (int i = 0; i < count; i++) {
                    EstandarteAward* award = estandarteAwardLoadFromFile(file);
                    if (award) {
                        schoolInfoAddAward(info, award);
                    }
                }
            }
        }
    }
    
    fclose(file);
    return info;
}
