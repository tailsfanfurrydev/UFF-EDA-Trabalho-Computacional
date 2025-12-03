#include "Parser.h"
#include "Indexer.h"
#include "../models/YearInfo.h"
#include "../models/SchoolInfo.h"
#include "../models/IndividualInfo.h"
#include "../models/Participation.h"
#include "../models/EstandarteAward.h"
#include "../models/ChampionshipInfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

static void trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    if(str != str) memmove(str, str, strlen(str) + 1);
}

static int isExtinct(const char* schoolName) {
    return strstr(schoolName, "[EXTINTA]") != NULL;
}

static void cleanSchoolName(char* dest, const char* src) {
    strncpy(dest, src, 255);
    dest[255] = '\0';
    
    char* extinct = strstr(dest, " [EXTINTA]");
    if(extinct) *extinct = '\0';

    char* extinct2 = strstr(dest, "[EXTINTA]");
    if(extinct2) *extinct2 = '\0';
    
    trim(dest);
}

static void cleanPersonName(char* name) {
    char* openParen = strchr(name, '(');
    if(openParen) {
        *openParen = '\0';
    }
    trim(name);
}

static int isValidPersonName(const char* name) {
    const char* invalidNames[] = {
        "Comissão de Frente",
        "Comissão de frente",
        "Comissão de carnaval",
        "Comissão de Carnaval",
        "Ala de Baianas",
        "Ala de baianas",
        "Ala de Passistas",
        "Ala de passistas",
        "Ala de Crianças",
        "Ala de crianças",
        "Bateria",
        "Samba-enredo",
        "Comunicação com o público",
        NULL
    };
    
    for(int i = 0; invalidNames[i] != NULL; i++) {
        if(strcasecmp(name, invalidNames[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

static int isSpecialAwardCategory(const char* category) {
    const char* specialCategories[] = {
        "Prêmio Especial",
        "Prêmio Especial (in memoriam)",
        "Prêmio Fernando Pamplona",
        NULL
    };
    
    for(int i = 0; specialCategories[i] != NULL; i++) {
        if(strcasecmp(category, specialCategories[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int isPersonCategory(const char* category) {
    const char* personCategories[] = {
        "Carnavalesco",
        "Intérprete",
        "Passista Feminino",
        "Passista Masculino",
        "Mestre-sala",
        "Porta-bandeira",
        "Destaque Feminino",
        "Destaque Masculino",
        "Personalidade",
        "Personalidade Feminina",
        "Personalidade Masculina",
        "Revelação",
        "Bateria",
        NULL
    };
    
    for(int i = 0; personCategories[i] != NULL; i++) {
        if(strcasecmp(category, personCategories[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static YearInfo* findOrCreateYearInfo(ParserContext* context, uint16_t year) {
    LinkedList* curr = context->yearInfoList;
    while(curr) {
        YearInfo* info = (YearInfo*)curr->info;
        if(info->year == year) return info;
        curr = curr->next;
    }
    
    YearInfo* newInfo = yearInfoCreate(year);
    context->yearInfoList = linkedListInsert(context->yearInfoList, newInfo);
    return newInfo;
}

static SchoolInfo* findOrCreateSchoolInfo(ParserContext* context, const char* schoolName, uint8_t isActive) {
    char cleanName[256];
    cleanSchoolName(cleanName, schoolName);
    
    LinkedList* curr = context->schoolInfoList;
    while(curr) {
        SchoolInfo* info = (SchoolInfo*)curr->info;
        if(strcmp(info->schoolName, cleanName) == 0) {
            if(!isActive && info->isActive) info->isActive = 0;
            return info;
        }
        curr = curr->next;
    }
    
    SchoolInfo* newInfo = schoolInfoCreate(cleanName, isActive);
    context->schoolInfoList = linkedListInsert(context->schoolInfoList, newInfo);
    return newInfo;
}

static IndividualInfo* findOrCreateIndividualInfo(ParserContext* context, const char* personName) {
    char cleanName[256];
    strncpy(cleanName, personName, 255);
    cleanName[255] = '\0';
    trim(cleanName);
    
    LinkedList* curr = context->individualInfoList;
    while(curr) {
        IndividualInfo* info = (IndividualInfo*)curr->info;
        if(strcmp(info->personName, cleanName) == 0) return info;
        curr = curr->next;
    }
    
    IndividualInfo* newInfo = individualInfoCreate(cleanName);
    context->individualInfoList = linkedListInsert(context->individualInfoList, newInfo);
    return newInfo;
}

ParserContext* parserContextCreate() {
    ParserContext* context = (ParserContext*)malloc(sizeof(ParserContext));
    context->yearInfoList = linkedListInitialize();
    context->schoolInfoList = linkedListInitialize();
    context->individualInfoList = linkedListInitialize();
    return context;
}

void parserContextFree(ParserContext* context) {
    if(!context) return;
    linkedListFree(context->yearInfoList, (FreeFunc)yearInfoFree);
    linkedListFree(context->schoolInfoList, (FreeFunc)schoolInfoFree);
    linkedListFree(context->individualInfoList, (FreeFunc)individualInfoFree);
    free(context);
}

int parseCampeas(ParserContext* context, const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if(!file) return -1;
    
    char line[1024];
    int firstLine = 1;
    
    while(fgets(line, sizeof(line), file)) {
        if(firstLine) {
            firstLine = 0;
            continue;
        }
        
        line[strcspn(line, "\n")] = 0;
        if(strlen(line) < 10) continue;
        
        char* tokens[6];
        int tokenCount = 0;
        char* token = strtok(line, "\t");
        
        while(token && tokenCount < 6) {
            tokens[tokenCount++] = token;
            token = strtok(NULL, "\t");
        }
        
        if(tokenCount < 6) continue;
        
        uint16_t year = (uint16_t)atoi(tokens[0]);
        char schoolName[256];
        cleanSchoolName(schoolName, tokens[1]);
        uint8_t titleNumber = (uint8_t)atoi(tokens[2]);
        char theme[256];
        strncpy(theme, tokens[3], 255);
        theme[255] = '\0';
        trim(theme);
        char designer[256];
        strncpy(designer, tokens[4], 255);
        designer[255] = '\0';
        trim(designer);
        
        char designerCopy[256];
        strncpy(designerCopy, designer, 255);
        designerCopy[255] = '\0';
        
        uint8_t isActive = !isExtinct(tokens[1]);
        
        YearInfo* yearInfo = findOrCreateYearInfo(context, year);
        SchoolInfo* schoolInfo = findOrCreateSchoolInfo(context, schoolName, isActive);
        
        ChampionshipInfo* champInfo = championshipInfoCreate(year, titleNumber, schoolName, theme, designer);
        yearInfoAddChampion(yearInfo, champInfo);
        schoolInfoAddTitle(schoolInfo, championshipInfoCreate(year, titleNumber, schoolName, theme, designer));
        
        char* viceSchools = tokens[5];
        char* viceToken = strtok(viceSchools, ";");
        while(viceToken) {
            trim(viceToken);
            if(strlen(viceToken) > 0 && strcmp(viceToken, "*") != 0) {
                char viceCleanName[256];
                cleanSchoolName(viceCleanName, viceToken);
                uint8_t viceActive = !isExtinct(viceToken);
                
                SchoolInfo* viceSchool = findOrCreateSchoolInfo(context, viceCleanName, viceActive);
                schoolInfoAddRunnerUp(viceSchool, championshipInfoCreate(year, 0, viceCleanName, "*", "*"));
                yearInfoAddRunnerUp(yearInfo, championshipInfoCreate(year, 0, viceCleanName, "*", "*"));
            }
            viceToken = strtok(NULL, ";");
        }
        
        if(strcmp(designerCopy, "*") != 0 && strlen(designerCopy) > 0) {
            char* designerToken = strtok(designerCopy, ";");
            while(designerToken) {
                trim(designerToken);
                cleanPersonName(designerToken);
                if(strlen(designerToken) > 0 && isValidPersonName(designerToken)) {
                    IndividualInfo* individual = findOrCreateIndividualInfo(context, designerToken);
                    Participation* part = participationCreate(schoolName, "Carnavalesco", year);
                    individualInfoAddParticipation(individual, part);
                }
                designerToken = strtok(NULL, ";");
            }
        }
    }
    
    fclose(file);
    return 0;
}

int parseEstandartes(ParserContext* context, const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if(!file) return -1;
    
    char line[1024];
    uint16_t currentYear = 0;
    
    while(fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        trim(line);
        
        if(strlen(line) == 0) continue;
        
        if(strlen(line) == 4 && isdigit(line[0])) {
            currentYear = (uint16_t)atoi(line);
            continue;
        }
        
        if(currentYear == 0) continue;
        
        char* tokens[3];
        int tokenCount = 0;
        char* token = strtok(line, "\t");
        
        while(token && tokenCount < 3) {
            tokens[tokenCount++] = token;
            token = strtok(NULL, "\t");
        }
        
        if(tokenCount < 2) continue;
        
        char category[128];
        strncpy(category, tokens[0], 127);
        category[127] = '\0';
        trim(category);
        
        if(isSpecialAwardCategory(category)) {
            if(tokenCount >= 2) {
                char personName[256];
                strncpy(personName, tokens[1], 255);
                personName[255] = '\0';
                trim(personName);
                cleanPersonName(personName);
                
                if(strlen(personName) > 0 && isValidPersonName(personName)) {
                    IndividualInfo* individual = findOrCreateIndividualInfo(context, personName);
                    Participation* part = participationCreate("Homenagem", category, currentYear);
                    individualInfoAddParticipation(individual, part);
                }
            }
            continue;
        }
        
        char schoolName[256];
        cleanSchoolName(schoolName, tokens[1]);
        
        uint8_t isActive = !isExtinct(tokens[1]);
        SchoolInfo* schoolInfo = findOrCreateSchoolInfo(context, schoolName, isActive);
        
        EstandarteAward* award = estandarteAwardCreate(currentYear, category, schoolName);
        schoolInfoAddAward(schoolInfo, award);
        
        //tem que ser pesoa, se nao for da categoria de psoa ignorar
        if(tokenCount >= 3 && strcmp(tokens[2], "") != 0 && isPersonCategory(category)) {
            char individualName[256];
            strncpy(individualName, tokens[2], 255);
            individualName[255] = '\0';
            trim(individualName);
            
            char* arrowPos = strstr(individualName, "->");
            if(arrowPos) {
                *arrowPos = '\0';
                trim(individualName);
            }
            
            if(strlen(individualName) > 0) {
                char* personToken = strtok(individualName, ";");
                while(personToken) {
                    trim(personToken);
                    cleanPersonName(personToken); 
                    if(strlen(personToken) > 0 && isValidPersonName(personToken)) {
                        IndividualInfo* individual = findOrCreateIndividualInfo(context, personToken);
                        Participation* part = participationCreate(schoolName, category, currentYear);
                        individualInfoAddParticipation(individual, part);
                    }
                    personToken = strtok(NULL, ";");
                }
            }
        }
    }
    
    fclose(file);
    return 0;
}

int saveAllStructures(ParserContext* context) {
    mkdir("data", 0755);
    mkdir("data/YearInfo", 0755);
    mkdir("data/SchoolInfo", 0755);
    mkdir("data/IndividualInfo", 0755);
    int count = 0;
    
    LinkedList* curr = context->yearInfoList;
    while(curr) {
        YearInfo* info = (YearInfo*)curr->info;
        char filename[512];
        snprintf(filename, sizeof(filename), "data/YearInfo/year-%04d.dat", info->year);
        yearInfoSave(info, filename);
        count++;
        curr = curr->next;
    }
    
    curr = context->schoolInfoList;
    while(curr) {
        SchoolInfo* info = (SchoolInfo*)curr->info;
        char filename[512];
        char safeName[256];
        strncpy(safeName, info->schoolName, 255);
        for(int i = 0; safeName[i]; i++) { //sanitização de nome, controle basico pra evitar problema, nao esquecer em outros arquivos
            if(safeName[i] == '/' || safeName[i] == '\\' || safeName[i] == ' ') {
                safeName[i] = '_';
            }
        }
        snprintf(filename, sizeof(filename), "data/SchoolInfo/school-%s.dat", safeName);
        schoolInfoSave(info, filename);
        count++;
        curr = curr->next;
    }
    
    curr = context->individualInfoList;
    while(curr) {
        IndividualInfo* info = (IndividualInfo*)curr->info;
        char filename[512];
        char safeName[256];
        strncpy(safeName, info->personName, 255);
        for(int i = 0; safeName[i]; i++) {
            if(safeName[i] == '/' || safeName[i] == '\\' || safeName[i] == ' ') {
                safeName[i] = '_';
            }
        }
        snprintf(filename, sizeof(filename), "data/IndividualInfo/individual-%s.dat", safeName);
        individualInfoSave(info, filename);
        count++;
        curr = curr->next;
    }
    
    return count;
}

int parseFiles(const char* campeasPath, const char* estandartesPath) {
    ParserContext* context = parserContextCreate();
    
    printf("Parsing campeas...\n");
    if(parseCampeas(context, campeasPath) != 0) {
        printf("Error parsing campeas\n");
        parserContextFree(context);
        return -1;
    }
    
    printf("Parsing estandartes...\n");
    if(parseEstandartes(context, estandartesPath) != 0) {
        printf("Error parsing estandartes\n");
        parserContextFree(context);
        return -1;
    }
    
    printf("Saving structures...\n");
    int count = saveAllStructures(context);
    
    printf("Done. %d files created\n", count);
    
    parserContextFree(context);
    return 0;
}

ParserContext* parseAndIndex(const char* campeasPath, const char* estandartesPath) {
    ParserContext* parserCtx = parserContextCreate();
    if(!parserCtx) return NULL;
    
    if(parseCampeas(parserCtx, campeasPath) != 0) {
        parserContextFree(parserCtx);
        return NULL;
    }
    
    if(parseEstandartes(parserCtx, estandartesPath) != 0) {
        parserContextFree(parserCtx);
        return NULL;
    }
    
    saveAllStructures(parserCtx);
    
    IndexerContext* indexerCtx = indexerCreate("data");
    if(!indexerCtx) {
        parserContextFree(parserCtx);
        return NULL;
    }
    
    indexerBuildFromParser(indexerCtx, parserCtx, "data");
    
    indexerFree(indexerCtx);
    
    return parserCtx;
}
