#ifndef PARSER_H
#define PARSER_H

#include "../models/YearInfo.h"
#include "../models/SchoolInfo.h"
#include "../models/IndividualInfo.h"
#include "../../libs/LinkedList.h"

typedef struct {
    LinkedList* yearInfoList;
    LinkedList* schoolInfoList;
    LinkedList* individualInfoList;
} ParserContext;

ParserContext* parserContextCreate();
void parserContextFree(ParserContext* context);

int parseFiles(const char* campeasPath, const char* estandartesPath);
int parseCampeas(ParserContext* context, const char* filePath);
int parseEstandartes(ParserContext* context, const char* filePath);
int saveAllStructures(ParserContext* context);
ParserContext* parseAndIndex(const char* campeasPath, const char* estandartesPath);

#endif
