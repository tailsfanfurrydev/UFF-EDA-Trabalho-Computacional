#ifndef SCHOOL_INFO_H //include guarde tem que ser adicionada em todos os headers pra evitar conflito :3
#define SCHOOL_INFO_H
#include "../../libs/LinkedList.h"
#include <stdint.h>

typedef struct {
    char schoolName[256];
    uint8_t isActive;
    LinkedList* titleList;
    LinkedList* runnerUpList;
    LinkedList* awardList;
} SchoolInfo;

SchoolInfo* schoolInfoCreate(const char* schoolName, uint8_t isActive);
void schoolInfoFree(SchoolInfo* info);
void schoolInfoPrint(SchoolInfo* info);
int schoolInfoCompare(SchoolInfo* a, SchoolInfo* b);
void schoolInfoAddTitle(SchoolInfo* info, void* championshipInfo);
void schoolInfoAddRunnerUp(SchoolInfo* info, void* championshipInfo);
void schoolInfoAddAward(SchoolInfo* info, void* estandarteAward);
int schoolInfoSave(SchoolInfo* info, const char* filepath);
SchoolInfo* schoolInfoLoad(const char* filepath);

#endif
