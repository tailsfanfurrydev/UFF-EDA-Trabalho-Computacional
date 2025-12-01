#include "Console.h"
#include "Mutation.h"
#include "models/YearInfo.h"
#include "models/SchoolInfo.h"
#include "models/IndividualInfo.h"
#include "models/ChampionshipInfo.h"
#include "models/EstandarteAward.h"
#include "models/Participation.h"
#include "../libs/BPlusTree2M.h"
#include "../libs/HashMapM2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void trim(char* str) {
    if(!str) return;
    
    char* start = str;
    while(isspace((unsigned char)*start)) start++;
    
    char* end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    if(start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

static void printHelp() {
    printf("\nComandos disponiveis:\n");
    printf("  print <ano|escola|individuo> <all|nome>\n");
    printf("  add ano <ano>               - Adiciona um ano \n");
    printf("  add escola <nome>           - Adiciona uma escola \n");
    printf("  add individuo <nome>        - Adiciona um individuo\n");
    printf("  remove ano <ano>            - Remove um ano\n");
    printf("  remove escola <nome>        - Remove uma escola\n");
    printf("  remove individuo <nome>     - Remove um individuo\n");
    printf("  help                        - Esta mensagem\n");
    printf("  quit | sair | q             - Encerra o programa\n\n");
}

static void handlePrintAno(IndexerContext *indexer, char *arg) {
    if(!indexer || !indexer->yearIndex) {
        printf("Erro: indice de anos nao carregado\n");
        return;
    }
    if(strcmp(arg, "all") == 0) {
        bPlusTreePrintAll2M(indexer->yearIndex->indexFile, indexer->yearIndex->t);
        printf("\n");
    } else {
        int year = atoi(arg);
        if(year == 0) {
            printf("Erro: ano invalido\n");
            return;
        }
        
        char *path = indexerSearchYear(indexer, year);
        if(!path) {
            printf("Ano %d nao encontrado\n", year);
            return;
        }
        
        YearInfo *yearInfo = yearInfoLoad(path);
        free(path);
        
        if(!yearInfo) {
            printf("Erro ao carregar informacoes do ano %d\n", year);
            return;
        }
        
        yearInfoPrint(yearInfo);
        yearInfoFree(yearInfo);
    }
}

static void handlePrintEscola(IndexerContext *indexer, char *arg) {
    if(!indexer || !indexer->schoolIndex) {
        printf("Erro: indice de escolas nao carregado\n");
        return;
    }
    
    if(strcmp(arg, "all") == 0) {
        hashMapM2PrintAll(indexer->schoolIndex);
        printf("\n");
    } else {
        char *path = indexerSearchSchool(indexer, arg);
        if(!path) {
            printf("Escola '%s' nao encontrada\n", arg);
            return;
        }
        
        SchoolInfo *schoolInfo = schoolInfoLoad(path);
        free(path);
        
        if(!schoolInfo) {
            printf("Erro ao carregar informacoes da escola '%s'\n", arg);
            return;
        }
        
        schoolInfoPrint(schoolInfo);
        schoolInfoFree(schoolInfo);
    }
}

static void handlePrintIndividuo(IndexerContext *indexer, char *arg) {
    if(!indexer || !indexer->individualIndex) {
        printf("Erro: indice de individuos nao carregado\n");
        return;
    }
    
    if(strcmp(arg, "all") == 0) {
        hashMapM2PrintAll(indexer->individualIndex);
        printf("\n");
    } else {
        char *path = indexerSearchIndividual(indexer, arg);
        if(!path) {
            printf("Individuo '%s' nao encontrado\n", arg);
            return;
        }
        
        IndividualInfo *individualInfo = individualInfoLoad(path);
        free(path);
        
        if(!individualInfo) {
            printf("Erro ao carregar informacoes do individuo '%s'\n", arg);
            return;
        }
        
        individualInfoPrint(individualInfo);
        individualInfoFree(individualInfo);
    }
}

static void handleAddAno(IndexerContext *indexer, char *arg) {
    int year = atoi(arg);
    if(year == 0) {
        printf("Erro: ano invalido\n");
        return;
    }
    
    printf("\n=== Adicionando ano %d ===\n", year);
    
    YearInfo *yearInfo = yearInfoCreate(year);
    if(!yearInfo) {
        printf("Erro ao criar estrutura do ano\n");
        return;
    }
    
    char input[512];
    int count;
    
    printf("Quantos campeoes? ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) return;
    count = atoi(input);
    
    for(int i = 0; i < count; i++) {
        printf("\n--- Campeao %d ---\n", i + 1);
        
        printf("Nome da escola: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        trim(input);
        char schoolName[256];
        strncpy(schoolName, input, 255);
        schoolName[255] = 0;
        
        printf("Numero do titulo: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        int titleNum = atoi(input);
        
        printf("Tema/Enredo: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char theme[256];
        strncpy(theme, input, 255);
        theme[255] = 0;
        
        printf("Carnavalesco: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char designer[256];
        strncpy(designer, input, 255);
        designer[255] = 0;
        
        ChampionshipInfo *champ = championshipInfoCreate(year, titleNum, schoolName, theme, designer);
        yearInfoAddChampion(yearInfo, champ);
    }
    
    printf("\nQuantos vice-campeoes? ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) {
        yearInfoFree(yearInfo);
        return;
    }
    count = atoi(input);
    
    for(int i = 0; i < count; i++) {
        printf("\n--- Vice-campeao %d ---\n", i + 1);
        
        printf("Nome da escola: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        trim(input);
        char schoolName[256];
        strncpy(schoolName, input, 255);
        schoolName[255] = 0;
        
        printf("Numero do titulo: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        int titleNum = atoi(input);
        
        printf("Tema/Enredo: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char theme[256];
        strncpy(theme, input, 255);
        theme[255] = 0;
        
        printf("Carnavalesco: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char designer[256];
        strncpy(designer, input, 255);
        designer[255] = 0;
        
        ChampionshipInfo *runner = championshipInfoCreate(year, titleNum, schoolName, theme, designer);
        yearInfoAddRunnerUp(yearInfo, runner);
    }
    
    if(mutationAddYear(indexer, yearInfo)) {
       // printf("\nAno %d adicionado com sucesso!\n", year);
    }
    
    yearInfoFree(yearInfo);
}

static void handleAddEscola(IndexerContext *indexer, char *arg) {
    printf("\n=== Adicionando escola '%s' ===\n", arg);
    
    char input[512];
    
    printf("Escola esta ativa? (1=sim, 0=nao): ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) return;
    int isActive = atoi(input);
    
    SchoolInfo *schoolInfo = schoolInfoCreate(arg, isActive);
    if(!schoolInfo) {
        printf("Erro ao criar estrutura da escola\n");
        return;
    }
    
    printf("\nQuantos titulos adicionar? ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) {
        schoolInfoFree(schoolInfo);
        return;
    }
    int count = atoi(input);
    
    for(int i = 0; i < count; i++) {
        printf("\n--- Titulo %d ---\n", i + 1);
        
        printf("Ano: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        int year = atoi(input);
        
        printf("Numero do titulo: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        int titleNum = atoi(input);
        
        printf("Tema/Enredo: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char theme[256];
        strncpy(theme, input, 255);
        theme[255] = 0;
        
        printf("Carnavalesco: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char designer[256];
        strncpy(designer, input, 255);
        designer[255] = 0;
        
        ChampionshipInfo *title = championshipInfoCreate(year, titleNum, arg, theme, designer);
        schoolInfoAddTitle(schoolInfo, title);
    }
    
    printf("\nQuantos vice-campeonatos adicionar? ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) {
        schoolInfoFree(schoolInfo);
        return;
    }
    count = atoi(input);
    
    for(int i = 0; i < count; i++) {
        printf("\n--- Vice-campeonato %d ---\n", i + 1);
        
        printf("Ano: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        int year = atoi(input);
        
        printf("Numero do titulo: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        int titleNum = atoi(input);
        
        printf("Tema/Enredo: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char theme[256];
        strncpy(theme, input, 255);
        theme[255] = 0;
        
        printf("Carnavalesco: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char designer[256];
        strncpy(designer, input, 255);
        designer[255] = 0;
        
        ChampionshipInfo *runner = championshipInfoCreate(year, titleNum, arg, theme, designer);
        schoolInfoAddRunnerUp(schoolInfo, runner);
    }
    
    printf("\nQuantos premios Estandarte de Ouro adicionar? ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) {
        schoolInfoFree(schoolInfo);
        return;
    }
    count = atoi(input);
    
    for(int i = 0; i < count; i++) {
        printf("\n--- Premio %d ---\n", i + 1);
        
        printf("Ano: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        int year = atoi(input);
        
        printf("Categoria: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char category[128];
        strncpy(category, input, 127);
        category[127] = 0;
        
        printf("Vencedor: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char winner[256];
        strncpy(winner, input, 255);
        winner[255] = 0;
        
        EstandarteAward *award = estandarteAwardCreate(year, category, winner);
        schoolInfoAddAward(schoolInfo, award);
    }
    
    if(mutationAddSchool(indexer, schoolInfo)) {
       // printf("\nEscola '%s' adicionada com sucesso!\n", arg);
    }
    
    schoolInfoFree(schoolInfo);
}

static void handleAddIndividuo(IndexerContext *indexer, char *arg) {
    printf("\n=== Adicionando individuo '%s' ===\n", arg);
    
    IndividualInfo *individualInfo = individualInfoCreate(arg);
    if(!individualInfo) {
        printf("Erro ao criar estrutura do individuo\n");
        return;
    }
    
    char input[512];
    
    printf("Quantas participacoes adicionar? ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) {
        individualInfoFree(individualInfo);
        return;
    }
    int count = atoi(input);
    
    for(int i = 0; i < count; i++) {
        printf("\n--- Participacao %d ---\n", i + 1);
        
        printf("Nome da escola: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        trim(input);
        char schoolName[256];
        strncpy(schoolName, input, 255);
        schoolName[255] = 0;
        
        printf("Categoria: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        char category[128];
        strncpy(category, input, 127);
        category[127] = 0;
        
        printf("Ano: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        int year = atoi(input);
        
        Participation *part = participationCreate(schoolName, category, year);
        individualInfoAddParticipation(individualInfo, part);
    }
    
    if(mutationAddIndividual(indexer, individualInfo)) {
       // printf("\nIndividuo '%s' adicionado com sucesso!\n", arg);
    }
    
    individualInfoFree(individualInfo);
}

static void processCommand(IndexerContext *indexer, char *input) {
    trim(input);
    
    if(strlen(input) == 0) return;
    
    char command[256];
    char structure[256];
    char arg[512];
    
    int parsed = sscanf(input, "%255s %255s %511[^\n]", command, structure, arg);
    
    if(strcmp(command, "help") == 0) {
        printHelp();
        return;
    }
    
    if(strcmp(command, "quit") == 0 || strcmp(command, "sair") == 0 || strcmp(command, "q") == 0) {
        return;
    }
    
    if(strcmp(command, "print") == 0) {
        if(parsed < 3) {
            printf("Erro: uso incorreto. Digite 'help' para ver os comandos\n");
            return;
        }
        
        trim(arg);
        
        if(strcmp(structure, "ano") == 0) {
            handlePrintAno(indexer, arg);
        } else if(strcmp(structure, "escola") == 0) {
            handlePrintEscola(indexer, arg);
        } else if(strcmp(structure, "individuo") == 0) {
            handlePrintIndividuo(indexer, arg);
        } else {
            printf("Erro: estrutura '%s' nao reconhecida\n", structure);
            printf("Use: ano, escola ou individuo\n");
        }
    } else if(strcmp(command, "remove") == 0) {
        if(parsed < 3) {
            printf("Erro: uso incorreto. Digite 'help' para ver os comandos\n");
            return;
        }
        
        trim(arg);
        
        if(strcmp(structure, "ano") == 0) {
            int year = atoi(arg);
            if(year == 0) {
                printf("Erro: ano invalido\n");
                return;
            }
            mutationRemoveYear(indexer, year);
        } else if(strcmp(structure, "escola") == 0) {
            mutationRemoveSchool(indexer, arg);
        } else if(strcmp(structure, "individuo") == 0) {
            mutationRemoveIndividual(indexer, arg);
        } else {
            printf("Erro: estrutura '%s' nao reconhecida\n", structure);
            printf("Use: ano, escola ou individuo\n");
        }
    } else if(strcmp(command, "add") == 0) {
        if(parsed < 3) {
            printf("Erro: uso incorreto. Digite 'help' para ver os comandos\n");
            return;
        }
        
        trim(arg);
        
        if(strcmp(structure, "ano") == 0) {
            handleAddAno(indexer, arg);
        } else if(strcmp(structure, "escola") == 0) {
            handleAddEscola(indexer, arg);
        } else if(strcmp(structure, "individuo") == 0) {
            handleAddIndividuo(indexer, arg);
        } else {
            printf("Erro: estrutura '%s' nao reconhecida\n", structure);
            printf("Use: ano, escola ou individuo\n");
        }
    } else {
        printf("ooops, comando nao encontrado: '%s'\n", command);
        printf("escreva help para ver os comandos disponiveis\n");
    }
}

void consoleRun(IndexerContext *indexer) {
    if(!indexer) {
        printf("Erro: indexer nao inicializado\n");
        return;
    }
    
    printf("\nHello World :3\n");
    printf("escreva help para ver os comandos disponiveis\n\n");
    
    char input[1024];
    
    while(1) {
        printf("> ");
        fflush(stdout);
        
        if(!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        trim(input);
        
        if(strcmp(input, "quit") == 0 || strcmp(input, "sair") == 0 || strcmp(input, "q") == 0) {
            printf("goodbye beautyful world\n");
            break;
        }
        
        processCommand(indexer, input);
    }
}
