#ifndef CHAMPIONSHIP_INFO_H
#define CHAMPIONSHIP_INFO_H
#include <stdint.h>

typedef struct {
    uint16_t year;
    uint8_t titleNumber;
    char schoolName[256];
    char theme[256];
    char carnivalDesigner[256];
} ChampionshipInfo;

ChampionshipInfo* championshipInfoCreate(uint16_t year, uint8_t titleNumber, const char* schoolName, const char* theme, const char* carnivalDesigner);
void championshipInfoFree(ChampionshipInfo* info);
void championshipInfoPrint(ChampionshipInfo* info);
int championshipInfoCompare(ChampionshipInfo* a, ChampionshipInfo* b);


#endif
