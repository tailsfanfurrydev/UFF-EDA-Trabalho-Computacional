#include "Questionary.h"
#include "models/YearInfo.h"
#include "models/SchoolInfo.h"
#include "models/IndividualInfo.h"
#include "models/ChampionshipInfo.h"
#include "models/EstandarteAward.h"
#include "models/Participation.h"
#include "../libs/BPlusTree2M.h"
#include "../libs/HashMapM2.h"
#include "../libs/BPlusStorage.h"
#include "../libs/LinkedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct{
    char schoolName[256];
    int count;
} SchoolCount;

typedef struct{
    char personName[256];
    int count;
} PersonCount;

typedef struct{
    char category[128];
    char personName[256];
    int count;
} CategoryWinner;

typedef struct{
    int startYear;
    int count;
} ConsecutiveWins;
typedef struct{
    SchoolCount *schools;
    int capacity;
    int count;
} SchoolCountList;
typedef struct{
    PersonCount *persons;
    int capacity;
    int count;
} PersonCountList;

static void trim(char* str){
    if(!str) return;
    char* start = str;
    while(isspace((unsigned char)*start)) start++;
    char* end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    if(start != str) memmove(str, start, strlen(start) + 1);
}

static int isExtinct(const char* schoolName){
    return strstr(schoolName, "[EXTINTA]") != NULL;
}
static void questionACallback(const char *key, const char *filePath, void *userData){
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school || !school->titleList || !school->awardList){
        if(school) schoolInfoFree(school);
        return;
    }
    
    LinkedList *title = school->titleList;
    while(title && title->info){
        ChampionshipInfo *champInfo = (ChampionshipInfo*)title->info;
        int championYear = champInfo->year;
        
        //checar se tem melhor escola
        LinkedList *award = school->awardList;
        while(award && award->info){
            EstandarteAward *estandarte = (EstandarteAward*)award->info;
            if(estandarte->year == championYear && 
               strcmp(estandarte->category, "Melhor Escola") == 0){
                printf("%d - %s\n", championYear, school->schoolName);
                break;
            }
            award = award->next;
        }
        
        title = title->next;
    }
    
    schoolInfoFree(school);
}

void questionA(IndexerContext *indexer){
    printf("\n=== QUESTAO A ===\n");
    printf("Anos e escolas em que a campea ganhou Estandarte de melhor escola:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    hashMapM2ForEach(indexer->schoolIndex, questionACallback, NULL);
    
    printf("\n");
}

typedef struct{
    char schoolName[256];
    int firstYear;
    int lastYear;
    int distance;
} SchoolDistance;

typedef struct{
    SchoolDistance *distances;
    int capacity;
    int count;
} QuestionBData;
static void questionBCallback(const char *key, const char *filePath, void *userData){
    QuestionBData *data = (QuestionBData*)userData;
    
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school || !school->titleList){
        if(school) schoolInfoFree(school);
        return;
    }
    
    int firstYear = -1, lastYear = -1;
    LinkedList *title = school->titleList;
    while(title && title->info){
        ChampionshipInfo *champInfo = (ChampionshipInfo*)title->info;
        if(firstYear == -1 || champInfo->year < firstYear) firstYear = champInfo->year;
        if(lastYear == -1 || champInfo->year > lastYear) lastYear = champInfo->year;
        title = title->next;
    }
    
    if(firstYear != -1 && lastYear != -1 && firstYear != lastYear){
        if(data->count >= data->capacity){
            data->capacity = data->capacity == 0 ? 64 : data->capacity * 2;
            data->distances = realloc(data->distances, data->capacity * sizeof(SchoolDistance));
        }
        
        strncpy(data->distances[data->count].schoolName, school->schoolName, 255);
        data->distances[data->count].schoolName[255] = '\0';
        data->distances[data->count].firstYear = firstYear;
        data->distances[data->count].lastYear = lastYear;
        data->distances[data->count].distance = lastYear - firstYear;
        data->count++;
    }
    
    schoolInfoFree(school);
}

void questionB(IndexerContext *indexer){
    printf("\n=== QUESTAO B ===\n");
    printf("Distancias entre titulos das escolas (ordem decrescente):\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    QuestionBData data ={NULL, 0, 0};
    hashMapM2ForEach(indexer->schoolIndex, questionBCallback, &data);

    for(int i = 0; i < data.count - 1; i++){
        for(int j = i + 1; j < data.count; j++){
            if(data.distances[j].distance > data.distances[i].distance){
                SchoolDistance temp = data.distances[i];
                data.distances[i] = data.distances[j];
                data.distances[j] = temp;
            }
        }
    }
    
    for(int i = 0; i < data.count; i++){
        printf("%s: %d anos\n", data.distances[i].schoolName, data.distances[i].distance);
    }
    
    if(data.distances) free(data.distances);
    printf("\n");
}

static void questionCCallback(const char *key, const char *filePath, void *userData){
    SchoolCountList *list = (SchoolCountList*)userData;
    
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school) return;
    
    int awardCount = linkedListSize(school->awardList);
    
    if(list->count >= list->capacity){
        list->capacity = list->capacity == 0 ? 64 : list->capacity * 2;
        list->schools = realloc(list->schools, list->capacity * sizeof(SchoolCount));
    }
    
    strncpy(list->schools[list->count].schoolName, key, 255);
    list->schools[list->count].schoolName[255] = '\0';
    list->schools[list->count].count = awardCount;
    list->count++;
    
    schoolInfoFree(school);
}

void questionC(IndexerContext *indexer){
    printf("\n=== QUESTAO C ===\n");
    printf("Escola(s) com mais estandartes na historia:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    SchoolCountList list ={NULL, 0, 0};
    hashMapM2ForEach(indexer->schoolIndex, questionCCallback, &list);
    
    if(list.count == 0){
        printf("Nenhuma escola encontrada\n");
        if(list.schools) free(list.schools);
        return;
    }
    
    int maxIdx = 0;
    for(int i = 1; i < list.count; i++){
        if(list.schools[i].count > list.schools[maxIdx].count){
            maxIdx = i;
        }
    }
    
    int maxCount = list.schools[maxIdx].count;
    for(int i = 0; i < list.count; i++){
        if(list.schools[i].count == maxCount){
            printf("%s: %d estandartes\n", list.schools[i].schoolName, list.schools[i].count);
        }
    }
    
    free(list.schools);
    printf("\n");
}

static void questionDCallback(const char *key, const char *filePath, void *userData){
    SchoolCountList *list = (SchoolCountList*)userData;
    
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school) return;
    
    int awardCount = linkedListSize(school->awardList);
    
    if(awardCount > 0){
        if(list->count >= list->capacity){
            list->capacity = list->capacity == 0 ? 64 : list->capacity * 2;
            list->schools = realloc(list->schools, list->capacity * sizeof(SchoolCount));
        }
        
        strncpy(list->schools[list->count].schoolName, key, 255);
        list->schools[list->count].schoolName[255] = '\0';
        list->schools[list->count].count = awardCount;
        list->count++;
    }
    
    schoolInfoFree(school);
}

void questionD(IndexerContext *indexer){
    printf("\n=== QUESTAO D ===\n");
    printf("Escola(s) com menos estandartes na historia:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    SchoolCountList list ={NULL, 0, 0};
    hashMapM2ForEach(indexer->schoolIndex, questionDCallback, &list);
    
    if(list.count == 0){
        printf("Nenhuma escola com estandartes encontrada\n");
        if(list.schools) free(list.schools);
        return;
    }
    
    int minIdx = 0;
    for(int i = 1; i < list.count; i++){
        if(list.schools[i].count < list.schools[minIdx].count){
            minIdx = i;
        }
    }
    
    int minCount = list.schools[minIdx].count;
    for(int i = 0; i < list.count; i++){
        if(list.schools[i].count == minCount){
            printf("%s: %d estandartes\n", list.schools[i].schoolName, list.schools[i].count);
        }
    }
    
    free(list.schools);
    printf("\n");
}

typedef struct{
    int year;
    char schoolName[256];
    int count;
} YearWinner;

typedef struct{
    IndexerContext *indexer;
    int currentYear;
    YearWinner *winners;
    int winnerCount;
    int winnerCapacity;
} QuestionEData;

static void questionECallback(const char *key, const char *filePath, void *userData){
    QuestionEData *data = (QuestionEData*)userData;
    
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school || !school->awardList){
        if(school) schoolInfoFree(school);
        return;
    }
    
    int countThisYear = 0;
    LinkedList *award = school->awardList;
    while(award && award->info){
        EstandarteAward *estandarte = (EstandarteAward*)award->info;
        if(estandarte->year == data->currentYear) countThisYear++;
        award = award->next;
    }
    
    if(countThisYear > 0){
        if(data->winnerCount >= data->winnerCapacity){
            data->winnerCapacity = data->winnerCapacity == 0 ? 64 : data->winnerCapacity * 2;
            data->winners = realloc(data->winners, data->winnerCapacity * sizeof(YearWinner));
        }
        
        data->winners[data->winnerCount].year = data->currentYear;
        strncpy(data->winners[data->winnerCount].schoolName, school->schoolName, 255);
        data->winners[data->winnerCount].schoolName[255] = '\0';
        data->winners[data->winnerCount].count = countThisYear;
        data->winnerCount++;
    }
    
    schoolInfoFree(school);
}

void questionE(IndexerContext *indexer){
    printf("\n=== QUESTAO E ===\n");
    printf("Maiores ganhadoras de estandarte por ano:\n\n");
    
    if(!indexer || !indexer->yearIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    QuestionEData data ={indexer, 0, NULL, 0, 0};
    
    for(int year = 1972; year <= 2025; year++){
        data.currentYear = year;
        int countBefore = data.winnerCount;
        
        hashMapM2ForEach(indexer->schoolIndex, questionECallback, &data);

        int maxCount = 0;
        for(int i = countBefore; i < data.winnerCount; i++){
            if(data.winners[i].count > maxCount){
                maxCount = data.winners[i].count;
            }
        }
        
        for(int i = countBefore; i < data.winnerCount; i++){
            if(data.winners[i].count == maxCount){
                printf("%d - %s: %d estandartes\n", year, data.winners[i].schoolName, data.winners[i].count);
            }
        }
    }
    
    if(data.winners) free(data.winners);
    printf("\n");
}




static void questionFCallback(const char *key, const char *filePath, void *userData){
    PersonCountList *list = (PersonCountList*)userData;
    
    IndividualInfo *person = individualInfoLoad(filePath);
    if(!person) return;
    
    int count = linkedListSize(person->participationList);
    
    if(count > 0){
        if(list->count >= list->capacity){
            list->capacity = list->capacity == 0 ? 64 : list->capacity * 2;
            list->persons = realloc(list->persons, list->capacity * sizeof(PersonCount));
        }
        
        strncpy(list->persons[list->count].personName, person->personName, 255);
        list->persons[list->count].personName[255] = '\0';
        list->persons[list->count].count = count;
        list->count++;
    }
    
    individualInfoFree(person);
}

void questionF(IndexerContext *indexer){
    printf("\n=== QUESTAO F ===\n");
    printf("Maiores ganhadores individuais de estandarte:\n\n");
    
    if(!indexer || !indexer->individualIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    PersonCountList list ={NULL, 0, 0};
    hashMapM2ForEach(indexer->individualIndex, questionFCallback, &list);
    
    if(list.count == 0){
        printf("Nenhum individuo encontrado\n");
        if(list.persons) free(list.persons);
        return;
    }
    
    int maxCount = 0;
    for(int i = 0; i < list.count; i++){
        if(list.persons[i].count > maxCount){
            maxCount = list.persons[i].count;
        }
    }
    
    for(int i = 0; i < list.count; i++){
        if(list.persons[i].count == maxCount){
            printf("%s: %d estandartes\n", list.persons[i].personName, list.persons[i].count);
        }
    }
    
    free(list.persons);
    printf("\n");
}


typedef struct{
    CategoryWinner *winners;
    int capacity;
    int count;
} QuestionGData;
void questionGCallback(const char *key, const char *filePath, void *userData){
    QuestionGData *data = (QuestionGData*)userData;
    
    IndividualInfo *person = individualInfoLoad(filePath);
    if(!person || !person->participationList){
        if(person) individualInfoFree(person);
        return;
    }
    
    typedef struct{
        char category[128];
        int count;
    } CategoryCount;
    
    CategoryCount *categories = NULL;
    int catCount = 0;
    
    LinkedList *part = person->participationList;
    while(part && part->info){
        Participation *participation = (Participation*)part->info;
        
        int found = 0;
        for(int i = 0; i < catCount; i++){
            if(strcmp(categories[i].category, participation->category) == 0){
                categories[i].count++;
                found = 1;
                break;
            }
        }
        
        if(!found){
            categories = realloc(categories, (catCount + 1) * sizeof(CategoryCount));
            strncpy(categories[catCount].category, participation->category, 127);
            categories[catCount].category[127] = '\0';
            categories[catCount].count = 1;
            catCount++;
        }
        
        part = part->next;
    }
    
    for(int i = 0; i < catCount; i++){
        int found = -1;
        for(int j = 0; j < data->count; j++){
            if(strcmp(data->winners[j].category, categories[i].category) == 0){
                found = j;
                break;
            }
        }
        
        if(found >= 0){
            if(categories[i].count > data->winners[found].count){
                strncpy(data->winners[found].personName, person->personName, 255);
                data->winners[found].personName[255] = '\0';
                data->winners[found].count = categories[i].count;
            }
        } else{
            if(data->count >= data->capacity){
                data->capacity = data->capacity == 0 ? 64 : data->capacity * 2;
                data->winners = realloc(data->winners, data->capacity * sizeof(CategoryWinner));
            }
            
            strncpy(data->winners[data->count].category, categories[i].category, 127);
            data->winners[data->count].category[127] = '\0';
            strncpy(data->winners[data->count].personName, person->personName, 255);
            data->winners[data->count].personName[255] = '\0';
            data->winners[data->count].count = categories[i].count;
            data->count++;
        }
    }
    
    if(categories) free(categories);
    individualInfoFree(person);
}
void questionG(IndexerContext *indexer){
    printf("\n=== QUESTAO G ===\n");
    printf("Maiores ganhadores por quesito:\n\n");
    
    if(!indexer || !indexer->individualIndex){
        printf("Error: indices nao carregados\n");
        return;
    }
    
    QuestionGData data ={NULL, 0, 0};
    hashMapM2ForEach(indexer->individualIndex, questionGCallback, &data);
    
    for(int i = 0; i < data.count; i++){
        printf("%s: %s (%d vezes)\n", data.winners[i].category, data.winners[i].personName, data.winners[i].count);
    }
    
    if(data.winners) free(data.winners);
    printf("\n");
}
static void questionHCallback(const char *key, const char *filePath, void *userData){
    IndividualInfo *person = individualInfoLoad(filePath);
    if(!person || !person->participationList){
        if(person) individualInfoFree(person);
        return;
    }
    
    char categories[100][128];
    int catCount = 0;
    
    LinkedList *part = person->participationList;
    while(part && part->info){
        Participation *participation = (Participation*)part->info;
        
        int found = 0;
        for(int i = 0; i < catCount; i++){
            if(strcmp(categories[i], participation->category) == 0){
                found = 1;
                break;
            }
        }
        
        if(!found && catCount < 100){
            strncpy(categories[catCount], participation->category, 127);
            categories[catCount][127] = '\0';
            catCount++;
        }
        
        part = part->next;
    }
    
    if(catCount > 1){
        printf("%s: %d categorias (", person->personName, catCount);
        for(int i = 0; i < catCount; i++){
            printf("%s", categories[i]);
            if(i < catCount - 1) printf(", ");
        }
        printf(")\n");
    }
    
    individualInfoFree(person);
}

void questionH(IndexerContext *indexer){
    printf("\n=== QUESTAO H ===\n");
    printf("individuos que ganharam em categorias distintas:\n\n");
    
    if(!indexer || !indexer->individualIndex){
        printf("Error: indices nao carregados\n");
        return;
    }
    
    hashMapM2ForEach(indexer->individualIndex, questionHCallback, NULL);
    
    printf("\n");
}



typedef struct{
    const char *categoryName;
} QuestionGenericData;

static void questionGenericMultipleSchoolsCallback(const char *key, const char *filePath, void *userData){
    QuestionGenericData *data = (QuestionGenericData*)userData;
    
    IndividualInfo *person = individualInfoLoad(filePath);
    if(!person || !person->participationList){
        if(person) individualInfoFree(person);
        return;
    }
    
    char schools[100][256];
    int schoolCount = 0;
    
    LinkedList *part = person->participationList;
    while(part && part->info){
        Participation *participation = (Participation*)part->info;
        
        if(strcmp(participation->category, data->categoryName) == 0){
            int found = 0;
            for(int i = 0; i < schoolCount; i++){
                if(strcmp(schools[i], participation->schoolName) == 0){
                    found = 1;
                    break;
                }
            }
            
            if(!found && schoolCount < 100){
                strncpy(schools[schoolCount], participation->schoolName, 255);
                schools[schoolCount][255] = '\0';
                schoolCount++;
            }
        }
        
        part = part->next;
    }
    
    if(schoolCount > 1){
        printf("%s: %d escolas (", person->personName, schoolCount);
        for(int i = 0; i < schoolCount; i++){
            printf("%s", schools[i]);
            if(i < schoolCount - 1) printf(", ");
        }
        printf(")\n");
    }
    
    individualInfoFree(person);
}

static void questionGenericMultipleSchools(IndexerContext *indexer, const char *categoryName, const char *title){
    printf("\n=== %s ===\n", title);
    printf("%s que ganharam em escolas distintas:\n\n", categoryName);
    if(!indexer || !indexer->individualIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    QuestionGenericData data ={categoryName};
    hashMapM2ForEach(indexer->individualIndex, questionGenericMultipleSchoolsCallback, &data);
    
    printf("\n");
}

void questionI(IndexerContext *indexer){
    questionGenericMultipleSchools(indexer, "Mestre-sala", "QUESTAO I");
}

void questionJ(IndexerContext *indexer){
    questionGenericMultipleSchools(indexer, "Porta-bandeira", "QUESTAO J");
}

void questionL(IndexerContext *indexer){
    questionGenericMultipleSchools(indexer, "Intérprete", "QUESTAO L");
}

void questionN(IndexerContext *indexer){
    questionGenericMultipleSchools(indexer, "Bateria", "QUESTAO N");
}


typedef struct{
    char carnavalesco[256];
    char schools[100][256];
    int schoolCount;
} CarnavalescoInfo;

typedef struct{
    CarnavalescoInfo *carnavalescos;
    int capacity;
    int count;
} QuestionMData;

static void questionMCallback(const char *key, const char *filePath, void *userData){
    QuestionMData *data = (QuestionMData*)userData;
    
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school || !school->titleList){
        if(school) schoolInfoFree(school);
        return;
    }
    
    LinkedList *title = school->titleList;
    while(title && title->info){
        ChampionshipInfo *champInfo = (ChampionshipInfo*)title->info;
        
        if(strcmp(champInfo->carnivalDesigner, "*") != 0){
            int found = -1;
            for(int i = 0; i < data->count; i++){
                if(strcmp(data->carnavalescos[i].carnavalesco, champInfo->carnivalDesigner) == 0){
                    found = i;
                    break;
                }
            }
            
            if(found >= 0){
                int schoolFound = 0;
                for(int j = 0; j < data->carnavalescos[found].schoolCount; j++){
                    if(strcmp(data->carnavalescos[found].schools[j], school->schoolName) == 0){
                        schoolFound = 1;
                        break;
                    }
                }
                if(!schoolFound && data->carnavalescos[found].schoolCount < 100){
                    strncpy(data->carnavalescos[found].schools[data->carnavalescos[found].schoolCount], school->schoolName, 255);
                    data->carnavalescos[found].schools[data->carnavalescos[found].schoolCount][255] = '\0';
                    data->carnavalescos[found].schoolCount++;
                }
            } else{
                if(data->count >= data->capacity){
                    data->capacity = data->capacity == 0 ? 64 : data->capacity * 2;
                    data->carnavalescos = realloc(data->carnavalescos, data->capacity * sizeof(CarnavalescoInfo));
                }
                strncpy(data->carnavalescos[data->count].carnavalesco, champInfo->carnivalDesigner, 255);
                data->carnavalescos[data->count].carnavalesco[255] = '\0';
                strncpy(data->carnavalescos[data->count].schools[0], school->schoolName, 255);
                data->carnavalescos[data->count].schools[0][255] = '\0';
                data->carnavalescos[data->count].schoolCount = 1;
                data->count++;
            }
        }
        
        title = title->next;
    }
    
    schoolInfoFree(school);
}




void questionM(IndexerContext *indexer){
    printf("\n=== QUESTAO M ===\n");
    printf("Carnavalescos que ganharam em escolas distintas:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    QuestionMData data ={NULL, 0, 0};
    hashMapM2ForEach(indexer->schoolIndex, questionMCallback, &data);
    
    for(int i = 0; i < data.count; i++){
        if(data.carnavalescos[i].schoolCount > 1){
            printf("%s: %d escolas (", data.carnavalescos[i].carnavalesco, data.carnavalescos[i].schoolCount);
            for(int j = 0; j < data.carnavalescos[i].schoolCount; j++){
                printf("%s", data.carnavalescos[i].schools[j]);
                if(j < data.carnavalescos[i].schoolCount - 1) printf(", ");
            }
            printf(")\n");
        }
    }
    
    if(data.carnavalescos) free(data.carnavalescos);
    printf("\n");
}





void questionO(IndexerContext *indexer){
    printf("\n=== QUESTAO O ===\n");
    printf("Maiores campeas por decada:\n\n");
    
    if(!indexer || !indexer->yearIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    int decades[][2] ={
       {1932, 1940},{1941, 1950},{1951, 1960},{1961, 1970},{1971, 1980},
       {1981, 1990},{1991, 2000},{2001, 2010},{2011, 2020},{2021, 2025}
    };
    
    for(int d = 0; d < 10; d++){
        int startYear = decades[d][0];
        int endYear = decades[d][1];
        
        SchoolCount *schools = NULL;
        int schoolCount = 0;
        int maxCount = 0;
        
        for(int year = startYear; year <= endYear; year++){
            char *yearPath = indexerSearchYear(indexer, year);
            if(!yearPath) continue;
            
            YearInfo *yearInfo = yearInfoLoad(yearPath);
            free(yearPath);
            if(!yearInfo || !yearInfo->champions){
                if(yearInfo) yearInfoFree(yearInfo);
                continue;
            }
            
            LinkedList *champ = yearInfo->champions;
            while(champ && champ->info){
                ChampionshipInfo *champInfo = (ChampionshipInfo*)champ->info;
                
                int found = -1;
                for(int i = 0; i < schoolCount; i++){
                    if(strcmp(schools[i].schoolName, champInfo->schoolName) == 0){
                        found = i;
                        break;
                    }
                }
                
                if(found >= 0){
                    schools[found].count++;
                    if(schools[found].count > maxCount) maxCount = schools[found].count;
                } else{
                    schools = realloc(schools, (schoolCount + 1) * sizeof(SchoolCount));
                    strncpy(schools[schoolCount].schoolName, champInfo->schoolName, 255);
                    schools[schoolCount].schoolName[255] = '\0';
                    schools[schoolCount].count = 1;
                    if(1 > maxCount) maxCount = 1;
                    schoolCount++;
                }
                
                champ = champ->next;
            }
            
            yearInfoFree(yearInfo);
        }
        
        printf("%d-%d:\n", startYear, endYear);
        for(int i = 0; i < schoolCount; i++){
            if(schools[i].count == maxCount){
                printf("  %s: %d titulos\n", schools[i].schoolName, schools[i].count);
            }
        }
        
        if(schools) free(schools);
    }
    
    printf("\n");
}

static void questionPCallback(const char *key, const char *filePath, void *userData){
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school || !school->titleList){
        if(school) schoolInfoFree(school);
        return;
    }
    
    int years[100];
    int yearCount = 0;
    
    LinkedList *title = school->titleList;
    while(title && title->info && yearCount < 100){
        ChampionshipInfo *champInfo = (ChampionshipInfo*)title->info;
        years[yearCount++] = champInfo->year;
        title = title->next;
    }
    for(int i = 0; i < yearCount - 1; i++){ //ordem
        for(int j = i + 1; j < yearCount; j++){
            if(years[j] < years[i]){
                int temp = years[i];
                years[i] = years[j];
                years[j] = temp;
            }
        }
    }
    int maxConsec = 1;
    int currentConsec = 1;
    int startYear = years[0];
    
    ConsecutiveWins *sequences = NULL;
    int seqCount = 0;
    
    for(int i = 1; i < yearCount; i++){
        if(years[i] == years[i-1] + 1){
            currentConsec++;
        } else{
            if(currentConsec >= 2){
                sequences = realloc(sequences, (seqCount + 1) * sizeof(ConsecutiveWins));
                sequences[seqCount].startYear = startYear;
                sequences[seqCount].count = currentConsec;
                seqCount++;
                if(currentConsec > maxConsec) maxConsec = currentConsec;
            }
            currentConsec = 1;
            startYear = years[i];
        }
    }
    
    if(currentConsec >= 2){
        sequences = realloc(sequences, (seqCount + 1) * sizeof(ConsecutiveWins));
        sequences[seqCount].startYear = startYear;
        sequences[seqCount].count = currentConsec;
        seqCount++;
        if(currentConsec > maxConsec) maxConsec = currentConsec;
    }
    
    if(maxConsec >= 2){
        for(int i = 0; i < seqCount; i++){
            if(sequences[i].count == maxConsec){
                char *type = (maxConsec == 2) ? "Bicampea" :
                             (maxConsec == 3) ? "Tricampea" :
                             (maxConsec == 4) ? "Tetracampea" :
                             (maxConsec == 5) ? "Pentacampea" :
                             (maxConsec == 6) ? "Hexacampea" :
                             (maxConsec == 7) ? "Heptacampea" : "Multicampea";
                
                printf("%s - %s: ", school->schoolName, type);
                for(int j = 0; j < sequences[i].count; j++){
                    printf("%d", sequences[i].startYear + j);
                    if(j < sequences[i].count - 1) printf(", ");
                }
                printf("\n");
            }
        }
    }
    
    if(sequences) free(sequences);
    schoolInfoFree(school);
}

void questionP(IndexerContext *indexer){
    printf("\n=== QUESTAO P ===\n");
    printf("Escolas com titulos consecutivos:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    hashMapM2ForEach(indexer->schoolIndex, questionPCallback, NULL);
    
    printf("\n");
}

void questionQ(IndexerContext *indexer){
    questionM(indexer);
}



typedef struct{
    char schoolName[256];
    int years[100];
    int yearCount;
} ViceInfo;

typedef struct{
    ViceInfo *vices;
    int capacity;
    int count;
} QuestionRData;

static void questionRCallback(const char *key, const char *filePath, void *userData){
    QuestionRData *data = (QuestionRData*)userData;
    
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school) return;
    
    int count = linkedListSize(school->runnerUpList);
    
    if(count > 0){
        if(data->count >= data->capacity){
            data->capacity = data->capacity == 0 ? 64 : data->capacity * 2;
            data->vices = realloc(data->vices, data->capacity * sizeof(ViceInfo));
        }
        
        strncpy(data->vices[data->count].schoolName, school->schoolName, 255);
        data->vices[data->count].schoolName[255] = '\0';
        data->vices[data->count].yearCount = 0;
        
        LinkedList *runner = school->runnerUpList;
        while(runner && runner->info && data->vices[data->count].yearCount < 100){
            ChampionshipInfo *champInfo = (ChampionshipInfo*)runner->info;
            data->vices[data->count].years[data->vices[data->count].yearCount++] = champInfo->year;
            runner = runner->next;
        }
        
        data->count++;
    }
    
    schoolInfoFree(school);
}

void questionR(IndexerContext *indexer){
    printf("\n=== QUESTAO R ===\n");
    printf("escolas com mais vice-campeonatos:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    QuestionRData data ={NULL, 0, 0};
    hashMapM2ForEach(indexer->schoolIndex, questionRCallback, &data);
    
    int maxVices = 0;
    for(int i = 0; i < data.count; i++){
        if(data.vices[i].yearCount > maxVices){
            maxVices = data.vices[i].yearCount;
        }
    }
    
    for(int i = 0; i < data.count; i++){
        if(data.vices[i].yearCount == maxVices){
            printf("%s: %d vices (", data.vices[i].schoolName, data.vices[i].yearCount);
            for(int j = 0; j < data.vices[i].yearCount; j++){
                printf("%d", data.vices[i].years[j]);
                if(j < data.vices[i].yearCount - 1) printf(", ");
            }
            printf(")\n");
        }
    }
    
    if(data.vices) free(data.vices);
    printf("\n");
}



static void questionSCallback(const char *key, const char *filePath, void *userData){
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school || !school->runnerUpList || !school->awardList){
        if(school) schoolInfoFree(school);
        return;
    }
    
    LinkedList *runner = school->runnerUpList;
    while(runner && runner->info){
        ChampionshipInfo *runnerInfo = (ChampionshipInfo*)runner->info;
        int runnerYear = runnerInfo->year;
        
        LinkedList *award = school->awardList;
        while(award && award->info){
            EstandarteAward *estandarte = (EstandarteAward*)award->info;
            if(estandarte->year == runnerYear && 
               strcmp(estandarte->category, "Melhor Escola") == 0){
                printf("%d - %s\n", runnerYear, school->schoolName);
                break;
            }
            award = award->next;
        }
        
        runner = runner->next;
    }
    
    schoolInfoFree(school);
}

void questionS(IndexerContext *indexer){
    printf("\n=== QUESTAO S ===\n");
    printf("Anos e escolas em que a vice-campea ganhou Estandarte de melhor escola:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    hashMapM2ForEach(indexer->schoolIndex, questionSCallback, NULL);
    
    printf("\n");
}


static void questionTCallback(const char *key, const char *filePath, void *userData){
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school) return;
    
    if(!school->isActive && school->titleList){
        printf("%s: ", school->schoolName);
        LinkedList *title = school->titleList;
        int first = 1;
        while(title && title->info){
            ChampionshipInfo *champInfo = (ChampionshipInfo*)title->info;
            if(!first) printf(", ");
            printf("%d", champInfo->year);
            first = 0;
            title = title->next;
        }
        printf("\n");
    }
    
    schoolInfoFree(school);
}

void questionT(IndexerContext *indexer){
    printf("\n=== QUESTAO T ===\n");
    printf("Escolas extintas que ganharam o Carnaval:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    hashMapM2ForEach(indexer->schoolIndex, questionTCallback, NULL);
    
    printf("\n");
}

static void questionUCallback(const char *key, const char *filePath, void *userData){
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school) return;
    
    if(!school->isActive && school->runnerUpList){
        printf("%s: ", school->schoolName);
        LinkedList *runner = school->runnerUpList;
        int first = 1;
        while(runner && runner->info){
            ChampionshipInfo *champInfo = (ChampionshipInfo*)runner->info;
            if(!first) printf(", ");
            printf("%d", champInfo->year);
            first = 0;
            runner = runner->next;
        }
        printf("\n");
    }
    
    schoolInfoFree(school);
}

void questionU(IndexerContext *indexer){
    printf("\n=== QUESTAO U ===\n");
    printf("Escolas extintas que foram vice-campeas:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    hashMapM2ForEach(indexer->schoolIndex, questionUCallback, NULL);
    
    printf("\n");
}



typedef struct{
    char schoolName[256];
    int startYear;
    int count;
} ConsecVice;

typedef struct{
    ConsecVice *results;
    int capacity;
    int count;
    int maxConsec;
} QuestionVData;

static void questionVCallback(const char *key, const char *filePath, void *userData){
    QuestionVData *data = (QuestionVData*)userData;
    
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school || !school->runnerUpList){
        if(school) schoolInfoFree(school);
        return;
    }
    
    int years[100];
    int yearCount = 0;
    
    LinkedList *runner = school->runnerUpList;
    while(runner && runner->info && yearCount < 100){
        ChampionshipInfo *champInfo = (ChampionshipInfo*)runner->info;
        years[yearCount++] = champInfo->year;
        runner = runner->next;
    }
    for(int i = 0; i < yearCount - 1; i++){
        for(int j = i + 1; j < yearCount; j++){
            if(years[j] < years[i]){
                int temp = years[i];
                years[i] = years[j];
                years[j] = temp;
            }
        }
    }
    
    int currentConsec = 1;
    int startYear = years[0];
    for(int i = 1; i < yearCount; i++){
        if(years[i] == years[i-1] + 1){
            currentConsec++;
        } else{
            if(currentConsec >= 2){
                if(data->count >= data->capacity){
                    data->capacity = data->capacity == 0 ? 64 : data->capacity * 2;
                    data->results = realloc(data->results, data->capacity * sizeof(ConsecVice));
                }
                
                strncpy(data->results[data->count].schoolName, school->schoolName, 255);
                data->results[data->count].schoolName[255] = '\0';
                data->results[data->count].startYear = startYear;
                data->results[data->count].count = currentConsec;
                if(currentConsec > data->maxConsec) data->maxConsec = currentConsec;
                data->count++;
            }
            currentConsec = 1;
            startYear = years[i];
        }
    }
    
    if(currentConsec >= 2){
        if(data->count >= data->capacity){
            data->capacity = data->capacity == 0 ? 64 : data->capacity * 2;
            data->results = realloc(data->results, data->capacity * sizeof(ConsecVice));
        }
        
        strncpy(data->results[data->count].schoolName, school->schoolName, 255);
        data->results[data->count].schoolName[255] = '\0';
        data->results[data->count].startYear = startYear;
        data->results[data->count].count = currentConsec;
        if(currentConsec > data->maxConsec) data->maxConsec = currentConsec;
        data->count++;
    }
    
    schoolInfoFree(school);
}

void questionV(IndexerContext *indexer){
    printf("\n=== QUESTAO V ===\n");
    printf("escolas commais vice-campeonatos consecutivos:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    QuestionVData data ={NULL, 0, 0, 0};
    hashMapM2ForEach(indexer->schoolIndex, questionVCallback, &data);
    
    for(int i = 0; i < data.count; i++){
        if(data.results[i].count == data.maxConsec){
            printf("%s: %d anos consecutivos (", data.results[i].schoolName, data.results[i].count);
            for(int j = 0; j < data.results[i].count; j++){
                printf("%d", data.results[i].startYear + j);
                if(j < data.results[i].count - 1) printf(", ");
            }
            printf(")\n");
        }
    }
    
    if(data.results) free(data.results);
    printf("\n");
}

static void questionXCallback(const char *key, const char *filePath, void *userData){
    SchoolInfo *school = schoolInfoLoad(filePath);
    if(!school) return;
    
    if(!school->isActive && school->awardList){
        int years[100];
        int yearCount = 0;
        
        LinkedList *award = school->awardList;
        while(award && award->info && yearCount < 100){
            EstandarteAward *estandarte = (EstandarteAward*)award->info;
            int found = 0;
            for(int i = 0; i < yearCount; i++){
                if(years[i] == estandarte->year){
                    found = 1;
                    break;
                }
            }
            if(!found) years[yearCount++] = estandarte->year;
            award = award->next;
        }
        
        if(yearCount > 0){
            printf("%s: ", school->schoolName);
            for(int i = 0; i < yearCount; i++){
                printf("%d", years[i]);
                if(i < yearCount - 1) printf(", ");
            }
            printf("\n");
        }
    }
    
    schoolInfoFree(school);
}

void questionX(IndexerContext *indexer){
    printf("\n=== QUESTAO X ===\n");
    printf("Escolas extintas que ganharam estandarte:\n\n");
    
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indices nao carregados\n");
        return;
    }
    
    hashMapM2ForEach(indexer->schoolIndex, questionXCallback, NULL);
    
    printf("\n");
}

// ===============================================
// QUESTAO ALL - Executa todas as questões
// ===============================================

void questionAll(IndexerContext *indexer) {
    printf("\n");
    printf("================================================================================\n");
    printf("                    EXECUTANDO TODAS AS QUESTOES                               \n");
    printf("================================================================================\n");
    
    questionA(indexer);
    questionB(indexer);
    questionC(indexer);
    questionD(indexer);
    questionE(indexer);
    questionF(indexer);
    questionG(indexer);
    questionH(indexer);
    questionI(indexer);
    questionJ(indexer);
    questionL(indexer);
    questionM(indexer);
    questionN(indexer);
    questionO(indexer);
    questionP(indexer);
    questionQ(indexer);
    questionR(indexer);
    questionS(indexer);
    questionT(indexer);
    questionU(indexer);
    questionV(indexer);
    questionX(indexer);
    
    printf("================================================================================\n");
    printf("                    TODAS AS QUESTOES EXECUTADAS                               \n");
    printf("================================================================================\n\n");
}

//finalmente
// eu nao aguentava mais
// chega
// por favor chega
// meus dedos tao doendo