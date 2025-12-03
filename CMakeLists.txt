cmake_minimum_required(VERSION 3.5)
project(UFF_EDA_Trabalho_Computacional C)

set(CMAKE_C_STANDARD 11)

include_directories(libs)
include_directories(src)

#option(BUILD_TESTS "comp testes " OFF)

#se adicionar novas libs inclua-as aqui
set(LIB_SOURCES
        libs/BinaryTree.c
        libs/BinarySearchTree.c
        libs/LinkedList.c
        libs/BPlusTree.c
        libs/BPlusTree2M.c
        libs/BPlusStorage.c
        libs/HashUtils.c
        libs/HashMap.c
        libs/HashMap2M.c
)
set(LIB_HEADERS
        libs/BinaryTree.h
        libs/BinarySearchTree.h
        libs/LinkedList.h
        libs/GenericTypes.h
        libs/GenericImplementations.h
        libs/BPlusTree.h
        libs/BPlusTree2M.h
        libs/BPlusStorage.h
        libs/HashUtils.h
        libs/HashMap.h
        libs/HashMap2M.h
)

#inclua src aqui
set(SRC_SOURCES
        src/Main.c
        src/Console.c
        src/Mutation.c
        src/Questionary.c
        src/models/ChampionshipInfo.c
        src/models/EstandarteAward.c
        src/models/SchoolInfo.c
        src/models/YearInfo.c
        src/models/Participation.c
        src/models/IndividualInfo.c
        src/parsers/Parser.c
        src/parsers/Indexer.c
)
set(SRC_HEADERS
        src/Console.h
        src/Mutation.h
        src/Questionary.h
        src/models/ChampionshipInfo.h
        src/models/EstandarteAward.h
        src/models/SchoolInfo.h
        src/models/YearInfo.h
        src/models/Participation.h
        src/models/IndividualInfo.h
        src/parsers/Parser.h
        src/parsers/Indexer.h
)

    
add_executable(UFF_EDA_Trabalho_Computacional
        ${SRC_SOURCES}
        ${SRC_HEADERS}
        ${LIB_SOURCES}
        ${LIB_HEADERS}
)
