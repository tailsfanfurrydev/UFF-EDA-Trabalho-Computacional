#include "Console.h"
#include "Mutation.h"
#include "Questionary.h"
#include "models/YearInfo.h"
#include "models/SchoolInfo.h"
#include "models/IndividualInfo.h"
#include "models/ChampionshipInfo.h"
#include "models/EstandarteAward.h"
#include "models/Participation.h"
#include "../libs/BPlusTree2M.h"
#include "../libs/HashMap2M.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

static void trim(char* str){
    if(!str) return;
    
    char* start = str;
    while(isspace((unsigned char)*start)) start++;
    
    char* end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    if(start != str){
        memmove(str, start, strlen(start) + 1);
    }
}

static void printHelp(){
    printf("\n");
    printf("COMANDOS:\n");
    printf("  print <tipo> <id>     - Mostra dados (tipo: ano, escola, individuo)\n");
    printf("  print <tipo> all      - Lista todos\n");
    printf("  add <tipo> <id>       - Adiciona novo\n");
    printf("  remove <tipo> <id>    - Remove existente\n");
    printf("  questao <letra>       - Executa questao (a-x ou 'all')\n");
    printf("  help                  - Mostra ajuda\n");
    printf("  quit | sair | q       - Sai\n");
    printf("\n");
}
static void handlePrintAno(IndexerContext *indexer, char *arg){
    if(!indexer || !indexer->yearIndex){
        printf("Erro: indice de anos nao carregado\n");
        return;
    }
    if(strcmp(arg, "all") == 0){
        bPlusTreePrintAll2M(indexer->yearIndex->indexFile, indexer->yearIndex->t);
        printf("\n");
    } else{
        int year = atoi(arg);
        if(year == 0){
            printf("Erro: ano invalido\n");
            return;
        }
        
        char *path = indexerSearchYear(indexer, year);
        if(!path){
            printf("Ano %d nao encontrado\n", year);
            return;
        }
        
        YearInfo *yearInfo = yearInfoLoad(path);
        free(path);
        if(!yearInfo){
            printf("Erro ao carregar informacoes do ano %d\n", year);
            return;
        }
        
        yearInfoPrint(yearInfo);
        yearInfoFree(yearInfo);
    }
}

static void handlePrintEscola(IndexerContext *indexer, char *arg){
    if(!indexer || !indexer->schoolIndex){
        printf("Erro: indice de escolas nao carregado\n");
        return;
    }
    
    if(strcmp(arg, "all") == 0){
        hashMapM2PrintAll(indexer->schoolIndex);
        printf("\n");
    } else{
        char *path = indexerSearchSchool(indexer, arg);
        if(!path){
            printf("Escola '%s' nao encontrada\n", arg);
            return;
        }
        
        SchoolInfo *schoolInfo = schoolInfoLoad(path);
        free(path);
        
        if(!schoolInfo){
            printf("Erro ao carregar informacoes da escola '%s'\n", arg);
            return;
        }
        
        schoolInfoPrint(schoolInfo);
        schoolInfoFree(schoolInfo);
    }
}

static void handlePrintIndividuo(IndexerContext *indexer, char *arg){
    if(!indexer || !indexer->individualIndex){
        printf("Erro: indice de individuos nao carregado\n");
        return;
    }
    
    if(strcmp(arg, "all") == 0){
        hashMapM2PrintAll(indexer->individualIndex);
        printf("\n");
    } else{
        char *path = indexerSearchIndividual(indexer, arg);
        if(!path){
            printf("Individuo '%s' nao encontrado\n", arg);
            return;
        }
        
        IndividualInfo *individualInfo = individualInfoLoad(path);
        free(path);
        
        if(!individualInfo){
            printf("Erro ao carregar informacoes do individuo '%s'\n", arg);
            return;
        }
        
        individualInfoPrint(individualInfo);
        individualInfoFree(individualInfo);
    }
}

static void handleAddAno(IndexerContext *indexer, char *arg) {
    int year = atoi(arg);
    if(year == 0){
        printf("Erro: ano invalido\n");
        return;
    }
    
    printf("\n=== ADICIONAR ANO %d ===\n\n", year);
    
    YearInfo *yearInfo = yearInfoCreate(year);
    if(!yearInfo){
        printf("Erro ao criar estrutura do ano\n");
        return;
    }
    
    char input[512];
    int count;
    
    printf("Quantos campeoes? ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) return;
    count = atoi(input);
    
    for(int i = 0; i < count; i++){
        printf("\n--- Campeao %d ---\n", i + 1);
        
        printf("Nome da escola: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        trim(input);
        char schoolName[256];
        strncpy(schoolName, input, 255);
        schoolName[255] = 0;
        
        char *schoolPath = indexerSearchSchool(indexer, schoolName);
        if(!schoolPath){
            printf("ERRO: Escola '%s' nao existe!\n", schoolName);
            printf("Adicione a escola primeiro com: add escola %s\n", schoolName);
            yearInfoFree(yearInfo);
            return;
        }
        free(schoolPath);
        
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
    if(!fgets(input, sizeof(input), stdin)){
        yearInfoFree(yearInfo);
        return;
    }
    count = atoi(input);
    
    for(int i = 0; i < count; i++){
        printf("\n--- Vice-campeao %d ---\n", i + 1);
        
        printf("Nome da escola: ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;
        trim(input);
        char schoolName[256];
        strncpy(schoolName, input, 255);
        schoolName[255] = 0;
        
        //existe? nao pode adicionar se nao existir
        char *schoolPath = indexerSearchSchool(indexer, schoolName);
        if(!schoolPath){
            printf("ERRO: Escola '%s' nao existe!\n", schoolName);
            printf("Adicione a escola primeiro com: add escola %s\n", schoolName);
            yearInfoFree(yearInfo);
            return;
        }
        free(schoolPath);
        
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
    
    printf("\n");
    mutationAddYear(indexer, yearInfo);
    
    yearInfoFree(yearInfo);
}

static void handleAddEscola(IndexerContext *indexer, char *arg){
    printf("\n=== ADICIONAR ESCOLA: %s ===\n\n", arg);
    
    char input[512];
    
    printf("Escola esta ativa? (1=sim, 0=nao): ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)) return;
    int isActive = atoi(input);
    
    SchoolInfo *schoolInfo = schoolInfoCreate(arg, isActive);
    if(!schoolInfo){
        printf("Erro ao criar estrutura da escola\n");
        return;
    }
    
    printf("\n");
    mutationAddSchool(indexer, schoolInfo);
    
    schoolInfoFree(schoolInfo);
}

static void handleAddIndividuo(IndexerContext *indexer, char *arg){
    printf("\n=== ADICIONAR INDIVIDUO: %s ===\n\n", arg);
    
    IndividualInfo *individualInfo = individualInfoCreate(arg);
    if(!individualInfo){
        printf("Erro ao criar estrutura do individuo\n");
        return;
    }
    
    char input[512];
    
    printf("Quantas participacoes adicionar? ");
    fflush(stdout);
    if(!fgets(input, sizeof(input), stdin)){
        individualInfoFree(individualInfo);
        return;
    }
    int count = atoi(input);
    
    for(int i = 0; i < count; i++){
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
    
    printf("\n");
    mutationAddIndividual(indexer, individualInfo);
    
    individualInfoFree(individualInfo);
}

static void processCommand(IndexerContext *indexer, char *input){
    trim(input);
    
    if(strlen(input) == 0) return;
    
    char command[256];
    char structure[256];
    char arg[512];
    
    int parsed = sscanf(input, "%255s %255s %511[^\n]", command, structure, arg);
    
    if(strcmp(command, "help") == 0){
        printHelp();
        return;
    }
    
    if(strcmp(command, "quit") == 0 || strcmp(command, "sair") == 0 || strcmp(command, "q") == 0){
        return;
    }
    
    if(strcmp(command, "print") == 0){
        if(parsed < 3){
            printf("Erro: uso incorreto. Digite 'help' para ver os comandos\n");
            return;
        }
        
        trim(arg);
        
        if(strcmp(structure, "ano") == 0){
            handlePrintAno(indexer, arg);
        } else if(strcmp(structure, "escola") == 0){
            handlePrintEscola(indexer, arg);
        } else if(strcmp(structure, "individuo") == 0){
            handlePrintIndividuo(indexer, arg);
        } else{
            printf("Erro: estrutura '%s' nao reconhecida\n", structure);
            printf("Use: ano, escola ou individuo\n");
        }
    } else if(strcmp(command, "remove") == 0){
        if(parsed < 3){
            printf("Erro: uso incorreto. Digite 'help' para ver os comandos\n");
            return;
        }
        
        trim(arg);
        
        if(strcmp(structure, "ano") == 0){
            int year = atoi(arg);
            if(year == 0){
                printf("Erro: ano invalido\n");
                return;
            }
            mutationRemoveYear(indexer, year);
        } else if(strcmp(structure, "escola") == 0){
            mutationRemoveSchool(indexer, arg);
        } else if(strcmp(structure, "individuo") == 0){
            mutationRemoveIndividual(indexer, arg);
        } else{
            printf("Erro: estrutura '%s' nao reconhecida\n", structure);
            printf("Use: ano, escola ou individuo\n");
        }
    } else if(strcmp(command, "add") == 0){
        if(parsed < 3){
            printf("Erro: uso incorreto. Digite 'help' para ver os comandos\n");
            return;
        }
        
        trim(arg);
        
        if(strcmp(structure, "ano") == 0){
            handleAddAno(indexer, arg);
        } else if(strcmp(structure, "escola") == 0){
            handleAddEscola(indexer, arg);
        } else if(strcmp(structure, "individuo") == 0){
            handleAddIndividuo(indexer, arg);
        } else{
            printf("Erro: estrutura '%s' nao reconhecida\n", structure);
            printf("Use: ano, escola ou individuo\n");
        }
    } else if(strcmp(command, "questao") == 0){
        if(parsed < 2){
            printf("Erro: uso incorreto. Use: questao <letra>\n");
            return;
        }
        if(strcmp(structure, "all") == 0){
            questionAll(indexer);
            return;
        }
        
        char letter = tolower(structure[0]);
        
        switch(letter){
            case 'a': questionA(indexer); break;
            case 'b': questionB(indexer); break;
            case 'c': questionC(indexer); break;
            case 'd': questionD(indexer); break;
            case 'e': questionE(indexer); break;
            case 'f': questionF(indexer); break;
            case 'g': questionG(indexer); break;
            case 'h': questionH(indexer); break;
            case 'i': questionI(indexer); break;
            case 'j': questionJ(indexer); break;
            case 'l': questionL(indexer); break;
            case 'm': questionM(indexer); break;
            case 'n': questionN(indexer); break;
            case 'o': questionO(indexer); break;
            case 'p': questionP(indexer); break;
            case 'q': questionQ(indexer); break;
            case 'r': questionR(indexer); break;
            case 's': questionS(indexer); break;
            case 't': questionT(indexer); break;
            case 'u': questionU(indexer); break;
            case 'v': questionV(indexer); break;
            case 'x': questionX(indexer); break;
            default:
                printf("Erro: questao '%c' nao existe\n", letter);
                printf("Use letras de a-x (exceto k e w) ou 'all' para todas\n"); //CADE O K E O W ROSETI??? ISSO ESTA ME DEIXANDO LOUCO ASDKHASDKJASHDSAD
        }
    } 
    //no one cares about this, mas é fofo e me deixa feliz, ass: allan miguel
    else if(strcmp(command, "miau") == 0 || strcmp(command, "meow") == 0) {
        const char *cats[] = {
            "\n"
            "           __..--''``---....___   _..._    __\n"
            " /// //_.-'    .-/\";  `        ``<._  ``.''_ `. / // /\n"
            "///_.-' _..--.'_    \\                    `( ) ) // //\n"
            "/ (_..-' // (< _     ;_..__               ; `' / ///\n"
            " / // // //  `-._,_)' // / ``--...____..-' /// / //\n"
            "Alerta C4T: gato detectado! iniciar protocolo 'cafuné'\n",
            
            "\n"
            "       |\\      _,,,---,,_\n"
            "ZZZzz /,`.-'`'    -.  ;-;;,_\n"
            "     |,4-  ) )-,_. ,\\ (  `'-'\n"
            "    '---''(_/--'  `-'\\_)  \n"
            "  não acorde o mestre dos ronsrons\n",
            
            "\n"
            "   |\\---/|\n"
            "   | ,_, |\n"
            "    \\_`_/-..----.\n"
            " ___/ `   ' ,\"\"+ \\  sk\n"
            "(__...'   __\\    |`.___.';\n"
            "  (_,...'(_,.`__)/'.....+\n"
            " mischievous thoughts\n",
            
            "\n"
            "    /\\_____/\\\n"
            "   /  o   o  \\\n"
            "  ( ==  ^  == )\n"
            "   )         (\n"
            "  (           )\n"
            " ( (  )   (  ) )\n"
            "(__(__)___(__)__)\n"
            " me dá petisco?\n",
            
            "\n"
            "  ,-.       _,---._ __  / \\\n"
            " /  )    .-'       `./ /   \\\n"
            "(  (   ,'            `/    /|\n"
            " \\  `-\"             \\'\\   / |\n"
            "  `.              ,  \\ \\ /  |\n"
            "   /`.          ,'-`----Y   |\n"
            "  (            ;        |   '\n"
            "  |  ,-.    ,-' petisco |  /\n"
            "  |  | (   |            | /\n"
            "  )  |  \\  `.___________|/\n"
            "  `--'   `--'\n"
            "ops, sem petiscos hoje!\n",
            
            "\n   the bug killer!!!!!\n"
            "       _                        \n"
            "       \\`*-.                    \n"
            "        )  _`-.                 \n"
            "       .  : `. .                \n"
            "       : _   '  \\               \n"
            "       ; *` _.   `*-._          \n"
            "       `-.-'          `-.       \n"
            "         ;       `       `.     \n"
            "         :.       .        \\    \n"
            "         . \\  .   :   .-'   .   \n"
            "         '  `+.;  ;  '      :   \n"
            "         :  '  |    ;       ;-. \n"
            "         ; '   : :`-:     _.`* ;\n"
            "[bug] .*' /  .*' ; .*`- +'  `*' \n"
            "      `*-*   `*-*  `*-*'\n",
            
            "\n"
            ".==============================================.\n"
            "|      mewo                                    |\n"
            "|                           .'\\                |\n"
            "|                          //  ;               |\n"
            "|                         /'   |               |\n"
            "|        .----..._    _../ |   \\               |\n"
            "|         \\'---._ `.-'      `  .'              |\n"
            "|          `.    '              `.             |\n"
            "|            :            _,.    '.            |\n"
            "|            |     ,_    (() '    |            |\n"
            "|            ;   .'(()).  '      _/__..-       |\n"
            "|            \\ _ '       __  _.-'--._          |\n"
            "|            ,'.'...____'::-'  \\     `'        |\n"
            "|           / |   /         .---.              |\n"
            "|     .-.  '  '  / ,---.   (     )             |\n"
            "|    / /       ,' (     )---`-`-`-.._          |\n"
            "|   : '       /  '-`-`-`..........--'\\         |\n"
            "|   ' :      /  /                     '.       |\n"
            "|   :  \\    |  .'         o             \\      |\n"
            "|    \\  '  .' /          o       .       '     |\n"
            "|     \\  `.|  :      ,    : _o--'.\\      |     |\n"
            "|      `. /  '       ))    (   )  \\>     |     |\n"
            "|        ;   |      ((      \\ /    \\___  |     |\n"
            "|        ;   |      _))      `'.-'. ,-'` '     |\n"
            "|        |    `.   ((`            |/    /      |\n"
            "|        \\     ).  .))            '    .       |\n"
            "|     ----`-'-'  `''.::.________:::mx'' ---    |\n"
            "|                                              |\n"
            "|                       glub glub              |\n"
            "|                                              |\n"
            ".==============================================.\n",
            
            "\n"
            "⠀⢀⣤⣶⣶⣶⣆⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣒⣒⣢⣀⠀⠀⠀⠀\n"
            "⢠⣿⣿⣿⣿⣍⣿⣿⡌⡆⠀⠀⠀⠀⠀⠀⠀⢠⣾⣿⣟⣻⡎⠻⡧⠀⠀⠀\n"
            "⠀⢿⣟⣿⣿⣿⣿⠇⢹⣸⠀⠀⠀⠀⠀⠀⠀⠸⣿⣿⣿⣿⡇⠀⣷⠀⠀⠀\n"
            "⠀⠈⠻⣿⠿⠚⠋⠈⠙⠉⢀⢀⣠⣴⣄⣀⣀⠀⠈⠛⠿⣟⣀⡠⠃⠀⠀⠀\n"
            "⠀⠀⠀⠀⡀⠀⠀⠀⠀⠀⠙⢾⣥⠀⠀⠀⠙⡆⠀⠀⠀⢀⠀⢀⠀⠀⠀⠀\n"
            "⠀⠀⡠⡪⢀⠔⠠⠂⠀⠀⠀⠀⠹⣷⣤⡶⠋⠁⠀⠊⠔⠡⠐⡁⠀⠀⠀⠀\n"
            "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢂⣷⣯⡡⢀⠀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
            "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡰⠛⠉⠀⠀⠙⠳⡅⠀⠀⠁⡀⠀⠀⠀⠀⠀⠀\n"
            "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⠀⠀⠀⠀⠁⠄⠀⠀⠀⠀\n"
            "                                  \n"
            "  brain empty, no thoughts\n",
            
            "\n"
            "               )\\._.,--....,'``.      \n"
            " .b--.        /;   _.. \\   _\\  (`._ ,.\n"
            "`=,-,-'~~~   `----(,_..'--(,_..'`-.;.'\n",

            //if only i could tell him
            "\n"
            "            *     ,MMM8&&&.            *\n"
            "                  MMMM88&&&&&    .\n"
            "                 MMMM88&&&&&&&      sometimes i feel\n"
            "     *           MMM88&&&&&&&&        so...   lonely\n"
            "                 MMM88&&&&&&&&     \n"
            "            *    'MMM88&&&&&&'    thanks for staying\n"
            "                   'MMM8&&&'      *          with me  \n"
            "          |\\___/|     /\\___/\\\n"
            "          )     (     )    ~( .              '\n"
            "         =\\     /=   =\\~    /=   even if you \n"
            "     .     )===(       ) ~ (        dont realise\n"
            "          /     \\     /     \\          how much i love you\n"
            "          |     |     ) ~   (\n"
            "         /       \\   /     ~ \\\n"
            "         \\       /   \\~     ~/\n"
            "  cs__/\\_/\\__  _/_/\\_/\\__~__/_/\\_/\\_/\\_/\\_/\\_\n"
            "  |  |  |  |( (  |  |  | ))  |  |  |  |  |  |\n"
            "  |  |  |  | ) ) |  |  |//|  |  |  |  |  |  |\n"
            "  |  |  |  |(_(  |  |  (( |  |  |  |  |  |  |\n"
            "  |  |  |  |  |  |  |  |\\)|  |  |  |  |  |  |\n"
            "  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |\n",
            //he is everything for me, he'll never know it, he cant ever know it


            "\n watchu gotta there?\n"
            "        _._     _,-'\"\"`-._\n"
            "     (,-.`._,'(       |\\`-/|\n"
            "         `-.-' \\ )-`( , o o)\n"
            "               `-    \\`_`\"'-\n"
            
        };
        
        int numCats = sizeof(cats) / sizeof(cats[0]);
        int randomIndex = rand() % numCats;
        printf("%s\n", cats[randomIndex]);
    } else{
        printf("ooops, comando nao encontrado: '%s'\n", command);
        printf("escreva help para ver os comandos disponiveis\n");
    }
}

void consoleRun(IndexerContext *indexer){
    if(!indexer){
        printf("Erro: indexer nao inicializado\n");
        return;
    }
    srand(time(NULL));
    
    printf("\n");
    printf("▄▖▄▖▄▖▄▖▄▖▄▖▄▖▄▖  ▄ ▄▖▄▖  ▄ ▄▖▄ ▄▖▄▖\n");
    printf("▌▌▙▌▌▌▐ ▙▖▌▌▚ ▙▖  ▌▌▌▌▚   ▌▌▌▌▌▌▌▌▚ \n");
    printf("▛▌▌ ▙▌▐ ▙▖▙▌▄▌▙▖  ▙▘▙▌▄▌  ▙▘▛▌▙▘▙▌▄▌\n");
    printf("Allan Miguel, Lucas Lyra, Dillan Costa\n\n");
    printf("Digite 'help' para ver os comandos\n\n");
    
    char input[1024];
    
    while(1){
        printf("> ");
        fflush(stdout);
        if(!fgets(input, sizeof(input), stdin)){
            break;
        }
        trim(input);
        
        if(strcmp(input, "quit") == 0 || strcmp(input, "sair") == 0 || strcmp(input, "q") == 0){
            printf("\nAdeus mundo\n\n");
            break;
        }
        
        processCommand(indexer, input);
    }
}
