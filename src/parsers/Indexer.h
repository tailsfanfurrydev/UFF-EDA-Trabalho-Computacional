#ifndef INDEXER_H
#define INDEXER_H

#include "Parser.h"
#include "../../libs/BPlusTree2M.h"
#include "../../libs/BPlusStorage.h"
#include "../../libs/HashMap2M.h"

typedef struct {
    BPlusTreeContext* yearIndex;
    HashMapM2Context* schoolIndex;
    HashMapM2Context* individualIndex;
} IndexerContext;

IndexerContext* indexerCreate(const char* baseDataPath);
void indexerFree(IndexerContext* context);

int indexerBuildFromParser(IndexerContext* context, ParserContext* parserContext, const char* baseDataPath);

char* indexerSearchYear(IndexerContext* context, int year);
char* indexerSearchSchool(IndexerContext* context, const char* schoolName);
char* indexerSearchIndividual(IndexerContext* context, const char* personName);

#endif
