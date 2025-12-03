#include "ChampionshipInfo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ChampionshipInfo* championshipInfoCreate(uint16_t year, uint8_t titleNumber, const char* schoolName, const char* theme, const char* carnivalDesigner){
    ChampionshipInfo* info = (ChampionshipInfo*)malloc(sizeof(ChampionshipInfo));
    if (!info) return NULL;
    
    info->year = year;
    info->titleNumber = titleNumber;
    strncpy(info->schoolName, schoolName, 255);
    info->schoolName[255] = '\0';
    strncpy(info->theme, theme, 255);
    info->theme[255] = '\0';
    strncpy(info->carnivalDesigner, carnivalDesigner, 255);
    info->carnivalDesigner[255] = '\0';
    
    return info;
}

void championshipInfoFree(ChampionshipInfo* info)  {
    if (info) free(info);
    
}

void championshipInfoPrint(ChampionshipInfo* info) {
    if (info)
        printf("%d - %s (carnavalesco: %s)\n", info->year, info->theme, info->carnivalDesigner);
}

int championshipInfoCompare(ChampionshipInfo* a, ChampionshipInfo* b){
    if (!a || !b) return 0;
    return (int)a->year - (int)b->year;
}
