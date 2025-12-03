#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parsers/Indexer.h"
#include "parsers/Parser.h"
#include "Console.h"
#include "../libs/BPlusStorage.h"

static IndexerContext* globalIndexer = NULL;

int tryLoad(){
    globalIndexer = indexerCreate("data");
    
    if(!globalIndexer){
        return 0;
    }
    
    char* testYear = indexerSearchYear(globalIndexer, 1984);
    if(!testYear){
        indexerFree(globalIndexer);
        globalIndexer = NULL;
        return 0;
    }
    free(testYear);
    
    return 1;
}

void parseAndLoadFiles(){
    char campeasPath[256];
    char estandartesPath[256];
    
    printf("Campeas.txt: ");
    if(scanf("%255s", campeasPath) != 1){
        printf("Erro ao ler caminho\n");
        return;
    }
    
    printf("Estandartes.txt: ");
    if(scanf("%255s", estandartesPath) != 1){
        printf("Erro ao ler caminho\n");
        return;
    }
    
    printf("\nfazendo parsing\n");
    ParserContext* parserCtx = parseAndIndex(campeasPath, estandartesPath);
    
    if(parserCtx){
        parserContextFree(parserCtx);
        printf("parsing concluido\n");
        
        if(tryLoad()){
            printf("indices carregados\n");
        }
    } else{
        printf("erro ao processar\n");
    }
}

void freeAll(){
    if(globalIndexer){
        indexerFree(globalIndexer);
        globalIndexer = NULL;
    }
    bPlusTreeCleanupCurrentDirectory();
}

int main(int argc, char *argv[]){
    if(tryLoad()){
        printf("indices encontrados\n");
    } else{
        printf("indices nao encontrados. forneca os arquivos:\n");
        parseAndLoadFiles();
    }
    
    if(!globalIndexer){
        printf("falha ao iniciar\n");
        return 1;
    }
    
    consoleRun(globalIndexer);
    
    freeAll();
    return 0;
}
