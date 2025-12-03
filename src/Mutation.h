#ifndef MUTATION_H
#define MUTATION_H
#include "parsers/Indexer.h"
#include "models/YearInfo.h"
#include "models/SchoolInfo.h"
#include "models/IndividualInfo.h"

int mutationRemoveYear(IndexerContext *indexer, int year);
int mutationRemoveSchool(IndexerContext *indexer, const char *schoolName);
int mutationRemoveIndividual(IndexerContext *indexer, const char *personName);


int mutationAddYear(IndexerContext *indexer, YearInfo *yearInfo);
int mutationAddSchool(IndexerContext *indexer, SchoolInfo *schoolInfo);
int mutationAddIndividual(IndexerContext *indexer, IndividualInfo *individualInfo);

#endif
