#include "Mutation.h"
#include "parsers/Indexer.h"
#include "models/YearInfo.h"
#include "models/SchoolInfo.h"
#include "models/IndividualInfo.h"
#include "models/ChampionshipInfo.h"
#include "models/Participation.h"
#include "models/EstandarteAward.h"
#include "../libs/BPlusTree2M.h"
#include "../libs/HashMap2M.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



static void buildYearFilePath(char *buffer, size_t bufferSize, int year){
    snprintf(buffer, bufferSize, "data/YearInfo/year-%d.dat", year);
}
static void buildSchoolFilePath(char *buffer, size_t bufferSize, const char *schoolName){
    char cleanName[256];
    strncpy(cleanName, schoolName, 255);
    cleanName[255] = '\0';
    
    for(int i = 0; cleanName[i]; i++){
        if(cleanName[i] == ' ') cleanName[i] = '_';
    }
    
    snprintf(buffer, bufferSize, "data/SchoolInfo/school-%s.dat", cleanName);
}

static void buildIndividualFilePath(char *buffer, size_t bufferSize, const char *personName){
    char cleanName[256];
    strncpy(cleanName, personName, 255);
    cleanName[255] = '\0';
    
    for(int i = 0; cleanName[i]; i++){
        if(cleanName[i] == ' ') cleanName[i] = '_';
    }
    
    snprintf(buffer, bufferSize, "data/IndividualInfo/individual-%s.dat", cleanName);
}

static LinkedList* filterListByYear(LinkedList *list, int year, int isChampionship){
    LinkedList *newList = NULL;
    LinkedList *current = list;
    
    while(current){
        int shouldKeep = 1;
        
        if(isChampionship){
            ChampionshipInfo *info = (ChampionshipInfo*)current->info;
            if(info->year == year) shouldKeep = 0;
        } else{
            EstandarteAward *award = (EstandarteAward*)current->info;
            if(award->year == year) shouldKeep = 0;
        }
        
        if(shouldKeep){
            if(isChampionship){
                ChampionshipInfo *original = (ChampionshipInfo*)current->info;
                ChampionshipInfo *copy = championshipInfoCreate(
                    original->year, original->titleNumber, original->schoolName,
                    original->theme, original->carnivalDesigner
                );
                newList = linkedListInsert(newList, copy);
            } else{
                EstandarteAward *original = (EstandarteAward*)current->info;
                EstandarteAward *copy = estandarteAwardCreate(
                    original->year, original->category, original->winner
                );
                newList = linkedListInsert(newList, copy);
            }
        }
        
        current = current->next;
    }
    
    return newList;
}

static LinkedList* filterParticipationsByYear(LinkedList *list, int year){
    LinkedList *newList = NULL;
    LinkedList *current = list;
    
    while(current){
        Participation *part = (Participation*)current->info;
        
        if(part->year != year){
            Participation *copy = participationCreate(
                part->schoolName, part->category, part->year
            );
            newList = linkedListInsert(newList, copy);
        }
        
        current = current->next;
    }
    
    return newList;
}

static LinkedList* filterParticipationsBySchool(LinkedList *list, const char *schoolName){
    LinkedList *newList = NULL;
    LinkedList *current = list;
    
    while(current){
        Participation *part = (Participation*)current->info;
        
        if(strcmp(part->schoolName, schoolName) != 0){
            Participation *copy = participationCreate(
                part->schoolName, part->category, part->year
            );
            newList = linkedListInsert(newList, copy);
        }
        
        current = current->next;
    }
    
    return newList;
}
//remover dados tambem da yeatr ok?
static LinkedList* filterChampionshipsBySchool(LinkedList *list, const char *schoolName){
    LinkedList *newList = NULL;
    LinkedList *current = list;
    
    while(current){
        ChampionshipInfo *champ = (ChampionshipInfo*)current->info;
        
        if(strcmp(champ->schoolName, schoolName) != 0){
            ChampionshipInfo *copy = championshipInfoCreate(
                champ->year, champ->titleNumber, champ->schoolName,
                champ->theme, champ->carnivalDesigner
            );
            newList = linkedListInsert(newList, copy);
        }
        
        current = current->next;
    }
    
    return newList;
}

int mutationRemoveYear(IndexerContext *indexer, int year){
    if(!indexer) return 0;
    
    char yearPath[512];
    buildYearFilePath(yearPath, sizeof(yearPath), year);
    
    if(access(yearPath, F_OK) != 0){
        printf("Erro ao remover: Ano %d nao existe\n", year);
        return 0;
    }
    
    YearInfo *yearInfo = yearInfoLoad(yearPath);
    if(!yearInfo){
        printf("Erro ao remover ano %d\n", year);
        return 0;
    }
    
    LinkedList *championsNode = yearInfo->champions;
    while(championsNode){
        ChampionshipInfo *champ = (ChampionshipInfo*)championsNode->info;
        
        char *schoolPath = indexerSearchSchool(indexer, champ->schoolName);
        if(schoolPath){
            SchoolInfo *school = schoolInfoLoad(schoolPath);
            if(school){
                LinkedList *oldTitles = school->titleList;
                school->titleList = filterListByYear(oldTitles, year, 1);
                linkedListFree(oldTitles, (void(*)(void*))championshipInfoFree);
                LinkedList *oldAwards = school->awardList;
                school->awardList = filterListByYear(oldAwards, year, 0);
                linkedListFree(oldAwards, (void(*)(void*))estandarteAwardFree);
                
                schoolInfoSave(school, schoolPath);
                schoolInfoFree(school);
            }
            free(schoolPath);
        }
        
        championsNode = championsNode->next;
    }
    
    LinkedList *runnersUpNode = yearInfo->runnersUp;
    while(runnersUpNode){
        ChampionshipInfo *runner = (ChampionshipInfo*)runnersUpNode->info;
        
        char *schoolPath = indexerSearchSchool(indexer, runner->schoolName);
        if(schoolPath){
            SchoolInfo *school = schoolInfoLoad(schoolPath);
            if(school){
                LinkedList *oldRunners = school->runnerUpList;
                school->runnerUpList = filterListByYear(oldRunners, year, 1);
                linkedListFree(oldRunners, (void(*)(void*))championshipInfoFree);
                
                LinkedList *oldAwards = school->awardList;
                school->awardList = filterListByYear(oldAwards, year, 0);
                linkedListFree(oldAwards, (void(*)(void*))estandarteAwardFree);
                
                schoolInfoSave(school, schoolPath);
                schoolInfoFree(school);
            }
            free(schoolPath);
        }
        
        runnersUpNode = runnersUpNode->next;
    }
    
    char cmdSchools[1024];
    snprintf(cmdSchools, sizeof(cmdSchools), "find data/SchoolInfo -name '*.dat' 2>/dev/null");
    FILE *fpSchools = popen(cmdSchools, "r");
    if(fpSchools){
        char schoolPath[512];
        while(fgets(schoolPath, sizeof(schoolPath), fpSchools)){
            schoolPath[strcspn(schoolPath, "\n")] = 0;
            
            SchoolInfo *school = schoolInfoLoad(schoolPath);
            if(school){
                LinkedList *oldAwards = school->awardList;
                LinkedList *newAwards = filterListByYear(oldAwards, year, 0);
                
                if(linkedListSize(oldAwards) != linkedListSize(newAwards)){
                    school->awardList = newAwards;
                    linkedListFree(oldAwards, (void(*)(void*))estandarteAwardFree);
                    schoolInfoSave(school, schoolPath);
                } else{
                    linkedListFree(newAwards, (void(*)(void*))estandarteAwardFree);
                }
                
                schoolInfoFree(school);
            }
        }
        pclose(fpSchools);
    }
    
    yearInfoFree(yearInfo);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "find data/IndividualInfo -name '*.dat' 2>/dev/null");
    FILE *fp = popen(cmd, "r");
    if(fp){
        char individualPath[512];
        while(fgets(individualPath, sizeof(individualPath), fp)){
            individualPath[strcspn(individualPath, "\n")] = 0;
            
            IndividualInfo *individual = individualInfoLoad(individualPath);
            if(individual){
                LinkedList *oldParts = individual->participationList;
                individual->participationList = filterParticipationsByYear(oldParts, year);
                
                if(oldParts != individual->participationList){
                    linkedListFree(oldParts, (void(*)(void*))participationFree);
                    individualInfoSave(individual, individualPath);
                }
                
                individualInfoFree(individual);
            }
        }
        pclose(fp);
    }
    
    if(unlink(yearPath) != 0){
        printf("Erro ao remover ano %d\n", year);
        return 0;
    }
    
    bPlusTreeRemove2M(indexer->yearIndex->indexFile, year, indexer->yearIndex->t);
    
    printf("Ano %d removido com sucesso\n", year);
    return 1;
}

int mutationRemoveSchool(IndexerContext *indexer, const char *schoolName){
    if(!indexer || !schoolName) return 0;
    
    char schoolPath[512];
    buildSchoolFilePath(schoolPath, sizeof(schoolPath), schoolName);
    
    if(access(schoolPath, F_OK) != 0){
        printf("Erro ao remover: Escola '%s' nao existe\n", schoolName);
        return 0;
    }
    
    char cmd[1024];
    
    snprintf(cmd, sizeof(cmd), "find data/YearInfo -name '*.dat' 2>/dev/null");
    FILE *fpYears = popen(cmd, "r");
    if(fpYears){
        char yearPath[512];
        while(fgets(yearPath, sizeof(yearPath), fpYears)){
            yearPath[strcspn(yearPath, "\n")] = 0;
            
            YearInfo *yearInfo = yearInfoLoad(yearPath);
            if(yearInfo){
                int modified = 0;
                
                LinkedList *oldChampions = yearInfo->champions;
                LinkedList *newChampions = filterChampionshipsBySchool(oldChampions, schoolName);
                
                if(linkedListSize(oldChampions) != linkedListSize(newChampions)){
                    yearInfo->champions = newChampions;
                    linkedListFree(oldChampions, (void(*)(void*))championshipInfoFree);
                    modified = 1;
                } else{
                    linkedListFree(newChampions, (void(*)(void*))championshipInfoFree);
                }
                
                LinkedList *oldRunnersUp = yearInfo->runnersUp;
                LinkedList *newRunnersUp = filterChampionshipsBySchool(oldRunnersUp, schoolName);
                
                if(linkedListSize(oldRunnersUp) != linkedListSize(newRunnersUp)){
                    yearInfo->runnersUp = newRunnersUp;
                    linkedListFree(oldRunnersUp, (void(*)(void*))championshipInfoFree);
                    modified = 1;
                } else{
                    linkedListFree(newRunnersUp, (void(*)(void*))championshipInfoFree);
                }
                
                if(modified){
                    yearInfoSave(yearInfo, yearPath);
                }
                
                yearInfoFree(yearInfo);
            }
        }
        pclose(fpYears);
    }
    
    snprintf(cmd, sizeof(cmd), "find data/IndividualInfo -name '*.dat' 2>/dev/null");
    FILE *fpIndividuals = popen(cmd, "r");
    if(fpIndividuals){
        char individualPath[512];
        while(fgets(individualPath, sizeof(individualPath), fpIndividuals)){
            individualPath[strcspn(individualPath, "\n")] = 0;
            
            IndividualInfo *individual = individualInfoLoad(individualPath);
            if(individual){
                LinkedList *oldParts = individual->participationList;
                LinkedList *newParts = filterParticipationsBySchool(oldParts, schoolName);
                
                if(linkedListSize(oldParts) != linkedListSize(newParts)){
                    individual->participationList = newParts;
                    linkedListFree(oldParts, (void(*)(void*))participationFree);
                    individualInfoSave(individual, individualPath);
                } else{
                    linkedListFree(newParts, (void(*)(void*))participationFree);
                }
                
                individualInfoFree(individual);
            }
        }
        pclose(fpIndividuals);
    }
    
    if(unlink(schoolPath) != 0){
        printf("Erro ao remover escola '%s'\n", schoolName);
        return 0;
    }
    
    hashMapM2Remove(indexer->schoolIndex, schoolName);
    
    printf("Escola '%s' removida com sucesso\n", schoolName);
    return 1;
}

int mutationRemoveIndividual(IndexerContext *indexer, const char *personName){
    if(!indexer || !personName) return 0;
    
    char individualPath[512];
    buildIndividualFilePath(individualPath, sizeof(individualPath), personName);
    
    if(access(individualPath, F_OK) != 0){
        printf("Erro ao remover: Individuo '%s' nao existe\n", personName);
        return 0;
    }
    
    if(unlink(individualPath) != 0){
        printf("Erro ao remover individuo '%s'\n", personName);
        return 0;
    }
    hashMapM2Remove(indexer->individualIndex, personName);
    
    printf("Individuo '%s' removido com sucesso\n", personName);
    return 1;
}

int mutationAddYear(IndexerContext *indexer, YearInfo *yearInfo){
    if(!indexer || !yearInfo) return 0;
    
    char yearPath[512];
    buildYearFilePath(yearPath, sizeof(yearPath), yearInfo->year);
    
    if(access(yearPath, F_OK) == 0){
        printf("Erro ao adicionar: Ano %d ja existe\n", yearInfo->year);
        return 0;
    }
    
    if(yearInfoSave(yearInfo, yearPath) != 0){
        printf("Erro ao adicionar ano %d\n", yearInfo->year);
        return 0;
    }
    
    bPlusTreeInsert2M(indexer->yearIndex->indexFile, yearInfo->year, indexer->yearIndex->t);
    
    LinkedList *championsNode = yearInfo->champions;
    while(championsNode){
        ChampionshipInfo *champ = (ChampionshipInfo*)championsNode->info;
        
        char *schoolPath = indexerSearchSchool(indexer, champ->schoolName);
        if(schoolPath){
            SchoolInfo *school = schoolInfoLoad(schoolPath);
            if(school){
                ChampionshipInfo *copy = championshipInfoCreate(
                    champ->year, champ->titleNumber, champ->schoolName,
                    champ->theme, champ->carnivalDesigner
                );
                schoolInfoAddTitle(school, copy);
                schoolInfoSave(school, schoolPath);
                schoolInfoFree(school);
            }
            free(schoolPath);
        }
        
        championsNode = championsNode->next;
    }
    
    LinkedList *runnersUpNode = yearInfo->runnersUp;
    while(runnersUpNode){
        ChampionshipInfo *runner = (ChampionshipInfo*)runnersUpNode->info;
        
        char *schoolPath = indexerSearchSchool(indexer, runner->schoolName);
        if(schoolPath){
            SchoolInfo *school = schoolInfoLoad(schoolPath);
            if(school){
                ChampionshipInfo *copy = championshipInfoCreate(
                    runner->year, runner->titleNumber, runner->schoolName,
                    runner->theme, runner->carnivalDesigner
                );
                schoolInfoAddRunnerUp(school, copy);
                schoolInfoSave(school, schoolPath);
                schoolInfoFree(school);
            }
            free(schoolPath);
        }
        
        runnersUpNode = runnersUpNode->next;
    }
    
    printf("Ano %d adicionado com sucesso\n", yearInfo->year);
    return 1;
}

int mutationAddSchool(IndexerContext *indexer, SchoolInfo *schoolInfo){
    if(!indexer || !schoolInfo) return 0;
    
    char schoolPath[512];
    buildSchoolFilePath(schoolPath, sizeof(schoolPath), schoolInfo->schoolName);
    
    if(access(schoolPath, F_OK) == 0){
        printf("Erro ao adicionar: Escola '%s' ja existe\n", schoolInfo->schoolName);
        return 0;
    }
    
    if(schoolInfoSave(schoolInfo, schoolPath) != 0){
        printf("Erro ao adicionar escola '%s'\n", schoolInfo->schoolName);
        return 0;
    }
    
    hashMapM2Insert(indexer->schoolIndex, schoolInfo->schoolName, schoolPath);
    
    printf("Escola '%s' adicionada com sucesso\n", schoolInfo->schoolName);
    return 1;
}

int mutationAddIndividual(IndexerContext *indexer, IndividualInfo *individualInfo){
    if(!indexer || !individualInfo) return 0;
    
    char individualPath[512];
    buildIndividualFilePath(individualPath, sizeof(individualPath), individualInfo->personName);
    
    if(access(individualPath, F_OK) == 0){
        printf("Erro ao adicionar: Individuo '%s' ja existe\n", individualInfo->personName);
        return 0;
    }
    
    if(individualInfoSave(individualInfo, individualPath) != 0){
        printf("Erro ao adicionar individuo '%s'\n", individualInfo->personName);
        return 0;
    }
    
    hashMapM2Insert(indexer->individualIndex, individualInfo->personName, individualPath);
    
    printf("Individuo '%s' adicionado com sucesso\n", individualInfo->personName);
    return 1;
}
