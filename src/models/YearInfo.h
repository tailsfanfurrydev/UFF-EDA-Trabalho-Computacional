#ifndef YEAR_INFO_H
#define YEAR_INFO_H

#include <stdint.h>
#include "../../libs/LinkedList.h"
#include "ChampionshipInfo.h"

typedef struct {
    uint16_t year;
    LinkedList* champions;
    LinkedList* runnersUp;
} YearInfo;

YearInfo* yearInfoCreate(uint16_t year);
void yearInfoFree(YearInfo* info);
void yearInfoPrint(YearInfo* info);
int yearInfoCompare(YearInfo* a, YearInfo* b);
void yearInfoAddChampion(YearInfo* info, void* championshipInfo);
void yearInfoAddRunnerUp(YearInfo* info, void* championshipInfo);

#endif
