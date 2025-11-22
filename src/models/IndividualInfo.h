#ifndef INDIVIDUAL_INFO_H
#define INDIVIDUAL_INFO_H
#include "../../libs/LinkedList.h"

typedef struct {
    char personName[256];
    LinkedList* participationList;
} IndividualInfo;

IndividualInfo* individualInfoCreate(const char* personName);
void individualInfoFree(IndividualInfo* info);
void individualInfoPrint(IndividualInfo* info);
int individualInfoCompare(IndividualInfo* a, IndividualInfo* b);
void individualInfoAddParticipation(IndividualInfo* info, void* participation);

#endif
