#include "YearInfo.h"
#include "ChampionshipInfo.h"
#include "../../libs/GenericTypes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    
    printf("campeoes: %d\n", championCount);
    if (championCount > 0) {
        linkedListPrint(info->champions, (PrintFunc)championshipInfoPrint);
    }
    printf("\nvices: %d\n", runnerUpCount);
    if (runnerUpCount > 0) {
        linkedListPrint(info->runnersUp, (PrintFunc)championshipInfoPrint);
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
