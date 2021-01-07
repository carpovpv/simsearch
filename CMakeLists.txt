cmake_minimum_required (VERSION 2.6)

SET(CMAKE_BUILD_TYPE "Release")
set (PROJECT LevelDbSearch)
project (${PROJECT})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11 -std=gnu++0x")

find_package(Boost REQUIRED COMPONENTS iostreams filesystem system)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
find_package(RDKit REQUIRED COMPONENTS FileParsers SmilesParse RDGeneral RDBoost ChemTransforms
    GraphMol Fingerprints DataStructs)
link_directories ( ${RDBASE}/lib )

include_directories(${RDKIT_INCLUDE_DIR})

set (HEADER 
	ctpl.h constants.h clean.h)

set (SOURCE 
        main.cpp clean.cpp)

add_executable ( ${PROJECT} ${HEADER} ${SOURCE} )
target_link_libraries(LevelDbSearch ${RDKIT_LIBRARIES} ${RDKit_LIBS} Boost::iostreams -lleveldb pthread)

add_executable ( LevelDbAdd constants.h clean.h add.cpp clean.cpp)
target_link_libraries(LevelDbAdd ${RDKIT_LIBRARIES} ${RDKit_LIBS} Boost::iostreams -lleveldb)

add_executable ( LevelDbServ constants.h clean.h serv.cpp clean.cpp)
target_link_libraries(LevelDbServ ${RDKIT_LIBRARIES} ${RDKit_LIBS} Boost::iostreams -lleveldb)


